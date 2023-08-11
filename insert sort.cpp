#include <iostream>
#include <vector>
#include <algorithm>


void insert_sort(std::vector<int>& v) { // 1 4 6 8 3(i) -> 1 3 6 4 8
    for (int i = 1; i < v.size(); i++) {
        int insert_place = std::upper_bound(v.begin(), v.begin() + i, v[i]) - v.begin();

        std::swap(v[insert_place], v[i]);

        for (int j = i; v[j] < v[j - 1]; j--) {
            std::swap(v[j], v[j - 1]);
        }
    }
}

int main()
{
    std::vector<int> v = { 8, 3, 4, 1, 5, 4, 6, 6 };

    insert_sort(v);


}
