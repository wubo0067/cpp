#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include <iomanip>

void test_uniqueptr() {
    std::cout << "test_uniqueptr" << std::endl;

    std::vector< std::unique_ptr< int > > numVector;

    std::unique_ptr< int > up1 = std::make_unique< int >( 1 );
    numVector.push_back( std::move( up1 ) );

    numVector.push_back( std::make_unique< int >( 2 ) );

    auto printNum = []( const std::unique_ptr< int >& up ) {
        std::cout << *up << std::endl;
    };

    std::for_each( numVector.begin(), numVector.end(), printNum );
    return;
}

void float_loop() {
    const double pi{3.1415926};
    for ( double radius{0.2}; radius <= 3.0; radius += .2 ) {
        std::cout << std::fixed << std::setprecision( 2 )
                  << " radius =" << std::setw( 5 ) << radius
                  << " area =" << std::setw( 6 ) << pi * radius * radius
                  << " delta to 3 = " << std::scientific << ( ( radius + 0.2 ) - 3.0 ) << std::endl;
    }
}

void range_loop() {
    int values[]{1, 2, 3, 4, 5, 6, 7};
    int total{};
    for ( auto x : values ) {
        total += x;
    }
    std::cout << "total=" << total << std::endl;
}

void test_smartptr() {
    std::shared_ptr< int > sp1{new int{10}};
    std::weak_ptr          wp1{sp1};

    std::cout << "wp1.use_count:" << wp1.use_count() << std::endl;

    auto sp2 = wp1.lock();
    *sp2     = 999;
    std::cout << "wp1.use_count:" << wp1.use_count() << std::endl;
    std::cout << "*sp1:" << *sp1 << std::endl;

    return;
}
