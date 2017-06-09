#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>

int caps_last_cap_1(void) {
	int fd;
	int result = -1;

	// try to get the maximum capability over the kernel interface
	fd = open("/proc/sys/kernel/cap_last_cap", O_RDONLY);
	if (fd >= 0) {
		char buf[32];
		int n, ret;

		if ((n=read(fd, buf, 31)) >= 0) {
			buf[n] = '\0';
			ret = sscanf(buf, "%d", &result);
			if (ret != 1)
				result = -1;
		}
		close(fd);
	}

	return result;
}

int caps_last_cap_2(void) {
	// try to get maximum capability by trying get the status of
	// each capability indeviually from the kernel.
	int cap = 0;
	while (prctl(PR_CAPBSET_READ, cap) >= 0)
		cap++;
	return cap - 1;
}

int main(int argc, char **argv) {
	int cap;

	cap = caps_last_cap_1();
	if (cap != -1)
		printf("[caps_last_cap_1] The maximum capability is : %d\n", cap);
	else
		printf("[caps_last_cap_1] can not get maximum capability over kernel interface\n");

	cap = caps_last_cap_2();
	printf("[caps_last_cap_2] The maximum capability is : %d\n", cap);

	exit(0);
}

// gcc 0003-cap-last-cap.c -o 0003-cap-last-cap
