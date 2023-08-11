#include <iostream>
#include <vector>
#include <thread>
#include <stack>
#include <mutex>
#include <unordered_set>
#include <random>

std::mutex mtx;

template<class T>
void quick_sort(typename T::iterator first, typename T::iterator last, std::stack<std::pair<typename typename T::iterator, typename T::iterator>>& tasks) { // 5 3 2 4 5 7 8 3 6 1 1 6(o)
	if (last - first <= 1) return;

	std::srand(0);

	size_t medium = std::rand() % (last - first);

	std::swap(*(first + medium), last - 1);

	medium = last - first - 1;

	size_t right_medium = medium;

	for (size_t i = 0;i < medium;) {
		if (*(first + i) >= *(first + medium)) {
			size_t j = medium;

			for (; *(first + j) == *(first + medium); j++) {
				std::swap(*(first + j), *(first + j - 1));
			}

			if (i != medium - 1) {
				std::swap(*(first + j), *(first + i));
			}

			medium--;

			if (*(first + j) == *(first + medium)) {
				right_medium = j;
			}
		}
	}

	
	mtx.lock();

	tasks.push({ first, first + medium });
	tasks.push({ first + right_medium + 1, last});

	mtx.unlock();

}


template<class T>
void get_work(std::stack<std::pair<typename typename T::iterator, typename T::iterator>>& tasks, std::unordered_set<size_t>& busy_threads) {

	mtx.lock();

	bool exist_busy_thread = busy_threads.size();
	bool tasks_empty = tasks.empty();

	mtx.unlock();


	while (!tasks_empty || exist_busy_thread) {
		mtx.lock();

		if (!tasks.empty()) {
			auto current_task = tasks.top();
			tasks.pop();

			busy_threads.insert(std::this_thread::get_id());

			mtx.unlock();

			quick_sort(current_task.first, current_task.second);

			mtx.lock();

			busy_threads.erase(std::this_thread::get_id());

			exist_busy_thread = busy_threads.size();
			tasks_empty = tasks.empty();
		}

		mtx.unlock();
	}


}


template<class T>
void sort(typename T::iterator first, typename T::iterator last) {
	std::vector<std::thread> threads(std::thread::hardware_concurrency());
	std::unordered_set<size_t> busy_threads;

	std::stack<std::pair<typename T::iterator, typename T::iterator>> tasks;

	tasks.push({ first, last });

	for (std::thread& t : threads) {
		t = std::move(std::thread(get_work, tasks, busy_threads));
	}
}

int main(){
	std::srand(0);

	std::vector<int> v(rand() % 1000);

	for (int i = 0; i < v.size(); i++) {
		v[i] = rand();
	}

	auto v2 = v;

	sort(v.begin(), v.end());

	std::sort(v2.begin(), v2.end());

	bool b = v2 == v;
}
