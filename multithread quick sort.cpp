#include <iostream>
#include <vector>
#include <thread>
#include <stack>
#include <mutex>
#include <random>

std::mutex mtx;

template<class T>
void quick_sort(typename T::iterator first, typename T::iterator last, std::stack<std::pair<typename T::iterator, typename T::iterator>>& tasks) { // 5 3 2 4 5 7 8 3 6 1 1 6(o)
	if (last - first <= 1) return;

	std::srand(0);

	size_t medium = std::rand() % (last - first);

	medium = (last - first) % 2;

	std::swap(*(first + medium), *(last - 1));

	medium = last - first - 1;

	size_t right_medium = medium;

	for (size_t i = 0;i < medium;) {
		if (*(first + i) >= *(first + medium)) {
			size_t j = medium;

			auto medium_val = *(first + medium);

			for (; first + j != last && *(first + j) == medium_val; j++) {
				std::swap(*(first + j), *(first + j - 1));
			}

			j--;

			if (i != medium - 1) {
				std::swap(*(first + j), *(first + i));
			}

			medium--;

			if (*(first + j) != medium_val) {
				right_medium--;
			}
		}
		else {
			i++;
		}
	}

	//quick_sort<T>(first, first + medium, v);
	//quick_sort<T>(first + right_medium + 1, last, v);

	
	mtx.lock();
	
	tasks.push({ first, first + medium });
	tasks.push({ first + right_medium + 1, last});
	
	mtx.unlock();

}


template<class T>
void get_work(std::stack<std::pair<typename T::iterator, typename T::iterator>>& tasks, std::vector<std::thread::id>& busy_threads) {

	//mtx.lock();

	//bool exist_busy_thread = busy_threads.size();
	//bool tasks_empty = tasks.empty();


	//mtx.unlock();


	while (true) {
		mtx.lock();

		if (tasks.empty() && !busy_threads.size()) {
			mtx.unlock();
			break;
		}

		if (!tasks.empty()) {
			auto current_task = tasks.top();
			tasks.pop();

			busy_threads.push_back(std::this_thread::get_id());

			mtx.unlock();

			quick_sort<std::vector<int>>(current_task.first, current_task.second, tasks);

			mtx.lock();

			busy_threads.erase(std::find(busy_threads.begin(), busy_threads.end(), std::this_thread::get_id()));

			//exist_busy_thread = busy_threads.size();
			//tasks_empty = tasks.empty();
		}

		mtx.unlock();
	}


}


template<class T>
void my_sort(typename T::iterator first, typename T::iterator last) {

	size_t num_threads = std::thread::hardware_concurrency();
	
	std::vector<std::thread> threads;
	std::vector<std::thread::id> busy_threads;
	
	std::stack<std::pair<typename T::iterator, typename T::iterator>> tasks;
	
	tasks.push({ first, last });
	
	for (size_t i = 0; i < num_threads; i++) {
		threads.push_back(std::thread(get_work<std::vector<int>>, std::ref(tasks), std::ref(busy_threads)));
	}
	
	//std::thread th(get_work<std::vector<int>>, std::ref(tasks), std::ref(busy_threads));
	//std::thread th2(get_work<std::vector<int>>, std::ref(tasks), std::ref(busy_threads));
	//
	//th.join();
	//th2.join();
	
	for (auto& thread : threads) {
		thread.join();
	}
}

int main(){
	std::srand(0);

	for (int i = 0; i < 100; i++) {

		std::vector<int> v(rand() % 10000);

		for (int i = 0; i < v.size(); i++) {
			v[i] = rand();
		}



		auto v2 = v;

		my_sort<std::vector<int>>(v.begin(), v.end());

		std::sort(v2.begin(), v2.end());

		bool b = v2 == v;

		std::cout << (int)b << " size: " << v.size() << std::endl;
	}
}
