-- TODO
--   1. Organize last  [DONE]
--   2. Organize getinfo  [DONE]
--   3. 'l' list
--   4. 'up' and 'down'

local src = {}
local last = {
  step = '',
  command = '',
  input = '',
  depth = 0,
  place = '',
  list = 0,
}

-- read source file
local function load_source(file)
  if src[file] then
    return src[file]
  else
    local lines = {}
    local f = io.open(file, "r")
    if not f then error('Could not open file '..file) end
    while true do
      local line = f:read()
      if not line then
        break
      end
      lines[#lines+1] = line
    end
    src[file] = lines
    return lines
  end
end


local function brk_debug(from_brk)
  assert(debug.gethook() == nil)

  -- load stack info
  local stack = {}
  while true do
    local info = debug.getinfo(#stack+1, 'nSl')
    if not info then break end
    stack[#stack+1] = info
  end
  
  -- find stack level
  local level = 1
  while stack[level].source:starts_with('@lua') or stack[level].what ~= 'Lua' do
    level = level+1
    if not stack[level] then
      return  -- ended source, continue execution
    end
  end

  -- find stack position
  local input = ''
  local info = stack[level]
  local depth = #stack
  local src
  if from_brk then depth = depth-1 end

  -- if the last command was a step, and we're still in the same source line, keep executing
  if last.command == 'step' and last.step == info.short_src..':'..info.currentline then
    input = 's'
    goto skip_input
  -- if the last command was a next, we keep running until the same depth or higher
  elseif last.command == 'next' and depth > last.depth then
    input = 'n'
    goto skip_input
  end

  -- load source file
  src = load_source(info.short_src)

  -- print debug info
  if last.place ~= info.source..':'..(info.name or '') then
    print((info.namewhat ~= '' and info.namewhat or info.what)..' '..(info.name or '?')..'() at '..info.short_src..':'..info.currentline)
  end
  if last.step ~= info.short_src..':'..info.currentline then
    print(string.format('%d\t%s', info.currentline, src[info.currentline]))
  end

  --
  -- read input from user
  --
  input = readline('(luax) ')
  if input == '' then 
    input = last.input 
  end
  last.input = input
  last.depth = depth
  last.place = info.source..':'..(info.name or '')
  last.step = info.short_src..':'..info.currentline

  ::skip_input::
  local last_command = last.command
  last.command = ''

  -- quit or continue
  if input == 'q' then
    os.exit(0)
  -- continue
  elseif input == 'c' then
    return
  -- step
  elseif input == 's' then
    last.command = 'step'
    debug.sethook(function() debug.sethook(); brk_debug() end, 'l')
    return
  -- step
  elseif input == 'n' then
    last.command = 'next'
    debug.sethook(function() debug.sethook(); brk_debug() end, 'l')
    return
  -- print expression
  elseif input:starts_with('p ') then
    -- add locals to environment
    local env = shallow_copy(_ENV)
    local j = 1
    while debug.getlocal(level, j) do
      local k,v = debug.getlocal(level, j)
      env[k] = v
      j = j+1
    end
    -- print expression
    local cmd = input:sub(3)
    local ret = table.pack(pcall(load('return '..cmd, nil, 't', env)))
    for i=2,#ret do p(ret[i]) end
    last.command = 'print'
  elseif input == 'l' then
    local n
    if last_command == 'list' then
      n = last.list
    else
      n = max { info.currentline-5, 1 }
    end
    last.list = n+11
    for i=n,n+10 do
      if src[i] then print(string.format('%d\t%s', i, src[i])) end
    end
    last.command = 'list'
  -- error
  elseif input == 'h' then
    print('p  --  inspect/execute expression')
    print('l  --  list source code')
    print('s  --  step')
    print('n  --  next (skip function calls)')
    print('c  --  continue')
    print('h  --  help')
    print('q  --  quit')
    last.command = 'help'
  else
    print('Undefined command: "'..input..'". Type "h" for help.')
  end

  brk_debug()
end


if DEBUG then
  function brk()
      debug.sethook(function() debug.sethook(); brk_debug(true) end, 'c')
  end
end

-- vim: ts=2:sw=2:sts=2:expandtab
