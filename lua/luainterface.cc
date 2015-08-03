
#include "luainterface.h"
#include "mylib.h"

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include <cassert>
#include <cstdlib>
#include <sstream>
#include <iostream>

#include "luahelper.h"

namespace lua {

LuaInterface::LuaInterface(function<void(string const&, void*)> error_cb, void* data)
    : l_state(luaL_newstate(), [](lua_State* l) { lua_close(l); }),  
      error_cb(error_cb), error_cb_data(data)
{
    luaL_openlibs(L());

    // store pointer to self (used to get this interface back from static methods)
    lua_pushlightuserdata(L(), reinterpret_cast<void*>(this));
    lua_setglobal(L(), "__engine_ptr");

#ifdef DEBUG
    Do("DEBUG = true");
#else
    Do("DEBUG = false");
#endif
    initialize_helper_functions(*this);
    LoadBuffer(lua_mylib_luac, lua_mylib_luac_len);
}


LuaInterface& 
LuaInterface::get(lua_State* L)
{
    lua_getglobal(L, "__engine_ptr");
    LuaInterface* lua = reinterpret_cast<LuaInterface*>(lua_touserdata(L, -1));
    lua->Pop();
    assert(lua);
    return *lua;
}



/*
 * load source
 */

void 
LuaInterface::LoadBuffer(unsigned char* code, size_t length) const
{
    int r = luaL_loadbuffer(L(), reinterpret_cast<const char*>(code), length, "preload");
    if(r == LUA_ERRSYNTAX) {
        Error("Syntax error");
    }
    assert(r != LUA_ERRMEM);

    if(Call(0, 0) == LUA_ERRRUN) {
        Error("Runtime error");
    }
}


void 
LuaInterface::LoadSource(string const& filename, string const& path) const
{
    // setup lua path
    if(path != "") {
        static char value[512]; // should be enough...
        snprintf(value, sizeof(value), "LUA_PATH=%s;;", path.c_str());
        putenv(value);
    }

    // load source
    int r = luaL_loadfile(L(), filename.c_str());
    if(r == LUA_ERRSYNTAX) {
        Error("Syntax error");
    } else if(r == LUA_ERRFILE) {
        Error("Could not read Lua file " + filename);
    }
    assert(r != LUA_ERRMEM);

    if(Call(0, 0) == LUA_ERRRUN) {  /* TODO */
        Error("Runtime error");
    }
}


/*
 * examine stack
 */

int
LuaInterface::StackSize() const
{
    return lua_gettop(L());
}


void 
LuaInterface::EnsureStackEmpty() const
{
    if(StackSize() != 0) {
        cerr << "Stack size == " << StackSize() << "\n";
        StackDump();
        abort();
    }
}


string
LuaInterface::StackDump() const
{
    int top = lua_gettop(L());

    auto inspect = [&](int i) {
        lua_getglobal(L(), "inspect");
        lua_pushvalue(L(), i);
        Call(1, 1);
        string s(lua_tostring(L(), -1));
        lua_pop(L(), 1);
        return s;
    };

    stringstream ss;
    ss << "Current lua stack:\n";
    for(int i=1; i<=top; ++i) {
        ss << "  " << i << "/" << i-top-1 << ": " << inspect(i) << "\n";
    }
    ss << "-----------------\n";
    cerr << ss.str();
    cerr.flush();
    return ss.str();
}


/*
 * get info about object in stack
 */

bool 
LuaInterface::IsA(string klass, int i) const
{
    int s = StackSize();

    lua_pushvalue(L(), i);

    if(lua_type(L(), -1) != LUA_TTABLE) {
        return false;
    }

    // stack:                          obj
    lua_pushstring(L(), "is_a");      // obj "is_a"
    lua_gettable(L(), -2);            // obj obj_is_a

    if(lua_isnil(L(), -1)) { // no "is_a"
        lua_pop(L(), 1);
        return false;
    }

    lua_getglobal(L(), klass.c_str());// obj obj_is_a klass
    lua_gettable(L(), -2);            // obj obj_is_a value
    bool v = lua_toboolean(L(), -1); 
    lua_pop(L(), 2);                  // obj

    lua_pop(L(), 1);
    assert(s == StackSize());

    return v;
}


bool 
LuaInterface::IsA(int lua_tp, int i) const
{
    return lua_type(L(), i) == lua_tp;
}


bool 
LuaInterface::IsNil(int i) const
{
    return lua_isnil(L(), i);
}

/*
 * add things to stack
 */


void
LuaInterface::Pop(int count) const 
{
    lua_pop(L(), count);
}


void 
LuaInterface::Push(int i) const 
{ 
    lua_pushnumber(L(), i); 
}


void 
LuaInterface::Push(double i) const 
{ 
    lua_pushnumber(L(), i); 
}


void 
LuaInterface::Push(string const& s) const 
{ 
    lua_pushstring(L(), s.c_str()); 
}


void 
LuaInterface::Push(bool b) const 
{ 
    lua_pushboolean(L(), b); 
}


void
LuaInterface::Push(Point const& p) const
{
    int s = StackSize();
    CallGlobalFunction("Point", p.x, p.y);
    assert(StackSize() == s+1);
}


void 
LuaInterface::PushGlobal(string const& global) const
{
    lua_getglobal(L(), global.c_str());
}


/*
 * lua object attributes
 */


bool 
LuaInterface::HasAttr(string const& field, int i) const
{
    int s = StackSize();

    bool has = true;
    lua_getfield(L(), i, field.c_str());
    if(lua_isnil(L(), -1)) {
        has = false;
    }
    lua_pop(L(), 1);

    assert(StackSize() == s);
    
    return has;
}


void 
LuaInterface::PushAttr(string const& attr, int i) const 
{
    if(!IsA(LUA_TTABLE, i)) {
        Error("Index is not a table");
    }

    lua_getfield(L(), i, attr.c_str());
}


/*
 * loop a table or array
 */


void 
LuaInterface::ForEach(function<void(int)> f, int i) const
{
    int s = StackSize();

    if(!lua_istable(L(), i)) {
        Error("Expected table.");
    }
    
    int n_obj = luaL_len(L(), i);

    for(int j=1; j<=n_obj; ++j) {
        lua_rawgeti(L(), i, j);  // get object
        int s2 = StackSize();
        f(j);                     // call callback
        assert(StackSize() == s2);
        lua_pop(L(), 1);
    }
    
    assert(StackSize() == s);
}


void 
LuaInterface::ForEachKey(function<void(int)> f) const
{
    int s = StackSize();

    if(!lua_istable(L(), -1)) {
        Error("Expected table.");
    }
    
    int i=1;
    lua_pushnil(L());
    while(lua_next(L(), -2)) {    // stores key (-2) and value (-1)
        lua_pop(L(), 1);          // pop value
        int s2 = StackSize();
        f(i++);                   // call callback
        assert(StackSize() == s2);
    }

    assert(StackSize() == s);
}


void 
LuaInterface::ForEachPair(function<void()> f) const
{
    int s = StackSize();

    if(!lua_istable(L(), -1)) {
        Error("Expected table.");
    }
    
    lua_pushnil(L());
    while(lua_next(L(), -2)) {    // stores key (-2) and value (-1)
        int s2 = StackSize();
        f();                      // call callback
        lua_pop(L(), 1);          // pop value
        assert(StackSize() == s2-1);
    }

    assert(StackSize() == s);
}


void 
LuaInterface::ForEachAttr(string const& attr, function<void(int)> f, int i) const
{
    PushAttr(attr);
    if(!lua_istable(L(), i)) {
        Error("Attribute '" + attr + "' is not a table");
    }
    ForEach(f, i);
    Pop();
}


/*
 * function calls
 */


int
LuaInterface::Call(int nargs, int nresults) const
{
    int s = StackSize();

    int base = lua_gettop(L()) - nargs;
    lua_pushcfunction(L(), Traceback);
    lua_insert(L(), base);
    int status = lua_pcall(L(), nargs, nresults, base);
    lua_remove(L(), base);

    if(nresults != LUA_MULTRET) {
        assert(StackSize() == s + nresults - nargs - 1);
    }
    
    return status;
}


int 
LuaInterface::ParameterCount() const
{
    return lua_gettop(L());
}


/*
 * Call C++ functions from Lua
 */
void 
LuaInterface::RegisterFunction(string const& name, lua_CFunction f) const
{
    int s = StackSize();

    Do(name + " = nil");  // skip strict
    lua_pushcfunction(L(), f);
    lua_setglobal(L(), name.c_str());

    assert(StackSize() == s);
}


void 
LuaInterface::RegisterFunction(string const& parent, string const& name, lua_CFunction f) const
{
    int s = StackSize();

    PushGlobal(parent);
    if(!lua_istable(L(), -1)) {
        Error("Expected table");
    }
    lua_pushcfunction(L(), f);
    lua_setfield(L(), -2, name.c_str());
    Pop();

    assert(StackSize() == s);
}


/*
 * Debug
 */

LuaInterface::SourceLine 
LuaInterface::CurrentLine() const
{
    lua_Debug ar;
    lua_getstack(L(), 0, &ar);
    lua_getinfo(L(), "S", &ar);
    lua_getinfo(L(), "n", &ar);
    lua_getinfo(L(), "l", &ar);
    return { 
        string(&ar.source[1]), 
        string(ar.short_src), 
        string(ar.what ? ar.what : ""), 
        string(ar.name ? ar.name : ""), 
        string(ar.namewhat ? ar.namewhat : ""), 
        ar.currentline };
}


vector<string> 
LuaInterface::Locals() const
{
    return { "TODO", };
}


void 
LuaInterface::CallOnNextLine(function<void()> f) const
{
    call_on_break = f;
    lua_sethook(L(), [](lua_State* L, lua_Debug*) { 
        lua_sethook(L, nullptr, LUA_MASKLINE, 0);  // disable hook
        lua_getglobal(L, "__engine_ptr");
        LuaInterface* luax = reinterpret_cast<LuaInterface*>(lua_touserdata(L, -1));
        luax->call_on_break();
    }, LUA_MASKLINE, 0);
}


void 
LuaInterface::CallOnNextReturn(function<void()> f) const
{
    call_on_break = f;
    last_call_stack_size = CallStackSize();
    lua_sethook(L(), [](lua_State* L, lua_Debug*) { 
        lua_getglobal(L, "__engine_ptr");
        LuaInterface* luax = reinterpret_cast<LuaInterface*>(lua_touserdata(L, -1));
        if(luax->CallStackSize() <= luax->last_call_stack_size) {
            lua_sethook(L, nullptr, LUA_MASKLINE, 0);  // disable hook
            luax->call_on_break();
        }
    }, LUA_MASKLINE, 0);
}


int 
LuaInterface::CallStackSize() const
{
    int depth = 0;
    lua_Debug e;
    while(lua_getstack(L(), depth++, &e));
    return depth;
}


}  // namespace lua


// vim: ts=4:sw=4:sts=4:expandtab
