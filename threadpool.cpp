#include "threadpool.h";

ThreadPool::ThreadPool(int m_thred_size):m_mutex(),m_cond(),m_started(false){
    m_thread_size = m_thred_size;
    start();
} 
void ThreadPool:: start(){
    assert(!m_started);
    assert(threads.empty());
    m_started = true;
    for(int i = 0; i<m_thread_size; i++){
        thread *t = new thread(bind(&ThreadPool::ThreadLoop, this));
        // threads.push_back(new thread(bind(&ThreadPool::ThreadLoop, this)));
        threads.push_back(t);
    }

}

ThreadPool::~ThreadPool()
{
    if(m_started)
    {
        end();
    }
}

void ThreadPool:: end(){
    unique_lock<mutex> lock(m_mutex);
    m_started = false;
    //清除掉线程空间
    for(auto i = threads.begin(); i!=threads.end(); i++){
        (*i)->join();
        delete *i;
    }
    threads.clear();
}

void ThreadPool::ThreadLoop(){
    while(m_started){
        func task = take();
        if(task)
            task();
    }
}

ThreadPool::func ThreadPool::take(){
    unique_lock<mutex> lock(m_mutex);
    while(task_queue.empty() && m_started){
        m_cond.wait(lock);
    }
    func task;
    if(!task_queue.empty() && m_started){
        task = task_queue.top().second;
        task_queue.pop();
    }
    return task; 
}

void ThreadPool::AddTask(const task& t){
    unique_lock<mutex> lock(m_mutex);
    task_queue.push(t);
    m_cond.notify_one();
}

