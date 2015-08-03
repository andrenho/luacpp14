function string.starts_with(self,w)
  typecheck { self='string', w='string' }
  return self:sub(1, w:len()) == w
end

-- vim: ts=2:sw=2:sts=2:expandtab
