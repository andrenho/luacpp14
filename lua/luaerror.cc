
#include "luainterface.h"

#if defined(__GNUC__) && !defined(__MINGW32__) && defined(DEBUG)
#  include <cxxabi.h>
#  include <execinfo.h>
#endif

#include <cstdlib>
#include <sstream>
#include <iostream>
#include <regex>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#define L (l_state.get())

/* here be dragons */

namespace lua {

void
LuaInterface::Error(string s) const
{
    lua_pushstring(L, s.c_str());
    lua_insert(L, 1);
    Traceback(L);
}


int 
LuaInterface::Traceback(lua_State* l)
{
    const char *msg = lua_tostring(l, 1);
    if (msg) {
        luaL_traceback(l, l, msg, 1);
    } else if (!lua_isnoneornil(l, 1)) {
        if (!luaL_callmeta(l, 1, "__tostring")) {
            lua_pushliteral(l, "(no error message)");
        }
    }
    // get lua backtrace
    stringstream ss;
    ss << "LUA STACK\n=========\n" + string(lua_tostring(l, -1)) + "\n\n";
    // get C++ backtrace
#if defined(__GNUC__) && !defined(__MINGW32__) && defined(DEBUG)
    ss << "C++ STACK\n=========";
    void *buffer[100];
    char** strings;
    int nptrs = backtrace(buffer, 100);
    strings = backtrace_symbols(buffer, nptrs);
    string prev = "";
    for(int i=0; i<nptrs; ++i) {
        string s = Demangle(string(strings[i]));
        if(s != prev) {
            ss << "\n\033[1;32m.\033[0m " + s;
        }
        prev = s;
    }
    free(strings);
#endif
    // call error callback
    //cerr << ss.str();
    lua_getglobal(l, "__engine_ptr");
    LuaInterface* lif = reinterpret_cast<LuaInterface*>(lua_touserdata(l, -1));
    lua_pop(l, -1);
    lif->error_cb(ss.str(), lif->error_cb_data);
    return 1;
}


string 
LuaInterface::Demangle(string s)
{
#if defined(__GNUC__) && !defined(__MINGW32__) && defined(DEBUG)
    regex e(".*\\((.+)(\\+.*)\\).*");
    
    smatch sm;
    regex_match(s, sm, e, regex_constants::match_default);
    if(sm.size() > 0) {
        int status;
        const char* c = string(sm[1]).c_str();
        char* realname = abi::__cxa_demangle(c, nullptr, nullptr, &status);
        if(realname) {
            // return demangled name
            string realn(realname);
            free(realname);
            return regex_replace(s, e, realn);
        } else {
            regex e(R"(|(\.?\/.+?)\(.*\) \[.*\]|)");
            return "\033[1;90m" + regex_replace(s, e, "$1") + "\033[0m";
        }
    } 
    else 
#endif
    {
        regex e(R"(|(\.?\/.+?)\(.*\) \[.*\]|)");
        return "\033[1;90m" + regex_replace(s, e, "$1") + "\033[0m";
    }
}


}  // namespace lua

// vim: ts=4:sw=4:sts=4:expandtab
