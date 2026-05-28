#pragma once

#include <functional>
#include <mutex>
#include <vector>
#include <atomic>
#include <queue>
#include <memory>
#include <future>

#include <thread>

class ThreadPool{

protected:
    struct TaskFunc
    {
        TaskFunc(){};
        std::function<void()> m_func;
    };
    typedef std::shared_ptr<TaskFunc> TaskFuncPtr;

public:

ThreadPool():m_threadNum(1),m_bTerminate(false){};

virtual ~ThreadPool(){
    stop();
};

bool init(size_t num){
    std::unique_lock<std::mutex> lock(m_mutex);
    if(!m_threads.empty()){
        return false;
    }
    m_threadNum = num;
    return true;
};


size_t getThreadNum(){
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_threads.size();
};


size_t getJobNum(){
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_tasks.size();
};


bool stop(){
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_bTerminate = true;
        m_condition.notify_all();
    }

    for(size_t i = 0; i< m_threads.size();i++){
        if(m_threads[i]->joinable()){
            m_threads[i]->join();
        }
        delete m_threads[i];
        m_threads[i] = nullptr;
    }

    std::unique_lock<std::mutex> lock(m_mutex);
    m_threads.clear();
    return true;
};

bool start(){
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_threads.empty()){
        return false;
    }

    for(size_t i = 0; i< m_threadNum;i++){
        m_threads.push_back(new std::thread(&ThreadPool::run,this));
    }
    return true;
};

template <class F, class... Args>
auto exec(F&& f, Args&&... args) ->std::future<decltype(f(args...))>{
    using RetType = decltype(f(args...));
    auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    TaskFuncPtr fPtr = std::make_shared<TaskFunc>();
    fPtr->m_func = [task](){
        (*task)();
    };

    std::unique_lock<std::mutex> lock(m_mutex);
    m_tasks.push(fPtr);
    m_condition.notify_one();
    return task->get_future();
};


bool waitForAllDone(){
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_tasks.empty()) return true;

    m_condition.wait(lock, [this]{return m_tasks.empty();});
    return true;
};


protected:
bool get(TaskFuncPtr& task){
    std::unique_lock<std::mutex> lock(m_mutex);
    if(m_tasks.empty()){
        m_condition.wait(lock,[this]{return m_bTerminate || !m_tasks.empty();});
    }

    if(m_bTerminate) return false;

    if(!m_tasks.empty()){
        task = std::move(m_tasks.front());
        m_tasks.pop();
        return true;
    }
    return false;

};

bool isTerminate(){return m_bTerminate;};

void run(){
    while(!isTerminate()){
        TaskFuncPtr task;
        bool ok=get(task);
        if(ok){
            ++m_atomic;
            try{
                task->m_func();
            }catch(...){
                throw;
            }
            m_atomic.fetch_sub(1, std::memory_order_relaxed);
            std::unique_lock<std::mutex> lock(m_mutex);
            if(m_atomic.load(std::memory_order_relaxed) == 0 && m_tasks.empty()){
                m_condition.notify_all();
            }
        }
    }
};

protected:
    std::queue<TaskFuncPtr>   m_tasks;
    std::vector<std::thread*> m_threads;
    std::mutex                m_mutex;
    std::condition_variable   m_condition;
    size_t           m_threadNum;
    bool             m_bTerminate;
    std::atomic<int> m_atomic{0};
};
