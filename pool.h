#include <iostream>
#include <thread>
#include <future>
#include <mutex>
#include <queue>
#include <vector>
#include <unordered_set>
#include <exception>


class ThreadPool{
    std::queue<std::pair<int, std::future<void>>> tasks;
    std::mutex mutex_for_tasks;
    std::condition_variable c_var_for_tasks;

    std::unordered_set<int> completed_tasks;
    std::mutex mutex_for_completed_tasks;
    std::condition_variable c_var_for_completed_tasks;

    std::vector<std::thread> threads;

    std::atomic<int> task_index = 0;

    std::atomic<bool> end = false;

    std::atomic<int> last_index_completed_task = -1;

    void run(){
        while (true){
            std::unique_lock<std::mutex> ul(mutex_for_tasks);

            c_var_for_tasks.wait(ul, [this]{return !tasks.empty() || end.load();});

            if (end.load()){
                break;
            }

            auto current_task = std::move(tasks.front());
            tasks.pop();
            current_task.second.get();

            {
                std::lock_guard<std::mutex> lg(mutex_for_completed_tasks);

                completed_tasks.insert(current_task.first);

                last_index_completed_task.store(current_task.first);

                c_var_for_completed_tasks.notify_all();
            }
        }
    }

public:
    ThreadPool(int num_threads){
        for (int i = 0; i < num_threads; i++){
            threads.push_back(std::thread(&ThreadPool::run, this));
        }
    }

    ~ThreadPool(){
        end.store(true);
        c_var_for_tasks.notify_all();
        
        for (auto& thread : threads){
            thread.join();
        }

        end.store(false);
    }

    template<class function, class ... Args>
    int add_task(function func, Args ... args){
        std::lock_guard lg(mutex_for_tasks);

        tasks.push(std::make_pair(task_index.load(), std::async(std::launch::deferred, func, args...)));

        task_index.fetch_add(1);

        c_var_for_tasks.notify_one();

        return task_index.load() - 1;
    }

    bool is_task_done(int index){
        if (index < 0 || index >= task_index){
            throw std::invalid_argument("incorrect index");
        }

        std::lock_guard<std::mutex> lg(mutex_for_completed_tasks);

        return completed_tasks.find(index) != completed_tasks.end();
    }

    void wait_task(int index){
        if (index < 0 || index >= task_index){
            throw std::invalid_argument("incorrect index");
        }

        std::unique_lock<std::mutex> ul(mutex_for_completed_tasks);

        c_var_for_completed_tasks.wait(ul, [this, index]{
            return completed_tasks.find(index) != completed_tasks.end();
            });
    }

    void wait_all_tasks(){
        std::unique_lock<std::mutex> ul(mutex_for_completed_tasks);

        c_var_for_completed_tasks.wait(ul, [this]{
            return !tasks.size() && last_index_completed_task.load() == task_index.load() - 1;
            });
    }
};