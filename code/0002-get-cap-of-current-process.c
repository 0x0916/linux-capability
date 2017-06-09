#include <stdio.h>
#include <string.h>
#include <sys/capability.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	int ret;
	struct __user_cap_header_struct	cap_header;
	struct __user_cap_data_struct cap_data[2];

	ret = capget(&cap_header, NULL);
	if (ret < 0) {
		fprintf(stderr, "capget error: %s", strerror(errno));
	}

	cap_header.pid = getpid();
	ret = capget(&cap_header, &cap_data[0]);
	if (ret < 0) {
		fprintf(stderr, "capget error: %s", strerror(errno));
	}

	printf("Capabilities of process %d is:\n", cap_header.pid);
	printf("\tCapInh: 0x%08x%08x\n", cap_data[1].inheritable, cap_data[0].inheritable);
	printf("\tCapEff: 0x%08x%08x\n", cap_data[1].effective, cap_data[0].effective);
	printf("\tCapPrm: 0x%08x%08x\n", cap_data[1].permitted, cap_data[0].permitted);

	exit(EXIT_SUCCESS);
}

// gcc 0002-get-cap-of-current-process.c -o 0002-get-cap-of-current-process
