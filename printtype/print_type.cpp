#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <iostream>


template <class T>
constexpr
std::string_view
type_name()
{
    using namespace std;
#ifdef __clang__
    string_view p = __PRETTY_FUNCTION__;
    return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
    string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
    return string_view(p.data() + 36, p.size() - 36 - 1);
#  else
    return string_view(p.data() + 49, p.find(';', 49) - 49);
#  endif
#elif defined(_MSC_VER)
    string_view p = __FUNCSIG__;
    return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

int& foo_lref();
int&& foo_rref();
int foo_value();

typedef struct PersonS{
	int age;
	std::string name;
}PersonT;

int
main()
{
    int i = 0;
    const int ci = 0;
    std::cout << "decltype(i) is " << type_name<decltype(i)>() << '\n';
    std::cout << "decltype((i)) is " << type_name<decltype((i))>() << '\n';
    std::cout << "decltype(ci) is " << type_name<decltype(ci)>() << '\n';
    std::cout << "decltype((ci)) is " << type_name<decltype((ci))>() << '\n';
    std::cout << "decltype(static_cast<int&>(i)) is " << type_name<decltype(static_cast<int&>(i))>() << '\n';
    std::cout << "decltype(static_cast<int&&>(i)) is " << type_name<decltype(static_cast<int&&>(i))>() << '\n';
    std::cout << "decltype(static_cast<int>(i)) is " << type_name<decltype(static_cast<int>(i))>() << '\n';
    std::cout << "decltype(foo_lref()) is " << type_name<decltype(foo_lref())>() << '\n';
    std::cout << "decltype(foo_rref()) is " << type_name<decltype(foo_rref())>() << '\n';
    std::cout << "decltype(foo_value()) is " << type_name<decltype(foo_value())>() << '\n';
	
	PersonT && p = PersonT();
	std::cout << "decltype(p) is " << type_name<decltype(p)>() << '\n';

	const PersonT & p1 = PersonT();
	std::cout << "decltype(p1) is " << type_name<decltype(p1)>() << '\n';
	
	#if __cplusplus==201402L
	std::cout << "C++14" << std::endl;
	#elif __cplusplus==201103L
	std::cout << "C++11" << std::endl;
	#elif __cplusplus==201703L
	std::cout << "C++17" << std::endl;
	#else
	std::cout << "C++" << std::endl;
	#endif	
}