#ifndef INCL_SYNC_QUEUE_H
#define INCL_SYNC_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T> class Sync_Queue
{
    std::mutex m;
    std::condition_variable cv;
    std::queue<T> lst;

    public:
    template <class... Whatever>
    void push(Whatever... x...)
    {
        {
            std::lock_guard<mutex> l(m);
            lst.push(x...);
        }
        cv.notify_one();
    }

    bool empty() const { return lst.empty(); }

    T pop()
    {
        {
            auto cond = [&lst]() -> bool { return not lst.empty(); };
            std::unique_lock<mutex> l(m);
            cv.wait(l, cond);
        }

        std::lock_guard<mutex> l(m);
        auto v = lst.front();
        lst.pop();
        return v;
    }
};

#endif /* INCL_SYNC_QUEUE_H */
