#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage %s <usecs>\n", argv[0]);
		return 1;
	}
	usleep(atoi(argv[1]));
	return 0;
}
