#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>


template <class T>
class threadsafe_queue {
public:
    threadsafe_queue() = default;

    threadsafe_queue(const threadsafe_queue& tq) {
        std::lock_guard<std::mutex> lg(mtx);

        queue = tq.queue;
    }

    threadsafe_queue& operator=(const threadsafe_queue& tq) {
        std::lock_guard<std::mutex> lg(mtx);

        queue = tq.queue;

        return *this;
    }

    threadsafe_queue(threadsafe_queue&& tq) {
        std::lock_guard<std::mutex> lg(mtx);

        queue = std::move(tq.queue);
    }

    threadsafe_queue& operator=(threadsafe_queue&& tq) {
        std::lock_guard<std::mutex> lg(mtx);

        queue = std::move(tq.queue);

        return *this;
    }

    size_t size() {
        std::lock_guard<std::mutex> lg(mtx);

        return queue.size();
    }

    bool empty() {
        std::lock_guard<std::mutex> lg(mtx);

        return queue.empty();
    }

    void push(const T& val) {
        std::lock_guard<std::mutex> lg(mtx);

        queue.push(val);

        c_var.notify_one();
    }

    void wait_and_pop() {
        std::unique_lock<std::mutex> ul(mtx);

        c_var.wait(ul, [&] {return !queue.empty(); });

        queue.pop();
    }

    bool try_pop() {
        std::lock_guard<std::mutex> lg(mtx);

        if (queue.empty()) {
            return false;
        }

        queue.pop();
    }

    std::pair<bool, T> front() {
        std::lock_guard<std::mutex> lg(mtx);

        if (queue.empty()) {
            return std::make_pair(false, T());
        }

        return std::make_pair(true, queue.front());
    }
    
private:
    mutable std::mutex mtx;

    std::queue<T> queue;

    std::condition_variable c_var;
};

void f(threadsafe_queue<int>& tq) {
    std::this_thread::sleep_for(std::chrono::seconds(2));

    tq.push(12);
}


int main()
{

    threadsafe_queue<int> tq;

    tq.push(1);
    tq.push(2);
    tq.push(3);
    tq.push(4);
    tq.push(5);

    tq.try_pop();
    tq.try_pop();
    tq.try_pop();
    tq.try_pop();
    tq.try_pop();

    std::thread t(f, std::ref(tq));

    tq.wait_and_pop();

    std::cout << (int)tq.front().first << " " << tq.front().second;

    t.join();
}

