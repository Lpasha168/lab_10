#include <iostream>
#include "MyVector.h"

int main() {
    MyVector<int> v;
    for (int i = 0; i < 25; ++i) {
        v.push_back(i * 10);
    }
    std::cout << "MyVector<int> size=" << v.size() << " capacity=" << v.capacity() << "\n";
    for (std::size_t i = 0; i < v.size(); ++i) {
        std::cout << v[i] << (i + 1 < v.size() ? ' ' : '\n');
    }
    v.pop_back();
    std::cout << "after pop_back size=" << v.size() << "\n";

    MyVector<bool> b;
    b.push_back(true);
    b.push_back(false);
    b.push_back(true);
    std::cout << "MyVector<bool> bits: " << b[0] << b[1] << b[2] << " (packed in uint8_t)\n";
    b[1] = true;
    std::cout << "after b[1]=true: " << b[0] << b[1] << b[2] << "\n";

    return 0;
}
