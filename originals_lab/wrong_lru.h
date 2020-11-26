// wrong lru cache, will crash
#ifndef _LRU_H_
#define _LRU_H_
#include <memory>
#include <limits>
#include <vector>
#include <ctime>
#include <set>
#include <map>
#include <algorithm>
#include <list>

template<typename T>
struct optional
{
    optional(const optional&) = delete;
    optional& operator=(const optional&) = delete;

    optional() {}

    optional(const T& data)
    {
        m_data = std::make_shared<T>(data);
    }

    optional(optional&& op)
    {
        m_data =  std::move(op.m_data);
    }

    optional& operator=(optional&& op)
    {
        m_data = std::move(op.m_data);
        return *this;
    }

    operator bool()
    {
        return m_data != nullptr;
    }

    T& GetDataRef()
    {
        return *m_data;
    }

    T GetData()
    {
        return *m_data;
    }

    std::shared_ptr<T> m_data;
};

template<typename T>
struct TimerItem
{
    TimerItem(const T& data, int64_t cycle)
            : m_data(data)
            , m_cycle(cycle)
    {}

    T m_data;
    int64_t m_cycle;
};

template<typename T>
class Timer
{
public:
    Timer()
            : m_cur_index(0)
            , m_last(std::time(nullptr))
    {
    }

    ~Timer() = default;

    void Add(T data, int64_t sec)
    {
        if (sec <= 0)
        {
            return;
        }
        int64_t cycle = sec / 60;
        int64_t left = sec % 60;

        int64_t pos = (m_cur_index + left) % 60;
        auto vec_ptr = Index(pos);
        if (!vec_ptr)
        {
            return;
        }
        vec_ptr->push_back(TimerItem<T>(data, cycle));
        m_last = std::time(nullptr);
    }

    std::vector<TimerItem<T>>* Index(size_t index)
    {
        if (index > 60 || index < 0)
        {
            return nullptr;
        }
        return &m_timer[index];
    }

    void NextIndex()
    {
        m_cur_index++;
        m_cur_index %= 60;
    }

    std::vector<TimerItem<T>> Handle()
    {
        std::vector<TimerItem<T>> expire_item;
        int64_t diff = std::time(nullptr) - m_last;
        for (; diff > 0; diff--)
        {
            auto vec_ptr = Index(m_cur_index);
            if (!vec_ptr)
            {
                continue;
            }
            for (auto& data : *vec_ptr)
            {
                data.m_cycle--;
            }
            vec_ptr->erase(std::remove_if(vec_ptr->begin(), vec_ptr->end(),
                                          [&](const TimerItem<T>& item)->bool {
                                              if (item.m_cycle <= 0)
                                              {
                                                  expire_item.push_back(item);
                                              }
                                              return item.m_cycle <= 0;
                                          }), vec_ptr->end());
        }
        return expire_item;
    }

    std::set<T> Clear()
    {
        std::set<T> keys;
        for (auto& data : m_timer)
        {
            for (auto& key : data)
            {
                keys.emplace(key.m_data);
            }
        }
        return keys;
    }

private:
    std::array<std::vector<TimerItem<T>>, 60> m_timer;
    size_t m_cur_index;
    int64_t m_last;
};

class ExpireKeyMgr
{
public:
    ExpireKeyMgr()
            : m_update_timeout(0)
            , m_update_timeout_count(0)
    {
    }

    void AddUpdateTimeoutCount()
    {
        m_update_timeout_count++;
        if (m_update_timeout_count > std::numeric_limits<int64_t>::max() - 100)
        {
            m_update_timeout = 0;
        }
    }

    int64_t GetUpdateTimeoutCount()
    {
        return m_update_timeout;
    }
    
    bool IsExpire(const std::string& key)
    {
        auto update_expire_keys = m_update_timer.Handle();
        auto expire_keys = m_expire_timer.Handle();

        for (auto& expire : update_expire_keys)
        {
            auto iter = m_expire_keys.find(expire.m_data);
            if (iter == m_expire_keys.end())
            {
                continue;
            }
            iter->second = eum_no_update;
            AddUpdateTimeoutCount();
        }

        for(auto& expire : expire_keys)
        {
            m_expire_keys[expire.m_data] = eum_no_update;
        }
        auto iter = m_expire_keys.find(key);
        if (iter == m_expire_keys.end() || iter->second == eum_updating)
        {
            return false;
        }

        iter->second = eum_updating;
        m_update_timer.Add(key, m_update_timeout);
        return true;
    }

    void AddExpireKey(const std::string& key, int64_t expire_time)
    {
        m_expire_keys.erase(key);
        m_expire_timer.Add(key, expire_time);
    }

    void SetAllKeyExpire()
    {
        m_update_timer.Clear();
        auto expire_key = m_expire_timer.Clear();
        for (auto& key : expire_key)
        {
            m_expire_keys[key] = eum_no_update;
        }
    }

    void Clear()
    {
        m_update_timer.Clear();
        m_expire_keys.clear();
        m_expire_timer.Clear();
    }

    void SetUpdateTimeout(int64_t sec)
    {
        m_update_timeout = sec;
    }

private:
    enum eum_key_status
    {
        eum_no_update, 
        eum_updating 
    };
    Timer<std::string> m_expire_timer;
    Timer<std::string> m_update_timer;
    std::map<std::string, eum_key_status> m_expire_keys;
    int64_t m_update_timeout;
    int64_t m_update_timeout_count;
};

