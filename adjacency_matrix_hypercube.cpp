#include <iostream>
#include <vector>


void visualize(const std::vector<std::vector<bool>>& m) {
	for (const auto& row : m) {
		for (const int e : row) {
			std::cout << (e ? "1" : " ") << " ";
		}
		std::cout << std::endl;
	}
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

int main() {
	auto m = createHypercube(32);

	visualize(m);
}
