#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <exception>
#include <future>
#include <atomic>
#include <queue>
#include <functional>

template <class T>
class matrix_parallel_mult {
public:
    matrix_parallel_mult() = delete;

    matrix_parallel_mult(size_t m, size_t n) {
        if (!m || !n) {
            throw std::exception("bad size");
        }
        for (int i = 0; i < m; i++) {
            data.push_back(std::move(std::vector<T>(n)));
        }
    }

    matrix_parallel_mult(matrix_parallel_mult&& m) {
        data = std::move(m.data);
    }

    matrix_parallel_mult& operator=(matrix_parallel_mult&& m) {
        std::lock_guard<std::mutex> lg(mtx_for_operations);

        data = std::move(m.data);
        return *this;
    }

    T& at(size_t i, size_t j) {
        std::lock_guard<std::mutex> lg(mtx_for_operations);

        return data[i][j];
    }

    std::pair<size_t, size_t> shape() {
        return std::make_pair(data.size(), data[0].size());
    }

    matrix_parallel_mult operator*(const matrix_parallel_mult& m) const{
        std::lock_guard<std::mutex> lg(mtx_for_operations);

        std::queue<std::function<std::pair<size_t, std::vector<T>>()>> tasks;
        std::mutex mtx_for_worker;
        std::mutex mtx_for_adding;

        size_t num_threads = std::thread::hardware_concurrency();
        std::vector<std::vector<T>> result_data;

        for (size_t i = 0; i < data.size(); i++) {
            result_data.push_back(std::vector<T>(m.data[0].size()));
        }

        for (size_t k = 0; k < m.data.size(); k++) {
            tasks.push([&] {return std::make_pair(k, mult(*this, m, k)); });
        }

        std::vector<std::thread> threads;

        for (size_t i = 0; i < num_threads; i++) {
            threads.push_back(std::thread(&matrix_parallel_mult::worker, *this, std::ref(tasks), std::ref(result_data), std::ref(mtx_for_worker), std::ref(mtx_for_adding)));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        matrix_parallel_mult result(1, 1);

        result.data = std::move(result_data);

        return result;
    }


private:
    std::vector<std::vector<T>> data;
    mutable std::mutex mtx_for_operations;


    void worker(std::queue<std::function<std::pair<size_t, std::vector<T>>()>>& tasks, std::vector<std::vector<T>>& result_data, std::mutex& mutex_for_worker, std::mutex& mtx_for_adding) {
        
        while (true) {
            std::unique_lock<std::mutex> ul(mutex_for_worker);

            if (tasks.empty()){
                break;
            }
               
            auto current_task = tasks.front();

            ul.unlock();

            auto result_k_row = current_task();

            mtx_for_adding.lock();

            result_data[result_k_row.first] = std::move(result_k_row.second);

            mtx_for_adding.unlock();
        }

    }

    std::vector<T> mult(const matrix_parallel_mult& m1, const matrix_parallel_mult& m2, size_t k) const{
        std::vector<T> result_row(m2.data[0].size());

        for (size_t i = 0; i < m1.data.size(); i++) {
            for (size_t j = 0; m2.data[0].size(); j++) {
                result_row[j] += m1.data[i][k] + m2.data[k][j];
            }
        }

        return result_row;
    }
};

template <class T>
matrix_parallel_mult<T> random_matrix(size_t m, size_t n) {
    matrix_parallel_mult<T> result(m, n);

    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; i < n; j++) {
            result.at(i, j) = std::rand();
        }
    }
}


int main(){
    std::srand(0);

    matrix_parallel_mult<int> m1 = random_matrix<int>(100, 100);
    matrix_parallel_mult<int> m2 = random_matrix<int>(100, 100);
    matrix_parallel_mult<int> m3 = m1 * m2;
}

