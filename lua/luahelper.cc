
#include "luahelper.h"

#include <cstdio>

/*
#ifdef DEBUG
#  include <readline/readline.h>
#  include <readline/history.h>
#endif
*/

#include "luainterface.h"

namespace lua {


void initialize_helper_functions(LuaInterface const& luax)
{
    (void) luax;
/*
#ifdef DEBUG
    luax.RegisterFunction("readline", [](lua_State* L) {
        const char* prompt = luaL_checkstring(L, 1);
        char* input = readline(prompt);
        if(!input) {
            lua_pushstring(L, "");
        } else {
            add_history(input);
            lua_pushstring(L, input);
            free(input);
        }
        return 1;
    });
#endif
*/
}


}  // namespace lua

// vim: ts=4:sw=4:sts=4:expandtab
