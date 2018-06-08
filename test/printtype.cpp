#include "stdafx.h"
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <iostream>

inline std::ostream& operator<<( std::ostream& os, std::string_view const& s ) {
    return os.write( s.data(), s.size() );
}

template < class T > constexpr std::string_view type_name() {
    using namespace std;
#ifdef __clang__
    string_view p = __PRETTY_FUNCTION__;
    return string_view( p.data() + 34, p.size() - 34 - 1 );
#elif defined( __GNUC__ )
    string_view p = __PRETTY_FUNCTION__;
#if __cplusplus < 201402
    return string_view( p.data() + 36, p.size() - 36 - 1 );
#else
    return string_view( p.data() + 49, p.find( ';', 49 ) - 49 );
#endif
#elif defined( _MSC_VER )
    string_view p = __FUNCSIG__;
    return string_view( p.data() + 84, p.size() - 84 - 7 );
#endif
}

int&  foo_lref();
int&& foo_rref();
int   foo_value();

void printtype() {
    int       i  = 0;
    const int ci = 0;
    std::cout << "decltype(i) is " << type_name< decltype( i ) >() << '\n';
    std::cout << "decltype((i)) is " << type_name< decltype( ( i ) ) >() << '\n';
    std::cout << "decltype(ci) is " << type_name< decltype( ci ) >() << '\n';
    std::cout << "decltype((ci)) is " << type_name< decltype( ( ci ) ) >() << '\n';
    std::cout << "decltype(static_cast<int&>(i)) is "
              << type_name< decltype( static_cast< int& >( i ) ) >() << '\n';
    std::cout << "decltype(static_cast<int&&>(i)) is "
              << type_name< decltype( static_cast< int&& >( i ) ) >() << '\n';
    std::cout << "decltype(static_cast<int>(i)) is "
              << type_name< decltype( static_cast< int >( i ) ) >() << '\n';
    std::cout << "decltype(foo_lref()) is " << type_name< decltype( foo_lref() ) >()
              << '\n';
    std::cout << "decltype(foo_rref()) is " << type_name< decltype( foo_rref() ) >()
              << '\n';
    std::cout << "decltype(foo_value()) is "
              << type_name< decltype( foo_value() ) >() << '\n';
    return;
}
