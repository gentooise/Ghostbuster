#include <linux/ioctl.h>

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define DEVICE_FILE_NAME "/dev/kbus0"

int main(int argc, char** argv) {
	int fd, ret_val;
	int val;
	char* buf[2] = { "\x00\x00\x00\x00", "\x01\x00\x00\x00" };

	// 0 disables interrupt on kbus
	// 1 enables interrupt on kbus
	if (argc != 2 || (argv[1][0] != '0' && argv[1][0] != '1') || argv[1][1] != '\0') {
		printf("Usage: %s [0|1]\n", argv[0]);
		return -1;
	}

	val = (argv[1][0] == '0' ? 0 : 1);

	fd = open(DEVICE_FILE_NAME, O_RDWR);
	if (fd < 0) {
		printf ("Can't open device file: %s\n", DEVICE_FILE_NAME);
		return -2;
	}

	// Call goes to kbus_write kernel function
	if ((ret_val = write(fd, buf[val], 4)) != 0) {
		printf("kbus write failed: %d\n", ret_val);
		close(fd);
		return -3;
	}

	close(fd); 

	return 0;
}
