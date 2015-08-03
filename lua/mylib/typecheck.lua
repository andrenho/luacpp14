function typecheck(values)
  if DEBUG then
    local i = 1
    while true do
      local k, v = debug.getlocal(2, i)
      if not k then 
        break 
      end
      if values[k] then
        local ok = true
        if type(values[k]) == 'string' then
          if type(v) ~= values[k] then ok = false end
        elseif type(values[k] == 'table') and values[k].is_a then
          if type(v) ~= 'table' or not v.is_a or not v.is_a[values[k]] then ok = false end
        else
          error('Invalid type definition '..inspect(values[k]), 2)
        end
        if not ok then
          local typedesc = (type(values[k]) == 'string') and values[k] or values[k]:classname()
          error('Type error: '..k..'='..inspect(v)..' (should be a '..typedesc..')', 2)
        end
      end
      i = i+1
    end
  end
end

function tablecheck(values)
  if DEBUG then
    local table = table.remove(values, 1)
    if type(table) ~= 'table' then
      error('"table" should be a table, not '..inspect(table))
    end
    local ok = true
    for key,tp in pairs(values) do
      if table[key] == nil then
        error('Type error: key `'..key..'` not found in table '..inspect(table))
      elseif type(table[key]) == 'table' and table[key].is_a then
        if not table[key].is_a[tp] then 
          error('Type error: '..key..'='..inspect(table[key])..' (should be a '..tp:classname()..')', 2)
        end
      elseif type(table[key]) ~= tp then
        error('Type error: '..key..'='..inspect(table[key])..' (should be a '..tp..')', 2)
      end
    end
  end
end

-- vim: ts=2:sw=2:sts=2:expandtab
