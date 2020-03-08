#ifndef REF_H_
#define REF_H_

#include <atomic>

struct Ref
{
    Ref() : m_ref(1) {}

    void Decrease()
    {
        m_ref--;
        if (!m_ref)
        {
            Delete(this);
        }
    }

    void Increase()
    {
        m_ref++;
    }

    virtual void Delete(void*) = 0;

    virtual ~Ref() = default;

    std::atomic<int> m_ref;
};

#endif
