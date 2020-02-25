-- 函数定义
tadd x y = x + y

-- if 表达式
tisodd x = if odd x then True else False

tif1 x = if x > 100 then x else 100

tif2 x = ( if x > 100 then x else 100 ) + 1

-- 用递归实现循环
tsum x = if x > 0 then x + tsum (x-1) else x

-- list
appendList x y = x ++ y

len :: Num b => [a] -> b
len (_:x) = len x + 1
len [] = 0

-- pattern
lucky :: Integral a => a -> String  
lucky 7 = "LUCKY NUMBER SEVEN!"  
lucky x = "Sorry, you're out of luck, pal!" 

check :: Integral x => x -> String
check x 
  | xx < 10 = "x * x  < 10"
  | xx > 10 = "x * x > 10"
  | otherwise = "x * x = 10"
  where xx = x * x

maxnum :: Integral a => [a] ->a 
maxnum [x] = x
-- maxnum (x:xs) | x > max = x | otherwise = max  where max = maxnum xs
maxnum (x:xs)  = max x (maxnum xs)

-- quickly sort
quickSort :: (Ord a)=>[a]->[a]
quickSort [] =[]
quickSort (x:xs)=
  let smaller=[a|a<-xs,a<=x]
      bigger=[a|a<-xs,a>x]
  in quickSort smaller ++[x]++quickSort bigger
