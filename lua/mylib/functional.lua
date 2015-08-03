function filter(table, func)
  local new_table = {}
  for _,v in ipairs(table) do
    if func(v) then new_table[#new_table+1] = v end
  end
  return new_table
end

function filter_tbl(table, func)
  local new_table = {}
  for k,v in pairs(table) do
    if func(k, v) then new_table[k] = v end
  end
  return new_table
end

function map(table, func)
  local new_table = {}
  for _,v in ipairs(table) do
    new_table[#new_table+1] = func(v)
  end
  return new_table
end

function map_tbl(table, func)
  local new_table = {}
  for k,v in pairs(table) do
    new_table[#new_table+1] = func(k, v)
  end
  return new_table
end

function partition(table, func)
  local true_tbl, false_tbl = {}, {}
  for _,v in ipairs(table) do
    if func(v) then 
      true_tbl[#true_tbl+1] = v 
    else
      false_tbl[#false_tbl+1] = v
    end
  end
  return true_tbl, false_tbl
end

function flatten(list)
  if type(list) ~= "table" then
    return {list}
  elseif list.is_a then
    return {list}
  end
  local flat_list = {}
  for _, elem in ipairs(list) do
    for _, val in ipairs(flatten(elem)) do
      flat_list[#flat_list + 1] = val
    end
  end
  return flat_list
end

function compact(table)
  return filter(table, function(i) return i ~= nil end)
end

function min(tbl, func)
  func = func or (function(x) return x end)
  local t, mn = tbl[1], func(tbl[1])
  for i=2,#tbl do
    local nn = func(tbl[i])
    if nn < mn then
      t = tbl[i]
      mn = nn
    end
  end
  return t
end

function max(tbl, func)
  func = func or (function(x) return x end)
  local t, mn = tbl[1], func(tbl[1])
  for i=2,#tbl do
    local nn = func(tbl[i])
    if nn > mn then
      t = tbl[i]
      mn = nn
    end
  end
  return t
end

function circular_next(tbl, value)
  if tbl[#tbl] == value then
    return tbl[1]
  else
    for i=1,#tbl-1 do
      if tbl[i] == value then return tbl[i+1] end
    end
  end
  error('Value ' .. inspect(value) .. ' not found in table.')
end

function contains(t, item)
  for _,v in ipairs(t) do
    if v == item then return true end
  end
  return false
end

function join(...)
  local ntbl = {}
  for _,tbl in ipairs{...} do
    for _,v in ipairs(tbl) do
      ntbl[#ntbl+1] = v
    end
  end
  return ntbl
end

function reverse(tbl)
  local ntbl = {}
  for i=#tbl,1,-1 do
    ntbl[#ntbl+1] = tbl[i]
  end
  return ntbl
end

function sort(tbl, f)
  local ntbl = shallow_copy(tbl)
  table.sort(ntbl, f)
  return ntbl
end

function sum(tbl)
  local v = tbl[1]
  for i=2,#tbl do
    v = v + tbl[i]
  end
  return v
end

function shallow_copy(tbl)
  local ntbl = {}
  for k,v in pairs(tbl) do 
    ntbl[k] = v 
  end
  return ntbl
end

function table_init(n, value)
  typecheck { n='number' }
  local ntbl = {}
  for i = 1,n do
    if type(value) == 'table' then
      ntbl[i] = shallow_copy(value)
    else
      ntbl[i] = value
    end
  end
  return ntbl
end

-- vim: ts=2:sw=2:sts=2:expandtab
