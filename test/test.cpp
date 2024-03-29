// test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>

typedef struct InitS
{
    InitS()
    : a( 10 ), b( "calm" ) {}

    int a = 1;
    std::string b{"Hello"};
} InitT;


template < typename... Elements >
class Metas;

template < typename Head, typename... Tail >
class Metas< Head, Tail... > : private Metas< Tail... > {
    Head head;
};

template <>
class Metas<> {
};

template < typename... T >
void f( T... args ) {
    size_t tcount = sizeof...( T );
    std::cout << "tcount:" << tcount << std::endl;
    return;
}

typedef struct CDefault
{
    CDefault() = default; // 这里保证使用由编译器生成的“平凡构造函数”，声明下，不需要定义，而且保证了CDefault是POD类型
    CDefault& operator=( const CDefault& ) = default;

    __int32 i{10};
};

template < typename T >
using MapString = std::map< T, char* >;
MapString< int > numberString;

class Foo {
public:
    Foo() = default;
    Foo( const Foo& f ) {
        std::cout << "foo copy construct" << std::endl;
    }

    Foo( Foo&& f ) {
        std::cout << "foo move construct" << std::endl;
    }

    Foo& operator=( const Foo& ) & {
        return *this;
    }
};

template < typename... Args >
void fun( Args&&... args ) {
    // 这里我们希望将fun的所有实参转发给work函数，由于fun使用的是右值引用，因此我们可以传递给它任意类型的实参，使用
    // forward能保持原有的类型
    //work(std::forward<Args>(args)...);
}

void testMap() {
    std::map< std::string, size_t > nameCounters{
        {"Marc", 5},
        {"Bob", 12},
        {"John", 3}};

    auto result = nameCounters.find( "Bob" );
    if ( result != cend( nameCounters ) )
        std::cout << "Count: " << result->second << std::endl;

    // --std=c++17
    //if(auto result = nameCounters.find("Bob"); result != cend(nameCounters))
    //	std::cout << "Count: " << result->second << std::endl;
}

template < typename T >
auto print_type_info( const T& t ) {
    if constexpr ( std::_Is_integral< T >::value ) {
        return t + 1;
    }
    else {
        return t + 0.01;
    }
}

// 区间迭代
void rangeIterator() {
    std::vector< int > vec = {1, 2, 3, 4, 5};
    for ( auto element : vec ) {
        std::cout << element << std::endl;
    }

    for ( auto& element : vec ) {
        element += 1;
    }

    for ( auto element : vec ) {
        std::cout << element << std::endl;
    }
}

extern void test_uniqueptr();
extern void printtype();
extern void float_loop();
extern void range_loop();
extern void test_smartptr();

int main() {
    InitT i;
    std::cout << "i.a" << i.a << std::endl;

    f( 0, "1", 0.9f );

    CDefault cd;

    Foo f1;
    Foo f2( std::move( f1 ) );
    Foo f3( f2 );

    testMap();

    std::cout << print_type_info( 5 ) << std::endl;
    std::cout << print_type_info( 3.14 ) << std::endl;

    rangeIterator();

    test_uniqueptr();

	printtype();

	float_loop();

	range_loop();

	test_smartptr();

    return 0;
}
