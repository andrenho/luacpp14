function math.trunc(v)
  return math.floor(v) + (v < 0 and 1 or 0)
end

-- vim: ts=2:sw=2:sts=2:expandtab
