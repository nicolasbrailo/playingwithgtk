#ifndef INCL_SYNC_QUEUE_H
#define INCL_SYNC_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class Sync_Queue
{
    bool signal_end;
    std::mutex m;
    std::condition_variable cv;
    std::queue<T> lst;

    public:
    Sync_Queue() : signal_end(false) {}

    template <class... Whatever>
    void push(Whatever... x...)
    {
        {
            std::lock_guard<std::mutex> l(m);
            lst.push(x...);
        }
        cv.notify_one();
    }

    bool signaled_end() { return signal_end; }
    void set_signal_end()
    {
        signal_end = true;
        cv.notify_all();
    }

    bool empty() const { return lst.empty(); }

    T pop()
    {
        {
            auto self = this;
            auto cond = [&self]() -> bool {
                            return (not self->lst.empty()) or self->signal_end;
                        };

            std::unique_lock<std::mutex> l(m);
            cv.wait(l, cond);
        }

        if (signal_end) return NULL;

        std::lock_guard<std::mutex> l(m);
        auto v = lst.front();
        lst.pop();
        return v;
    }
};

#endif /* INCL_SYNC_QUEUE_H */
