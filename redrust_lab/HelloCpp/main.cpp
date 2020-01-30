#include <functional>
#include <iostream>
#include <list>
//functional
int funA(int a,int b)
{
    std::cout << "funA" << std::endl;
    return 1;
}
//lambda
int	main(int argc, char **argv)
{
    std::list<int> a;
    std::function<int(int,int)> call = funA;
    int temp = call(5,6);

    std::function<int(int,int)> call2;
    int n = 5;
    //anonymous functioj
    // return -> type
    call2 = [n/*extern variables capture list*/](/*parameter list*/int a,int b) -> int
    {
        //function body
        std::cout << n << std::endl;
        std::cout << a << std::endl;
        std::cout << b << std::endl;
        std::cout << "funB" << std::endl;
        return 30;
    };
    int temp2 = call2(10,20);
    system("pause");
    return 0;
}