class BaseCache
{
public:
    BaseCache() = default;
    virtual ~BaseCache() = default;
    virtual bool HasKey(const std::string& key) = 0;
    virtual void Pop(const std::string& key) = 0;
    virtual void Resize(size_t s) = 0;
    virtual void Clear() = 0;
    virtual size_t GetSize() = 0;
    virtual void SetExpireTime(int64_t hour, int64_t min, int64_t sec) {}
    virtual std::string GetStatus() {return "";}
    virtual void GetHitInfo(uint64_t& hit, uint64_t& lose) {};
    ExpireKeyMgr& GetExpireKeyMgr() {return m_expire_mgr;}
protected:
    ExpireKeyMgr m_expire_mgr;
};

template<typename T>
class Cache :
        public BaseCache
{
public:
    Cache() = default;
    ~Cache() override = default;
    virtual void Push(const std::string& key, const T& data, int64_t expire_sec) = 0;
    virtual optional<T> Get(const std::string& key) = 0;
};

template<class T, size_t size = 64>
class LRU
        : public Cache<T>
{
public:
    LRU()
            : m_size(size)
            , m_expire_time(0)
            , m_hit(0)
            , m_lose(0)
    {}

    virtual ~LRU() = default;

    void Hit()
    {
        if (m_hit >= std::numeric_limits<uint64_t>::max() - 100)
        {
            m_hit = 0;
        }
        m_hit++;
    }

    void Lose()
    {
        if (m_lose >= std::numeric_limits<uint64_t>::max() - 100)
        {
            m_lose = 0;
        }
        m_lose++;
    }

    void Push(const std::string& key, const T& data, int64_t expire_sec = 0)
    {
        auto find_iter = m_data_table.find(key);
        if (find_iter != m_data_table.end())
        {
            m_data_list.push_front(*(find_iter->second));
            m_data_list.front()->m_data = std::make_shared<T>(data);

            m_data_list.erase(find_iter->second);
            m_data_table[key] = m_data_list.begin();
            return;
        }

        if (m_data_list.size() >= m_size)
        {
            Pop();
        }

        std::shared_ptr<Data> data_ptr = std::make_shared<Data>();
        data_ptr->m_key = key;
        data_ptr->m_data = std::make_shared<T>(data);

        m_data_list.push_front(data_ptr);
        m_data_table[key] = m_data_list.begin();

        if (expire_sec)
        {
            BaseCache::m_expire_mgr.AddExpireKey(key, expire_sec);
            return;
        }

        BaseCache::m_expire_mgr.AddExpireKey(key, m_expire_time);
    }
    
    optional<T> Get(const std::string& key)
    {
        auto iter = m_data_table.find(key);
        if (iter == m_data_table.end())
        {
            Lose();
            return optional<T>();
        }

        if (BaseCache::m_expire_mgr.IsExpire(key))
        {
            Lose();
            return optional<T>();
        }

        m_data_list.push_front(*(iter->second));
        m_data_list.erase(iter->second);
        m_data_table[key] = m_data_list.begin();

        Hit();
        return *(m_data_list.front()->m_data);
    }
    
    void Pop(const std::string& key)
    {
        auto iter = m_data_table.find(key);
        if (iter == m_data_table.end())
        {
            return;
        }
        m_data_list.erase(iter->second);
        m_data_table.erase(key);
    }

    void Pop()
    {
        if (!m_data_list.empty())
        {
            Pop(m_data_list.back()->m_key);
        }
    }

    void Resize(size_t s)
    {
        m_size = s;
        while(m_data_table.size() > m_size)
        {
            Pop();
        }
    }

    void Clear()
    {
        m_data_table.clear();
        m_data_list.clear();
        BaseCache::m_expire_mgr.Clear();
    }

    size_t GetSize()
    {
        return m_data_table.size();
    }

    virtual void SetExpireTime(int64_t hour, int64_t min, int64_t sec)
    {
        m_expire_time = hour * 3600 + min * 60 * sec;
    }

    virtual bool HasKey(const std::string& key)
    {
        return m_data_table.find(key) != m_data_table.end();
    }

    virtual void GetHitInfo(uint64_t& hit, uint64_t& lose)
    {
        hit = m_hit;
        lose = m_lose;
    }

    void CheckIter()
    {
        assert(m_data_list.size() == m_data_table.size());
        return;
        for (auto & iter : m_data_table)
        {
            std::cout << "cur :" <<  iter.second._M_node << std::endl;
            std::cout << "prev : " << iter.second._M_node->_M_prev << std::endl;
            std::cout << "next : " << iter.second._M_node->_M_next << std::endl;
        }
        std::cout << std::endl;
    }
private:
    struct Data
    {
        std::string m_key;
        std::shared_ptr<T> m_data;
    };
    using list_iter = typename std::list<std::shared_ptr<Data>>::iterator;

    std::list<std::shared_ptr<Data>> m_data_list;
    std::map<std::string, list_iter> m_data_table;
    size_t m_size;
    int64_t m_expire_time;
    uint64_t m_hit;
    uint64_t m_lose;
};

#endif
