#include <functional>
#include <map>
#include <queue>
#include <memory>
#include <thread>

#include <string.h>
#include <ucontext.h>

#include <event.h>
#include <zmq.hpp>

using ZMQIniter = std::function<void(zmq::socket_t&)>;

class FiberCTX
{
private:
    //! 协程栈的大小
    enum
    {
        FIBER_DEFAULT_STACK_SIZE = 1024
    };

    //! 保存注册到协程中的函数
    struct Func
    {
        std::function<void()> m_task;
    };

    //! 协程上下文
    ucontext_t m_ctx;
    //! 协程栈
    char m_stack[FIBER_DEFAULT_STACK_SIZE];
    //! 切出的目标协程上下文
    FiberCTX *m_next_ctx;
    //! 保存函数
    Func *m_func;

public:

    FiberCTX() :
        m_next_ctx(nullptr), m_func(nullptr)
    {
        m_ctx.uc_stack.ss_sp = m_stack;
        m_ctx.uc_stack.ss_size = FIBER_DEFAULT_STACK_SIZE;
        m_ctx.uc_stack.ss_flags = 0;
        m_ctx.uc_link = nullptr;
    }

    ~FiberCTX()
    {
        if (m_func)
        {
            delete m_func;
        }
    }

    //! 运行task
    static void RunTask(Func *task)
    {
        task->m_task();
    }

    //! 设置task
    void SetTask(std::function<void()> task)
    {
        m_func = new Func;
        m_func->m_task = task;
    }

    //! 切换到目标协程
    void Swap(FiberCTX *ctx)
    {
        getcontext(&ctx->m_ctx);
        makecontext(&ctx->m_ctx, (void (*)(void)) RunTask, 1, ctx->m_func);
        ctx->m_next_ctx = this;
        swapcontext(&m_ctx, &ctx->m_ctx);
    }

    //! 恢复目标协程
    void Resume(FiberCTX *ctx)
    {
        swapcontext(&m_ctx, &ctx->m_ctx);
    }

    //! 切出
    void Yield()
    {
        swapcontext(&m_ctx, &m_next_ctx->m_ctx);
    }

    //! 重置协程栈
    void Reset()
    {
        bzero(m_stack, FIBER_DEFAULT_STACK_SIZE);
    }

};

struct CZSocket
{
    CZSocket(int32_t type) :
        m_ctx(nullptr), m_type(type)
    {
    }

    void SetCTX(FiberCTX *ctx)
    {
        m_ctx = ctx;
    }

    void Release()
    {
        if (m_ctx)
        {
            delete m_ctx;
        }
    }

    FiberCTX *m_ctx;
    zmq::socket_t m_socket;
    int32_t m_type;
};

//! @brief ZMQ池
class ZMQPool
{
public:
    ZMQPool()
    {
        m_release = [this](CZSocket *socket)
        {
            auto iter = m_zmq_pools.find(socket->m_type);
            if (iter != m_zmq_pools.end())
            {
                socket->Release();
                iter->second.emplace(socket);
            }
        };
    }

    ~ZMQPool()
    {
        for (auto item : m_zmq_pools)
        {
            while (!item.second.empty())
            {
                CZSocket *socket = item.second.front();
                delete socket;
                item.second.pop();
            }
        }
    }

    std::shared_ptr<CZSocket> GetZMQ(const int32_t &type)
    {
        auto iter = m_zmq_pools.find(type);
        if (iter != m_zmq_pools.end())
        {
            if (iter->second.empty())
            {
                //TODO extend the pool
            }
            auto socket = iter->second.front();
            iter->second.pop();
            return std::shared_ptr<CZSocket>(socket, m_release);
        }
        return nullptr;
    }

    void AddZMQPool(const int32_t &type, const ZMQIniter &initer,
                    const uint32_t &size)
    {
        m_initers[type] = initer;
        std::queue<CZSocket*> zmq_pool;
        for (uint32_t i = 0; i < size; i++)
        {
            CZSocket *socket = new CZSocket(type);
            initer(socket->m_socket);
            zmq_pool.emplace(socket);
        }
        m_zmq_pools.emplace(std::make_pair(type, std::move(zmq_pool)));
    }

private:
    std::map<int32_t, ZMQIniter> m_initers;
    std::function<void(CZSocket*)> m_release;
    std::map<int32_t, std::queue<CZSocket*>> m_zmq_pools;
};

struct CoEvent
{
    CoEvent();

    void Run();

    void Stop();

    static void CoCallback(evutil_socket_t, short, void *);

    static void SwitchCallback(evutil_socket_t, short, void *);

    static void InitREQ(zmq::socket_t& socket);

    void RunTask();

    void CoSend();

    void CoRead();

    event_base* m_ebase;
    event* m_coevent;
    FiberCTX m_main_fiber;
    FiberCTX* m_current_fiber;
    std::function<void()> m_task;
};

CoEvent::CoEvent()
    : m_ebase(nullptr),
      m_coevent(nullptr),
      m_current_fiber(nullptr)
{
    m_ebase = event_base_new();
    m_coevent = event_new(m_ebase, -1, EV_PERSIST, CoCallback, this);
    event_add(m_coevent, nullptr);
}

void CoEvent::Run()
{
    event_base_dispatch(m_ebase);
}

void CoEvent::Stop()
{
    event_base_free(m_ebase);
}

void CoEvent::CoCallback(evutil_socket_t, short, void* p)
{
    auto obj = static_cast<CoEvent*>(p);
    obj->RunTask();
}

void CoEvent::SwitchCallback(evutil_socket_t, short, void* p)
{
    //todo
}

void CoEvent::RunTask()
{
    m_current_fiber = new FiberCTX;
    m_current_fiber->SetTask(m_task);
    m_main_fiber.Swap(m_current_fiber);
    m_main_fiber.Reset();
}

CoEvent co_event;

void Start()
{
    co_event.Run();
}

int main()
{
    return 0;
}
