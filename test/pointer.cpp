#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include "stdafx.h"

void test_uniqueptr()
{
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
