F = setmetatable({}, { __index = function(t, k)
  local function_def = 'return function ' .. string.gsub(k, '%)', ') return ', 1) .. ' end'
  t[k] = assert(load(function_def))()
  return t[k]
end })

-- vim: ts=2:sw=2:sts=2:expandtab
