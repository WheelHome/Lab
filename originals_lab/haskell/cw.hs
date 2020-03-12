-- code war
-- calnum 0 123 = 321
calnum x y =
  let p1 = x * 10 
      p2 = mod x 10
      p3 = round (y / 10)
  in calnum 0 p3
  
