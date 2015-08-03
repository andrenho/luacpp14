#ifndef LUA_LUAINTERFACE_H_
#define LUA_LUAINTERFACE_H_

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}

#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
using namespace std;

#include "point.h"

struct lua_State;

namespace lua {

class LuaInterface {
public:
    LuaInterface(function<void(string const&, void*)> error_cb, void* data);
    static LuaInterface& get(lua_State* L);

    // load source
    void LoadBuffer(unsigned char* code, size_t length) const;
    void LoadSource(string const& filename, string const& path = "") const;

    // examine stack
    int    StackSize() const;
    string StackDump() const;
    void   EnsureStackEmpty() const;

    // get info about object in stack
    bool IsA(string klass, int i=-1) const;
    bool IsA(int lua_type, int i=-1) const;
    bool IsNil(int i=-1) const;
    template<class T> typename enable_if<is_floating_point<T>::value, T>::type Get(int i=-1) const;
    template<class T> typename enable_if<is_integral<T>::value, T>::type       Get(int i=-1) const;
    template<class T> typename enable_if<is_same<T, string>::value, T>::type   Get(int i=-1) const;
    template<class T> typename enable_if<is_pointer<T>::value, T>::type        Get(int i=-1) const;
    template<class T> typename enable_if<is_same<T, Point>::value, T>::type    Get(int i=-1) const;
    template<class T> typename enable_if<is_same<T, vector<typename T::value_type, typename T::allocator_type>>::value, T>::type Get(int i=-1) const;
    template<class T> T GetGlobal(string const& variable) const;

    // remove things from stack
    void                                                                       Pop(int count=1) const;
    template<class T> typename enable_if<!is_void<T>::value, T>::type          Pop() const;
    template<class T> typename enable_if<is_void<T>::value, T>::type           Pop() const {}

    // add things to stack
    void Push(int i) const;
    void Push(double i) const;
    void Push(string const& s) const;
    void Push(bool b) const;
    void Push(Point const& p) const;
    template<class T> void Push(T* ptr) const;
    template<typename T> void Push(vector<T> const& v) const;
    void PushGlobal(string const& global) const;

    // lua object attributes
    bool HasAttr(string const& field, int i=-1) const;
    void PushAttr(string const& attr, int i=-1) const;
    template<class T> T GetAttr(string const& attr, int i=-1) const;
    template<class T> T GetAttrDef(string const& attr, T const& def, int i=-1) const;
    template<class T> void SetAttr(string const& attr, T const& value, int i=-1) const;

    // loop a table or array
    void ForEach(function<void(int)> f, int i=-1) const;
    void ForEachPair(function<void()> f) const;
    void ForEachKey(function<void(int)> f) const;
    void ForEachAttr(string const& attr, function<void(int)> f, int i=-1) const;

    // function calls
    template<class ...P> void CallFunctionInStack(P... pars) const;
    template<typename T, class ...P> T CallFunctionInStack(P... pars) const;
    template<class ...P> void CallMethod(string const& method, P... pars) const;
    template<class ...P> void CallVoidMethod(string const& method, P... pars) const;
    template<typename T, class ...P> T CallMethod(string const& method, P... pars) const;
    template<class ...P> void CallGlobalFunction(string const& f, P... pars) const;
    template<typename T, class ...P> T CallGlobalFunction(string const& f, P... pars) const;
    int Call(int nargs, int nresults) const;
    int ParameterCount() const;

    // immediate commands
    template<typename T> T Do(string const& code) const;
    void Do(string const& code) const;

    // call C++ functions from Lua
    void RegisterFunction(string const& name, lua_CFunction f) const;
    void RegisterFunction(string const& parent, string const& name, lua_CFunction f) const;

    // manage userdata
    template<typename Class, typename ...ParamType, typename... String> void RegisterConstructor(String... pars) const;

    // debug
    struct SourceLine {
        string source;
        string short_src;
        string what;
        string name;
        string namewhat;
        int line;
    };
    SourceLine CurrentLine() const;
    vector<string> Locals() const;
    void CallOnNextLine(function<void()> f) const;
    void CallOnNextReturn(function<void()> f) const;
    int CallStackSize() const;

    // error management (in luaerror.cc)
    void Error(string s) const;

    inline lua_State* L() const { return l_state.get(); }

private:
    // private templates
    template<class T, typename... S> T* InitializeObject(T* t);
    template<class Arg1, class... Args> void PushParameters(const Arg1& arg1, const Args&... args) const;
    void PushParameters() const;

    // error management (in luaerror.cc)
    static int Traceback(lua_State* l);
    static string Demangle(string s);

    // internal members
    unique_ptr<lua_State, function<void(lua_State*)>> l_state;
    function<void(string const&, void*)> error_cb;
    void* error_cb_data;

    mutable function<void()> call_on_break = nullptr;
    mutable int last_call_stack_size = 0;

    // constructors
    LuaInterface(LuaInterface const& other) = delete;
    LuaInterface(LuaInterface&& other) = delete;
    LuaInterface& operator=(LuaInterface const& other) = delete;
    LuaInterface& operator=(LuaInterface&& other) = delete;
};

}  // namespace lua

#include "luainterface.inl.h"

#endif  // LUA_LUAINTERFACE_H_

// vim: ts=4:sw=4:sts=4:expandtab
