#include <cstdio>
#include <iostream>
#include<bitset>


int main()
{
	std::cout << std::hex << ~1 << std::endl;
	std::cout << std::bitset<sizeof(int) * 8>(~1) << std::endl;
    printf("hello from embeded_list!\n");
    return 0;
}