local function is_array(t)
  for k,_ in pairs(t) do
    if type(k) ~= 'number' then return false end
  end
  return true
end

local parse_table  -- forward declaration

local function value(v, max_depth, cur_depth)
  if v == nil then
    return 'nil'
  elseif type(v) == 'string' then
    return "'" .. tostring(v) .. "'"
  elseif type(v) == 'table' then
    return parse_table(v, max_depth, cur_depth)
  elseif type(v) == 'number' and v ~= math.floor(v) then  -- floating point
    return string.format('%.3f', v)
  elseif type(v) == 'function' then
    return '[function]'
  else
    return tostring(v)
  end
end

parse_table = function(v, max_depth, cur_depth)
  if cur_depth == max_depth then
    if is_array(v) then
      return '{' .. (#v > 0 and '#'..tostring(#v) or '') .. '}'
    else
      if v.inspect then
        return '{ ' .. v:inspect() .. ' }'
      else
        return '{ ' .. (v.classname and (v:classname() or '?') or '') .. '... }'
      end
    end
  else
    if is_array(v) then
      return '{ ' .. table.concat(map(v, function(w) 
        return value(w, max_depth, cur_depth+1)
      end), ', ') .. ' }'
    else
      if v.inspect then
        return '{ ' .. v:inspect() .. ' }'
      else
        return '{ ' .. 
          ((not v.class) and '@' or '') ..
          (v.classname and (v:classname() or '?') .. ': ' or '') ..   -- name of the class
          -- values
          table.concat(
            map_tbl(
              filter_tbl(v, 
                function(k,v) 
                  return k ~= 'class' and k ~= 'class_desc' and k ~= 'is_a' and k ~= '__index' and k ~= '_init' and k ~= 'classname' --and type(v) ~= 'function'
                end),
              function(k, v2)
                local key = (type(k) == 'string' and k or ('[' .. value(k, 1, 0) .. ']'))
                local val = value(v2, max_depth, cur_depth+1)
                return key .. '=' .. val
              end), 
            ', ') .. ' }'
      end
    end
  end
end

function inspect(...)
  return table.concat(map({...}, 
    function(v) return value(v, 2, 0) end), '\t')
end

function p(...)
  io.write(inspect(...))
  io.write("\n")
  io.flush()
end

-- vim: ts=2:sw=2:sts=2:expandtab
