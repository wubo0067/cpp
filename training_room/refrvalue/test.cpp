#include <iostream>
#include <type_traits>
#include <typeinfo>

class Widget {
public:
	Widget() : m_a(10) {}

	~Widget() noexcept {
		std::cout << "~Widget" << std::endl;
	}

	void PrintWidget() const {
		std::cout << "Widget m_a[" << m_a << "]" << std::endl;
	}
private:
	int m_a;
};

Widget MakeWidget() {
	return Widget();
}

template<typename T>
struct TypeParseTraits;

#define REGISTER_PARSE_TYPE(X) template <> struct TypeParseTraits<X> \
	{ static const char* name; } ; const char* TypeParseTraits<X>::name = #X
	

REGISTER_PARSE_TYPE(int);
REGISTER_PARSE_TYPE(int&);
REGISTER_PARSE_TYPE(const int&);
REGISTER_PARSE_TYPE(double);
REGISTER_PARSE_TYPE(Widget);
REGISTER_PARSE_TYPE(Widget&);

template<typename T>
void PrintfRefType(T && t) {
	std::cout << "-----------------: " << TypeParseTraits<T>::name << std::endl;
	std::cout << "is_reference: " << std::is_reference<T>::value << std::endl;
	std::cout << "is_rvalue_reference: " << std::is_rvalue_reference<T>::value << std::endl;
	std::cout << "is_lvalue_reference: " << std::is_lvalue_reference<T>::value << std::endl;
}

int main() {
	Widget && w = MakeWidget();

	std::cout << "------" << std::endl;
	
	w.PrintWidget();

	PrintfRefType(w);

	int a = 10;
	PrintfRefType(a);
	const int & b = a; 
	PrintfRefType(b);

	int && c = 10;
	// 这里还有个技术点，就是reference collapsing
	// c是可以取地址的，虽然c的本质是个右值引用，但到了universal reference中它还是绑定的左值引用，所以要通过转发来获取其本质
	std::cout << std::hex << &c << std::endl;
	// 这里让我很奇怪，c进去后，T是左值引用
	PrintfRefType(10);
	PrintfRefType(c);
}
