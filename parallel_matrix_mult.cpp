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

    matrix_parallel_mult(matrix_parallel_mult&& m) noexcept{
        data = std::move(m.data);
    }

    matrix_parallel_mult& operator=(matrix_parallel_mult&& m) noexcept{
        std::lock_guard<std::mutex> lg(mtx_for_operations);

        data = std::move(m.data);
        return *this;
    }

    const T& at(size_t i, size_t j) const{
        std::lock_guard<std::mutex> lg(mtx_for_operations);

        return data[i][j];
    }

    T& at(size_t i, size_t j) {
        std::lock_guard<std::mutex> lg(mtx_for_operations);

        return data[i][j];
    }

    std::pair<size_t, size_t> shape() const{
        return std::make_pair(data.size(), data[0].size());
    }

    matrix_parallel_mult operator*(const matrix_parallel_mult& m) const{
        std::lock_guard<std::mutex> lg(mtx_for_operations);

        std::queue<std::function<std::pair<size_t, std::vector<T>>()>> tasks;
        std::mutex mtx_for_worker;
        std::mutex mtx_for_adding;

        size_t num_threads = std::thread::hardware_concurrency();
        std::vector<std::vector<T>> result_data(data.size());

        for (size_t i = 0; i < data.size(); i++) {
            tasks.push([&, i] {return std::make_pair(i, mult(*this, m, i)); });
        }

        std::vector<std::thread> threads;
        
        for (size_t i = 0; i < num_threads; i++) {
            threads.push_back(std::thread([&] {worker(tasks, result_data, mtx_for_worker, mtx_for_adding); }));
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


    void worker(std::queue<std::function<std::pair<size_t, std::vector<T>>()>>& tasks, std::vector<std::vector<T>>& result_data, std::mutex& mutex_for_worker, std::mutex& mtx_for_adding) const{
        
        while (true) {
            std::unique_lock<std::mutex> ul(mutex_for_worker);

            if (tasks.empty()){
                break;
            }
               
            auto current_task = tasks.front();
            tasks.pop();

            ul.unlock();

            auto result_i_row = current_task();

            mtx_for_adding.lock();

            result_data[result_i_row.first] = std::move(result_i_row.second);

            mtx_for_adding.unlock();
        }

    }

    std::vector<T> mult(const matrix_parallel_mult& m1, const matrix_parallel_mult& m2, size_t i) const{
        std::vector<T> result_row(m2.data[0].size());

        for (size_t k = 0; k < m2.data.size(); k++) {
            for (size_t j = 0; j < m2.data[0].size(); j++) {
                result_row[j] += m1.data[i][k] * m2.data[k][j];
            }
        }

        return result_row;
    }
};

template <class T>
matrix_parallel_mult<T> random_matrix(size_t m, size_t n) {
    matrix_parallel_mult<T> result(m, n);

    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            result.at(i, j) = std::rand();
        }
    }

    return result;
}

template <class T>
void print_matrix(const matrix_parallel_mult<T>& m) {

    std::cout << std::endl;
    for (size_t i = 0; i < m.shape().first; i++) {
        for (size_t j = 0; j < m.shape().second; j++) {
            std::cout << m.at(i, j) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

}

int main(){
    std::srand(0);

    int m = 20;
    int n = 20;

    matrix_parallel_mult<long long> m1 = random_matrix<long long>(m, n);
    matrix_parallel_mult<long long> m2 = random_matrix<long long>(m, n);

    //m1.at(0,0) = m2.at(0,0) = 1;
    //m1.at(0,1) = m2.at(0,1) = 2;
    //m1.at(1,0) = m2.at(1,0) = 3;
    //m1.at(1,1) = m2.at(1,1) = 4;

    matrix_parallel_mult<long long> m3 = m1 * m2;

    print_matrix(m1);
    print_matrix(m2);
    print_matrix(m3);
}

