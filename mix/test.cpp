#include <iostream>
#include <stdint.h>

constexpr int32_t DefSize() {
	return 10;
}

const int32_t DefConstSize() {
	return 8;
}

// 这里使用-std=c++1y，不会告警
template<typename X, typename Y>
auto Add(X x, Y y) ->decltype(x+y) {
	return x + y;
}

int32_t main(int32_t argc, char *argv[]) {
	int32_t numAry[DefSize()];

	std::cout << "numAry size:" << sizeof(numAry) << std::endl;

	int32_t numAry1[DefConstSize()];

	std::cout << "numAry1 size:" << sizeof(numAry1) << std::endl;

	auto z = Add(1, 20.1);

	std::cout << "z:" << z << std::endl;

	return 0;
}