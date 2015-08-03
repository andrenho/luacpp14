LUA INTERFACE

  Load source:

     LoadBuffer(buffer, buffer_size)
     LoadSource(lua_source_file, [path])

  Examine stack:

     StackSize()            -> return the size of the stack
     StackDump()            -> dumps the current stack
     EnsureStackEmpty()     -> aborts if the stack is not empty (to be used as an `assert`)

  Get information about the object in the stack

     IsA(type, [i])         -> the object is of this type?
     Get<Type>([i])         -> get value and convert to C++ object
     GetArray<Type>([i])    -> get array and convert to C++ vector
     Pop([n=1])             -> pop n items from stack
     Pop<Type>()            -> pop the value at the top of the stack as a C++ type
     PopArray<Type>()       -> return a C++ vector from the array at the top of the stack

  Add things to the stack

     Push(value)            -> Push a immediate value into the stack
     PushGlobal(name)       -> Push a global into the stack
     
  Access a Lua object attribute

     HasAttr(name, [i])        -> return if object has attribute
     PushAttr(name, [i])       -> get object attribute and put it in the top of the stack
     GetAttr<Type>(name, [i])  -> return object attribute as a C++ object
     SetAttr(name, value, [i]) -> set object attribute

  Loop a table/array:

     ForEach(&f, [i])          -> class function `f` for each element on the table 
     ForEachKey(&f, [i])       -> class function `f` for each element on the table 
   
  Function calls:

     CallGlobalFunction<Type>(name, [parameters...])  -> call a function and returns the result
     CallMethod<Type>(name, [parameters...])          -> call a method and returns the result
     Call(nargs, nret, [i])                           -> call method at the top of the stack (low level)

  Immediate operations:
    
     Do<Type>(code)         -> execute Lua string and return the result as a C++ object
     Do(code)               -> execute Lua string and push the result into the stack


