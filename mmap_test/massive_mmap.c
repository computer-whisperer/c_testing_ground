#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BYTES_MAPPED 100000000000
#define NUM_WRITES 10000000

int main(int argc, char * argv[])
{
	char * memory_space = mmap(NULL, BYTES_MAPPED, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (memory_space > 0)
	{
		printf("Massive memory map successful, blindly forking twice.\n");
		fork();
		fork();
		printf("Writing randomly to shared memory.\n");
		for (long i = 0; i < NUM_WRITES; i++)
		{
			memory_space[(BYTES_MAPPED/NUM_WRITES)*i] = 'H';
		}
		printf("Sleeping for 60 secs.\n");
		sleep(60);
	}
	else
	{
		printf("Memory map failed!\n");
	}
}
