#include <iostream>

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

int main() {
	Widget && w = MakeWidget();

	std::cout << "------" << std::endl;
	
	w.PrintWidget();
}