#ifndef CCBUFFER_H_
#define CCBUFFER_H_

#include <string>
#include <map>
#include <queue>
#include <unistd.h>

namespace cc
{

class ccBuffer
{
public:
    explicit ccBuffer(const size_t& pages = 1)
        : m_buffer_size(pages * getpagesize())
        , m_use(0)
        , m_left(m_buffer_size)
        , m_max_block(0)
    {
        m_buffers_vect.emplace_back(std::string(m_buffer_size, 0));
        m_buffer = &m_buffers_vect[0];
    }

    ~ccBuffer() = default;

    template<typename TYPE, size_t extern_size = 0, typename... Args>
    TYPE* New(Args... args)
    {
        TYPE* obj = nullptr;
        size_t object_size = GetSize<TYPE>(extern_size);
        do
        {
            if (object_size > m_max_block)
            {
                break;
            }

            if ((obj = (TYPE*)GetBlockFromPool(object_size)) != nullptr)
            {
                return obj;
            }    

            for(auto& item : m_freeblock_pool)
            {
                if (item.first < object_size)
                {
                    continue;
                }
                if((obj = (TYPE*)item.second.Get()) != nullptr)
                {
                    return obj;
                }
            }
        } while(0);
        

        if (object_size > m_left)
        { 
            CreateBuffer();
        }

        char* p = &m_buffer->front();

        auto size_info = reinterpret_cast<size_t *>(p + m_use);
        *size_info = sizeof(TYPE) + extern_size;

        obj = new (p + m_use + sizeof(size_t)) TYPE (args...);
        UpdateBuffer(object_size);
        return obj;
    }

    bool Delete(void* obj)
    {
        auto size_info = reinterpret_cast<size_t*>(obj);
        size_info--;
        size_t size = Align(*size_info + sizeof(size_t));
        if (size > m_max_block)
        {
            m_max_block = size;
        }
        auto iter = m_freeblock_pool.find(size);
        if (iter == m_freeblock_pool.end())
        {
            FreeBlocks blocks;
            blocks.Append((char*)obj);
            m_freeblock_pool.insert(std::make_pair(size, blocks));
            return true;
        }
        iter->second.Append((char*)obj);
        return true;
    }

private:
    void CreateBuffer()
    {
        m_buffers_vect.emplace_back(std::string(m_buffer_size, 0));
        m_left = m_buffer_size;
        m_use = 0;
        m_buffer = &m_buffers_vect.back();
    }

    template<typename TYPE>
    size_t GetSize(const size_t& ex)
    {
        return Align(sizeof(size_t) + sizeof(TYPE) + ex);
    }

    void* GetBlockFromPool(const size_t& size)
    {
        auto iter = m_freeblock_pool.find(size);
        if (iter == m_freeblock_pool.end())
        {
            return nullptr;
        }
        void* block = iter->second.Get();
        bzero(block, size);
        return block;
    }

    size_t Align(const size_t& size)
    {
        return 8 - (size & 7) + size;
    }

    void UpdateBuffer(const size_t& size)
    {
        m_use += size;
        m_left = m_buffer_size - m_use;
    }

    const size_t m_buffer_size;
    size_t m_use;
    size_t m_left;
    std::vector<std::string> m_buffers_vect;
    std::string* m_buffer;

    class FreeBlocks
    {
    public:
        FreeBlocks()
            : m_count(0)
        {
        }

        void Append(char* block)
        {
            m_queue.push(block);
            m_count++;
        }

        char* Get()
        {
            if (m_queue.empty())
            {
                return nullptr;
            }
            m_count--;
            char* block = m_queue.front();
            m_queue.pop();
            m_count--;
            return block;
        }

    private:
        int m_count;
        std::queue<char*> m_queue;
    };

    std::map<size_t, FreeBlocks> m_freeblock_pool;
    size_t m_max_block;
};

};
#endif
