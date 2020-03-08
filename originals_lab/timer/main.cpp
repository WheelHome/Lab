#include "timer.hpp"
#include <iostream>
#include <unistd.h>

int main()
{
    Timer timer;
    timer.Start();
    sleep(1);
    timer.Stop();
    std::cout << timer.S() << std::endl;

    timer.Start();
    for (int i = 0; i < 3; i++)
    {
        sleep(1);
        timer.Accumulate();
    }
    timer.Stop();
    std::cout << timer.S() << std::endl;
    return 0;
}
