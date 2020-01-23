#include <iostream>
#include <string>
#include <vector>
#include <any>
#include <unordered_map>
#include <cassert>
#include <ctype.h>

template<typename T>
struct Transform
{
};

template<>
struct Transform<int>
{
    static int To(const std::string& s)
    {
        return atoi(s.c_str());
    }
};

template<>
struct Transform<std::string>
{
    static std::string To(const std::string& s)
    {
        return s;
    }
};

class Parser
{
public:
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

    bool HasParam(const std::string& key)
    {
        auto iter = m_params_iters.find(key);
        return iter != m_params_iters.end();
    }

    template<typename T>
    std::optional<T> Param(const std::string& key)
    {
        auto iter = m_params_iters.find(key);
        if (iter == m_params_iters.end() || iter->second == m_params.end() - 1)
        {
            return {};
        }
        auto next = iter->second;
        next++;
        return Transform<T>::To(*next);
    }
private:
    std::vector<std::string> m_params;
    std::unordered_map<std::string, std::vector<std::string>::iterator> m_params_iters;
};

/**
 * parse the argument
 * -n [num] : number of threads
 * -t [sec] : seconds
 * -c [number] : number of request
 */

int main(int argc, char** argv)
{
    //ParseClientArgv(argc, argv);
    Parser p;
    p.Parse(argc, argv);
    std::string params[] = {"-n", "-t", "-c"};
    int values[3] = {0};

    for (int i = 0; i < 3; i++)
    {
        if (p.HasParam(params[i]))
        {
            auto value = p.Param<int>(params[i]);
            if (value)
            {
                values[i] = value.value();
            }
        }
    }

    for(auto i : values)
    {
        std::cout << i << std::endl;
    }
    return 0;
}
