#include <cstdio>
#include "dbg-macro/dbg.h"

int main()
{
	std::string message = "Hello DBG";
	dbg(message);
    return 0;
}