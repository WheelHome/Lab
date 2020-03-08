#include <iostream>
#include <assert.h>

#define private public
#include "ccbuffer.hpp"

using namespace cc;

struct Test
{
    int size;
    int buffer[];
};

void Info(ccBuffer& buffer)
{
    std::cout << "size :" << buffer.m_buffer->size() << std::endl;
    std::cout << "use :" << buffer.m_use << std::endl;
    std::cout << "left :" << buffer.m_left << std::endl;
    std::cout << std::endl;
}


int main()
{
    ccBuffer buffer;
    size_t* size_info = nullptr;

    //3 int, 3 * 4 = 12
    auto t = buffer.New<Test, 2 * sizeof(int)>();
    Info(buffer);

    size_info = (size_t*) t;
    size_info--;
    std::cout << size_info << std::endl;

    assert(*size_info == 12);

    //========

    t = buffer.New<Test, 2 * sizeof(int)>();
    Info(buffer);

    size_info = (size_t*) t;
    size_info--;
    assert(*size_info == 12);

    //========

    bool ret = buffer.Delete(t);
    assert(ret && "fail to delete");

    t = buffer.New<Test, 2 * sizeof(int)>();
    Info(buffer);

    //========

    buffer.Delete(t);
    int* p = buffer.New<int>();
    assert(*p == 0);

    size_info = (size_t*) p;
    size_info--;
    assert(*size_info == 12);

    //========

    ccBuffer buffer2(10);
    auto w = buffer2.New<Test, 2 * sizeof(int)>();

    return 0;
}
