// copy_construct.cpp: 定义控制台应用程序的入口点。
//

#if defined(__GNUC__)
#include <stdint.h>
using __int32 = int32_t;
#elif defined(_MSC_VER)
#include "stdafx.h"
#endif

#include <iostream>

// https://www.ibm.com/developerworks/community/blogs/5894415f-be62-4bc0-81c5-3956e82276f3/entry/RVO_V_S_std_move?lang=en
// rvo必须是表达式类型和返回值类型完全一致才会有效，we must keep our type of return statement the same as function return type.
// http://czxyl.me/2017/08/04/(%E8%AF%91)ROV%20VS%20stdmove/

class Widget
{
public:
	Widget(__int32 i = 0) : m_num(new __int32(i)) {
		std::cout << "Widget construct" << std::endl;
	}


	Widget(const Widget & w) : m_num(new __int32(*w.m_num)) {
		// 这个拷贝构造是一次都不调用的
		std::cout << "Widget copy construct" << std::endl;
	}

	Widget(Widget && w) {
		std::cout << "Widget move construct" << std::endl;
		m_num = w.m_num;
		w.m_num = nullptr;
	}

	~Widget() {
		std::cout << "Widget destruct" << std::endl;
		delete m_num;
	}

	__int32 * m_num;
};

Widget getWidgetRvo() {
	std::cout << "----getWidget RVO----" << std::endl;
	//RVO
	return Widget(99);
}

Widget getWidgetDefNRvo() {
	std::cout << "----getWidget DefNRVO----" << std::endl;
	Widget w(98);
	return w;
}

Widget getWidgetNRvo(__int32 i) {
	std::cout << "----getWidget NRVO----" << std::endl;
	Widget w(98), w1(100);
	if(i < 1) {
		return w;
	}
	return w1;
}

Widget getWidgetMove() {
	std::cout << "----getWidget MOVE----" << std::endl;
	Widget w(98);
	return std::move(w);
}

// 同样有dingling reference的问题，不要这样用
Widget && getWidgetRRef() {
	std::cout << "----getWidget RREF----" << std::endl;
	Widget w(98);
	return std::move(w);
}

// 同样有dingling reference的问题，不要这样用，如果内部有堆对象，这样使用非常的危险
const Widget && getWidgetCRRef() {
	std::cout << "----getWidget CRREF----" << std::endl;
	Widget w(98);
	std::cout << "w address=" << std::hex << &w << std::endl;
	return std::move(w);
}

int main()
{
	{
		Widget w1 = getWidgetRvo();
		std::cout << "w1.num=" << *w1.m_num << std::endl;
		// 		----getWidget RVO----
		// 			Widget construct
		// 			w1.num = 99
		// 			Widget destruct
	}

	{
		Widget w2 = getWidgetDefNRvo();
		std::cout << "w2.num=" << *w2.m_num << std::endl;
// 		----getWidget DefNRVO----
// 			Widget construct
// 			w2.num = 98
// 			Widget destru	
	}

	{
		Widget w2 = getWidgetNRvo(0);
		std::cout << "w2.num=" << *w2.m_num << std::endl;
// 		----getWidget NRVO----
// 			Widget construct
// 			Widget construct
// 			Widget move construct
// 			Widget destruct
// 			Widget destruct
// 			w2.num = 98
// 			Widget destru	
	}

	{
		Widget w3 = getWidgetMove();
		std::cout << "w3.num=" << *w3.m_num << std::endl;
		// 		----getWidget MOVE----
		// 			Widget construct
		// 			Widget move construct
		// 			Widget destruct
		// 			w3.num = 98
		// 			Widget destruct
	}

	{
		Widget && w4 = getWidgetRRef();
		std::cout << "w4.num=" << *w4.m_num << std::endl;
// 		----getWidget RREF----
// 			Widget construct
// 			Widget destruct
// 			w4.num = 76882	
	}


	{
		const Widget && w5 = getWidgetCRRef();
		std::cout << "w5.num=" << *w5.m_num << std::endl;
		std::cout << "w5 address=" << std::hex << &w5 << std::endl;
// 		----getWidget CRREF----
// 			Widget construct
// 			w address = 0x7ffc83b77f60
// 			Widget destruct
// 			w5.num = 755020
// 			w5 address = 0x7ffc83b77f	
	}
	return 0;
}

