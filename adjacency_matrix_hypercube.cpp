#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>


void visualize(const std::vector<std::vector<bool>>& m) {
	for (const auto& row : m) {
		for (const int e : row) {
			std::cout << (e ? "1" : " ") << " ";
		}
		std::cout << std::endl;
	}
}

void visualizeQ(std::queue<int> q) {
	while (!q.empty()) {
		std::cout << q.front() << " ";
		q.pop();
	}
	std::cout << std::endl;
}

bool is2Degree(int n) {
	int copy_n = n;

	while (copy_n % 2 == 0 && copy_n) {
		copy_n /= 2;
	}

	return (bool)copy_n;
}

void writeIdentity(std::vector<std::vector<bool>>& m, int start_i, int end_i, int start_j) {
	for (int i = start_i, j = start_j; i < end_i; i++, j++) {
		m[i][j] = true;
	}
}

void fillMatrix(std::vector<std::vector<bool>>& m, int start, int end) {
	writeIdentity(m, start, start + (end - start) / 2, start + (end - start) / 2);
	writeIdentity(m, start + (end - start) / 2, end, start);

	if (end - start != 2) {
		fillMatrix(m, start, start + (end - start) / 2);
		fillMatrix(m, start + (end - start) / 2, end);
	}
}

std::vector<std::vector<bool>> createHypercube(int n) {
	if (n == 0) {
		throw std::invalid_argument("hypercube size cannot be 0");
	}
	else if (!is2Degree(n)) {
		throw std::invalid_argument("hypercube size must be 2 power");
	}

	std::vector<std::vector<bool>> m(n);

	for (auto& vec : m) {
		vec = std::vector<bool>(n);
	}

	fillMatrix(m, 0, n);

	return m;
}

std::queue<int> getPath(std::vector<std::vector<bool>> m, int start, int end) {
	std::unordered_set<int> labelled;

	std::queue<std::pair<int, std::queue<int>>> verticiesAndPath;

	verticiesAndPath.push(std::make_pair(start, std::queue<int>()));
	labelled.insert(start);

	while (!verticiesAndPath.empty()) {
		int size = verticiesAndPath.size();

		for (int i = 0; i < size; i++) {
			
			for (int j = 0; j < m[0].size(); j++) {
				if (m[verticiesAndPath.front().first][j] && labelled.find(j) == labelled.end()) {
					labelled.insert(j);
					std::queue<int> newPath(verticiesAndPath.front().second);
					newPath.push(verticiesAndPath.front().first);

					if (j == end) {
						return newPath;
					}

					verticiesAndPath.push(std::make_pair(j, newPath));
				}
			}

			verticiesAndPath.pop();
		}
	}
	
	return std::queue<int>();
}

int main() {
	int n;
	std::cin >> n;

	auto m = createHypercube(n);

	visualize(m);
	

	while (true) {
		int start, end;

		std::cin >> start;
		std::cin >> end;

		auto q = getPath(m, start, end);

		std::cout << std::endl << "Q: \n";

		visualizeQ(q);
	}
}
