#include <iostream>
#include <string>
#include <vector>
#include <any>
#include <unordered_map>
#include <cassert>
#include <ctype.h>

struct Parser
{
    Parser() = default;

    ~Parser() = default;

    void Parse(int argc, char** argv)
    {
        m_params.reserve(argc);
        for (int i = 1; i < argc; i++)
        {
            auto param = std::string(argv[i]);
            m_params.emplace_back(param);
            m_params_iters.insert(std::make_pair(param, m_params.end() - 1));
        }
    }

    std::vector<std::string> m_params; 

    std::unordered_map<std::string, std::vector<std::string>::iterator> m_params_iters;
};

/**
   -n [num] : number of threads
   -t [sec] : seconds
   -c [number] : number of request
 */

void ParseClientArgv(int argc, char** argv)
{
    Parser p;
    p.Parse(argc, argv);
    auto& param_table = p.m_params_iters;
    auto vect_end = p.m_params.end();
    std::string get_param[] = {"-n", "-t", "-c"};
    int get_value[3] = {0};

    for (int i = 0; i < 3; i++)
    {
        auto iter = param_table.find(get_param[i]);
        if(iter != param_table.end())
        {
            assert(iter->second!= vect_end - 1);
            auto next = iter->second;
            next++;
            get_value[i] = atoi((*next).c_str());
        }    
    }
    for (auto i : get_value)
    {
        std::cout << i << std::endl;
    }
}

int main(int argc, char** argv)
{
    ParseClientArgv(argc, argv);
    return 0;
}
