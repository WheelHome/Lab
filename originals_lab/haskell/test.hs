-- 函数定义
tadd x y = x + y

-- if 表达式
tisodd x = if odd x then True else False

-- 用递归实现循环
tsum x = if x > 0 then x + tsum (x-1) else x
