#include <iostream>
#include <vector>
#include <thread>
#include <stack>
#include <mutex>
#include <random>
#include <initializer_list>
#include <condition_variable>

std::mutex mtx;

std::condition_variable cond;


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

	
	std::lock_guard<std::mutex> lg(mtx);
	
	tasks.push({ first, first + medium });
	tasks.push({ first + right_medium + 1, last});

	cond.notify_all();
}


template<class T>
void get_work(std::stack<std::pair<typename T::iterator, typename T::iterator>>& tasks, std::vector<std::thread::id>& busy_threads) {


	while (true) {
		std::unique_lock<std::mutex> ul(mtx);

		cond.wait(ul, [&tasks, &busy_threads] {return !tasks.empty() || busy_threads.size() || (!busy_threads.size() && tasks.empty()); });

		if (tasks.empty() && busy_threads.size()) {
			ul.unlock();
			continue;
		}

		else if (tasks.empty() && !busy_threads.size()) {
			ul.unlock();
			break;
		}

		
		auto current_task = tasks.top();
		tasks.pop();

		if (!tasks.empty()) {
			cond.notify_all();
		}


		busy_threads.push_back(std::this_thread::get_id());

		ul.unlock();

		quick_sort<std::vector<int>>(current_task.first, current_task.second, tasks);

		ul.lock();

		busy_threads.erase(std::find(busy_threads.begin(), busy_threads.end(), std::this_thread::get_id()));

		cond.notify_all();
		
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
	
	
	for (auto& thread : threads) {
		thread.join();
	}
}

int main(){
	std::srand(0);

	for (int i = 0; i < 100; i++) {

		std::vector<int> v(rand() % 1000);

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
