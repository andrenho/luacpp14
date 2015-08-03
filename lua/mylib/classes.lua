function class(...)
  -- cls is the new class
  local cls, bases = {}, {...}
  -- copy from base class
  for i, base in ipairs(bases) do
    for k, v in pairs(base) do
      cls[k] = v
    end
  end
  -- set __index and is_a
  cls.__index, cls.is_a = cls, {[cls] = true}
  for i, base in ipairs(bases) do
    for c in pairs(base.is_a) do
      cls.is_a[c] = true
    end
    cls.is_a[base] = true
  end
  -- set __call
  setmetatable(cls, { 
    __call = function(c, ...)
      local instance = setmetatable({}, c)
      instance.class = cls
      local init = instance.__init
      if init then init(instance, ...) end
      return instance
    end
  })
  -- return class
  cls.classname = function(self)
    if self.class_desc == nil then
      for k,v in pairs(_G) do
        if v == self.class or v == self then
          self.class_desc = k
          break
        end
      end
    end
    return self.class_desc
  end
  return cls
end

function static_class()
  local cls = {}
  cls.is_a = { [cls] = true }
  cls.classname = function(self)
    if self.class_desc == nil then
      for k,v in pairs(_G) do
        if v == self.class or v == self then
          self.class_desc = k
          break
        end
      end
    end
    return self.class_desc
  end
  return cls
end

-- vim: ts=2:sw=2:sts=2:expandtab
