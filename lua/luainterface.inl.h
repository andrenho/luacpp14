
#ifndef LUA_LUAINTERFACE_INL_H_
#define LUA_LUAINTERFACE_INL_H_

#include <cassert>

namespace lua {

/*
 * get info about object in stack
 */

template<class T> inline typename enable_if<is_floating_point<T>::value, T>::type 
LuaInterface::Get(int i) const
{
    if(!lua_isnumber(L(), i)) {
        Error("Expected number.");
    }
    return lua_tonumber(L(), i);
}


template<class T> inline typename enable_if<is_integral<T>::value, T>::type 
LuaInterface::Get(int i) const 
{ 
    if(lua_isnumber(L(), i)) {
        return lua_tonumber(L(), i);
    } else if(lua_isboolean(L(), i)) {
        return lua_toboolean(L(), i);
    } else if(lua_isnil(L(), i)) {
        return false;
    } else {
        Error("Expected number.");
        return 0;
    }
}


template<class T> inline typename enable_if<is_same<T, string>::value, T>::type 
LuaInterface::Get(int i) const
{
    if(!lua_isstring(L(), i)) {
        Error("Expected string.");
    }
    return string(lua_tostring(L(), i));
}


template<class T> inline typename enable_if<is_pointer<T>::value, T>::type 
LuaInterface::Get(int i) const {
    if(!lua_islightuserdata(L(), i) && !lua_isuserdata(L(), i)) {
        Error("Expected userdata or lightuserdata");
    }
    return reinterpret_cast<T>(lua_touserdata(L(), i));
}


template<class T> typename enable_if<is_same<T, Point>::value, T>::type
LuaInterface::Get(int i) const
{
    int s = StackSize();
    lua_pushvalue(L(), i);
    if(!IsA("Point")) {
        Error("Expected Point");
    }
    auto x = GetAttr<double>("x"),
         y = GetAttr<double>("y");
    lua_pop(L(), 1);
    assert(s == StackSize());
    return { x, y };
}


template<class T> typename enable_if<is_same<T, vector<typename T::value_type, typename T::allocator_type>>::value, T>::type 
LuaInterface::Get(int i) const
{
    int s = StackSize();

    T v;
    if(!lua_istable(L(), i)) {
        Error("Expected table.");
    }
    int n_obj = luaL_len(L(), i);

    for(int j=1; j<=n_obj; ++j) {
        lua_rawgeti(L(), i, j);  // get object
        v.push_back(Get<typename T::value_type>());
        lua_pop(L(), 1);
    }

    assert(s == StackSize());
    return v;
}


template<class T> inline T 
LuaInterface::GetGlobal(string const& variable) const
{
    lua_getglobal(L(), variable.c_str());
    auto t = Get<T>();
    Pop();
    return t;
}


/*
 * remove things from the stack
 */


template<class T> inline typename enable_if<!is_void<T>::value, T>::type 
LuaInterface::Pop() const 
{
    int s = StackSize();
    T t = Get<T>(-1);
    lua_pop(L(), 1);
    assert(StackSize() == s-1);
    return t;
}


/*
 * add things to the stack
 */
template<class T> inline void 
LuaInterface::Push(T* ptr) const 
{
    int s = StackSize();
    lua_pushlightuserdata(L(), ptr);
    assert(StackSize() == s+1);
}


template<typename T> inline void 
LuaInterface::Push(vector<T> const& v) const
{
    int s = StackSize();
    lua_createtable(L(), v.size(), 0);
    int i = 1;
    for(auto const& t: v) {
        Push(t);
        lua_rawseti(L(), -2, i++);
    }
    assert(StackSize() == s+1);
}


/*
 * lua object attributes
 */
template<typename T> inline T 
LuaInterface::GetAttr(string const& attr, int i) const 
{
    if(!IsA(LUA_TTABLE, i)) {
        Error("Index is not a table");
    }

    lua_getfield(L(), i, attr.c_str());
    return Pop<T>();
}


template<class T> inline T 
LuaInterface::GetAttrDef(string const& attr, T const& def, int i) const
{
    if(!IsA(LUA_TTABLE, i)) {
        Error("Index is not a table");
    }

    lua_getfield(L(), i, attr.c_str());
    if(IsNil()) {
        Pop();
        return def;
    } else {
        return Pop<T>();
    }
}


template<class T> inline void 
LuaInterface::SetAttr(string const& attr, T const& value, int i) const
{
    Push(value);

    if(!IsA(LUA_TTABLE, i-1)) {
        Error("Index is not a table");
    }

    lua_setfield(L(), i-1, attr.c_str());
}


/* 
 * function call
 */

template<class ...P> inline void 
LuaInterface::CallMethod(string const& method, P... pars) const 
{
    int s = StackSize();

    lua_getfield(L(), -1, method.c_str());
    if(lua_isnil(L(), -1)) {
        Error("Method `" + method + "` not found.");
    }
    lua_pushvalue(L(), -2);

    // parameters...
    PushParameters(pars...);

    // stack:                             obj fct obj [parameters]
    Call(sizeof...(P)+1, 1);

    assert(StackSize() == s+1);
}


template<class ...P> inline void 
LuaInterface::CallVoidMethod(string const& method, P... pars) const
{
    int s = StackSize();

    lua_getfield(L(), -1, method.c_str());
    if(lua_isnil(L(), -1)) {
        Error("Method `" + method + "` not found.");
    }
    lua_pushvalue(L(), -2);

    // parameters...
    PushParameters(pars...);

    // stack:                             obj fct obj [parameters]
    Call(sizeof...(P)+1, 0);

    assert(StackSize() == s);
}


template<typename T, class ...P> inline T 
LuaInterface::CallMethod(string const& method, P... pars) const
{
    CallMethod(method, pars...);
    return Pop<T>();
}


template<class ...P> inline void
LuaInterface::CallFunctionInStack(P... pars) const
{
    int s = StackSize();

    if(!lua_isfunction(L(), -1)) {
        Error("Stacked value not a function.");
    }

    // parameters...
    PushParameters(pars...);

    // stack:                             obj fct obj [parameters]
    Call(sizeof...(P), 1);

    assert(StackSize() == s+1-1);
}


template<typename T, class ...P> inline T
LuaInterface::CallFunctionInStack(P... pars) const
{
    CallFunctionInStack(pars...);
    return Pop<T>();
}


template<class ...P> inline void 
LuaInterface::CallGlobalFunction(string const& f, P... pars) const 
{
    int s = StackSize();

    lua_getglobal(L(), f.c_str());
    if(lua_isnil(L(), -1)) {
        Error("Function `" + f + "` not found.");
    }

    // parameters...
    PushParameters(pars...);

    // stack:                             obj fct obj [parameters]
    Call(sizeof...(P), 1);

    assert(StackSize() == s+1);
}


template<typename T, class ...P> inline T
LuaInterface::CallGlobalFunction(string const& f, P... pars) const
{
    CallGlobalFunction(f, pars...);
    return Pop<T>();
}


/*
 * immediate operations
 */


template<typename T> inline T
LuaInterface::Do(string const& code) const 
{
    int s = StackSize();

    int r = luaL_loadstring(L(), code.c_str());
    if(r == LUA_ERRSYNTAX) {
        Error("syntax error in immediate command");
    } else if(r == LUA_ERRFILE) {
        Error("error loading immediate command");
    }
    Call(0, 1);
    auto t = Pop<T>();

    assert(StackSize() == s);

    return t;
}


inline void 
LuaInterface::Do(string const& code) const 
{
    int r = luaL_loadstring(L(), code.c_str());
    if(r == LUA_ERRSYNTAX) {
        Error("syntax error in immediate command");
    } else if(r == LUA_ERRFILE) {
        Error("error loading immediate command");
    }
    Call(0, LUA_MULTRET);
}


/*
 * Manage userdata
 */



template<class Class, typename... ParamType> inline Class*
LuaInterface::InitializeObject(Class* t)
{
    int i = 0;
    return new(t) Class(Get<ParamType>(++i)...);
}


template<class Class, typename ...ParamType, typename... String> inline void 
LuaInterface::RegisterConstructor(String... pars) const
{
    int s = StackSize();

    RegisterFunction(pars..., [](lua_State* L) {
        auto& luax = LuaInterface::get(L);
        
        // create constructor
        Class* t = reinterpret_cast<Class*>(lua_newuserdata(L, sizeof(Class)));
        luax.InitializeObject<Class, ParamType...>(t);

        // create destructor
        luaL_newmetatable(L, "mt");
        lua_pushcfunction(L, [](lua_State* L) {
            Class* t = reinterpret_cast<Class*>(lua_touserdata(L, 1));
            t->~Class();
            return 0;
        });
        lua_setfield(L, -2, "__gc");
        lua_setmetatable(L, -2);

        return 1;
    });

    assert(StackSize() == s);
}



/*
 * private templates
 */


template<class Arg1, class... Args> inline void 
LuaInterface::PushParameters(const Arg1& arg1, const Args&... args) const 
{
    Push(arg1);
    PushParameters(args...);
}
inline void LuaInterface::PushParameters() const {}


}  // namespace lua

#endif  // LUA_LUAINTERFACE_INL_H_

// vim: ts=4:sw=4:sts=4:expandtab
