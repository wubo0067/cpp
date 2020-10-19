#include <iostream>
#include <memory>
#include <vector>

struct A
{
	A( int v )
	    : a( v ) {}

	A( const A& a1 )
	    : a( a1.a ) {
		std::cout << "A const copy construct" << std::endl;
	}

	A( A&& a1 )
	    : a( a1.a ) {
		std::cout << "A move construct" << std::endl;
	}

	int a;
};

void testPush() {
	std::vector< A > vec;
	// 这里A&&绑定临时变量，这个符合编译器
	vec.push_back( A{ 10 } );
}

// 这就是移动赋值的好处，而且没有内存泄露
auto getContent() {
	auto content{ std::make_unique< char[] >( 8 ) };
	std::sprintf( content.get(), "%s", "Hello!" );
	return content;
}

int main( int argc, char* argv[] ) {
	auto content = getContent();
	std::cout << "Hello unique_ptr " << content.get() << std::endl;
	testPush();
	return 0;
}