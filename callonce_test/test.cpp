#include <iostream>
#include <stdint.h>
#include <absl/base/call_once.h>

class MyInitClass {
public:
	MyInitClass() {}

	void init() const {
		absl::call_once(m_once, &MyInitClass::SayOnceHello, this);
	}

private:
	void SayOnceHello() const {
		std::cout << "Say Hello Once" << std::endl;
	}

	mutable absl::once_flag m_once;
};

int32_t main(int32_t argc, char ** argv) {
	MyInitClass myInitC;
	myInitC.init();
	myInitC.init();
	myInitC.init();

	return 0;
}

// [calm@localhost base]$ bazel build spinlock_wait
// [calm@localhost base]$ bazel build base