#ifndef TQUEUE
#define TQUEUE

#include <deque>
#include <mutex>

template<typename T>
class T_queue
{
private:
    std::deque<T> deq;
    std::mutex mut;
public:
    T_queue() = default;
    
    T&& pop_front()
    {
        mut.lock();
        if(deq.size() == 0)
        {
            mut.unlock();
            throw "queue is empty!";
        }
        T temp = std::move(deq.front());
        deq.pop_front();
        mut.unlock();
        return std::move(temp);
    }

    void push_back(T&& temp)
    {
        mut.lock();
        deq.push_back(std::move(temp));
        mut.unlock();
    }
};


#endif