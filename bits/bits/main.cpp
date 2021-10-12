#include <cstdio>
#include <stdint.h>
#include <stdlib.h>

const uint32_t nameSize = 32;

#define BaseNameSize ( size_t )(&((struct Person*)0)->name)

struct Person
{
	uint32_t age;
	uint32_t sex;
	char name[ nameSize ];
};

void testFieldBaseSize() { 
	struct Person p = {};
	printf( "name base size:%d\n", BaseNameSize );
}

void testInitNums() { 
	int32_t n[] = { [0] = 5, [1] = 1 }; 

	printf( "nums size is :%d\n", sizeof( n ) );
}

int main() {
	uint64_t tgid = 15;
	uint64_t pid  = 3;

	uint64_t tgpid_pid = tgid << 32 | pid;

	printf( "tgid: %016llx, pid: %016llx, tgpid_pid %016llx\n", tgid, pid, tgpid_pid );

	uint32_t pid1 = tgpid_pid;

	printf( "pid1: %u\n", pid1 );

	testFieldBaseSize();

	testInitNums();

	return 0;
}