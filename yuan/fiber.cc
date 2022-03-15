#include "fiber.h"
#include <atomic>
#include "config.h"
#include "macro.h"

namespace yuan {

static Logger::ptr g_system_logger = YUAN_GET_LOGGER("system");

// 以下两个为对协程的统计量
static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

// 分别用来记录当前在运行的协程和线程的主协程
static thread_local Fiber *t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

// 先约定协程的栈大小为1MB，之后可以通过配置文件修改
static ConfigVar<uint32_t>::ptr g_fiber_stack_size = 
    Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

// 自定义一个内存分配器，可能有多种实现，比如malloc，mmap等。看了侯捷老师的内存课后，这里可以优化
class MallocStackAllocator {
public:
    static void *Alloc(size_t size) {
        return malloc(size);
    }

    // 这里传size的原因是考虑到以后可能调用munmap，所以兼容
    static void Dealloc(void *vp, size_t size) {
        free(vp);
    }
};

// 增加灵活性，想改实现把等号右边改一下即可
using StackAllocator = MallocStackAllocator;

// 私有构造方法，只有主协程会使用这个方法
Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    // 协程相关，即获取上下文，可以man来了解
    if (getcontext(&m_ctx)) {
        YUAN_ASSERT2(false, "getContext");
    }

    ++s_fiber_count;
}

// 这个方法是真正构造工作的协程
Fiber::Fiber(std::function<void()> cb, size_t stackSize) : m_id(++s_fiber_id), m_cb(cb) {
    ++s_fiber_count;
    // 除了一些特殊任务需要大的栈空间，其他都用全局约定好的
    m_stacksize = stackSize ? stackSize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
        YUAN_ASSERT2(false, "getContext");
    }

    // 看man makecontext，切换上下文前的准备
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    
    makecontext(&m_ctx, MainFunc, 0);
}

Fiber::~Fiber() {
    --s_fiber_count;
    // 根据是否是主协程做不同的操作。主协程没有栈
    if (m_stack) {
        YUAN_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
        StackAllocator::Dealloc(&m_stack, m_stacksize);
    } else {
        YUAN_ASSERT(!m_cb);
        YUAN_ASSERT(m_state == EXEC);

        if (t_fiber == this) {
            SetThis(nullptr);
        }
    }

}

void Fiber::reset(std::function<void()> cb) {
    // 先assert，判断能否重置。主协程不能重置
    YUAN_ASSERT(m_stack);
    YUAN_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    // 下面转换context与构造方法一致
    if (getcontext(&m_ctx)) {
        YUAN_ASSERT2(false, "getContext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;
    
    makecontext(&m_ctx, MainFunc, 0);

    m_cb = cb;
    m_state = INIT;
}

void Fiber::swapIn() {
    SetThis(this);
    YUAN_ASSERT(m_state != EXEC);

    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        YUAN_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapOut() {
    SetThis(t_threadFiber.get());

    if (swapcontext(&m_ctx, &(t_threadFiber->m_ctx))) {
        YUAN_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber *fiber) {
    t_fiber = fiber;
}


Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }

    // 如果没有当前在运行的协程，则创建主协程并返回
    Fiber::ptr main_fiber(new Fiber);
    YUAN_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return main_fiber;
}
    
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    YUAN_ASSERT(cur);
    try {
        cur->m_cb();
        // 必须置为null，可能function里引用了智能指针，要减少引用计数
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception &e) {
        YUAN_LOG_ERROR(g_system_logger) << "Fiber Except: " << e.what();
        cur->m_state = EXCEPT;
    } catch (...) {
        YUAN_LOG_ERROR(g_system_logger) << "Fiber Except: ";
        cur->m_state = EXCEPT;
    }
}
}