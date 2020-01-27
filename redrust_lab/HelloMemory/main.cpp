#include "Allocator.h"
#include "CELLTimestamp.hpp"
#include <thread>
#include <memory>

const int tCount = 4;
void workFun(int index)
{

    char* data[1100];
    for(size_t i = 0 ; i < 1100; i++)
    {
        data[i] = new char[1 + i];
    }

    for(size_t i = 0 ; i < 1100; i++)
    {
        delete[] data[i];
    }
}

class ClassA
{
public:
    int num;
public:
    ClassA(/* args */)
    {
        std::cout << "ClassA" << std::endl;
    }
    
    ~ClassA()
    {
        std::cout << "~ClassA" << std::endl;
    }
};

void fun(std::shared_ptr<ClassA> pA)
{
    std::cout << "use_count=" << pA.use_count() << std::endl;
    pA->num++;
}

int main()
{
    /*
    //1
    char* data1 = new char [128];
    delete[] data1;

    //2
    char* data2 = new char;
    delete data2;

    //3
    char* data3 = new char [64];
    delete[] data3;*/

    //4}
     /*std::thread* t[tCount];
    for(int n = 0 ; n < tCount ; n++)
    {
        t[n] = new std::thread(workFun,n);
    }
    CELLTimestamp tTime;
    for(int n = 0 ; n < tCount ; n++)
    {
        t[n]->join();
        // t[n]->detach();
    }
    std::cout << tTime.getElapsedTimeInMilliSec() << std::endl;
    std::cout << "Hello,Main thread." << std::endl;
    for(int n = 0 ; n < tCount ; n++)
    {
        delete t[n];
    }*/
    int* a = new int;
    *a = 100;
    std::cout << "a=" << *a << std::endl;
    delete a;
    
    std::shared_ptr<int> b = std::make_shared<int>();
    *b = 100;
    std::cout << "b=" << *b << std::endl;

    ClassA* a1 = new ClassA();
    a1->num = 100;
    delete a1;

    std::shared_ptr<ClassA> a2 = std::make_shared<ClassA>();
    std::cout << "use_count=" << a2.use_count() << std::endl;
    a2->num = 100;
    std::cout << "a2=" << a2->num << std::endl;
    ClassA* a4 = a2.get();
    fun(a2);
    std::cout << "fun(a2)=" << a2->num << std::endl;
    std::cout << "use_count=" << a2.use_count() << std::endl;

    return 0;
}