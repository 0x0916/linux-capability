#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/capability.h>

void usage(void) {
	fprintf(stderr, "Usage: getpcaps <pid> [<pid> ...]\n");
	exit(1);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		usage();
	}

	for (++argv; --argc > 0; ++argv) {
		int ret;
		struct __user_cap_header_struct	cap_header;
		struct __user_cap_data_struct cap_data[2];
		ret = capget(&cap_header, NULL);
		if (ret < 0) {
			fprintf(stderr, "capget error: %s", strerror(errno));
		}
		memset(cap_data, 0, sizeof(cap_data));
		// get capabilities of the calling thred if pid is 0
		cap_header.pid = atoi(argv[0]);
		ret = capget(&cap_header, &cap_data[0]);
		if (ret < 0) {
			fprintf(stderr, "capget error: %s", strerror(errno));
		} else {
			printf("\nCapabilities of process %d is:\n", cap_header.pid);
			printf("\tCapInh: 0x%08x%08x\n", cap_data[1].inheritable, cap_data[0].inheritable);
			printf("\tCapEff: 0x%08x%08x\n", cap_data[1].effective, cap_data[0].effective);
			printf("\tCapPrm: 0x%08x%08x\n", cap_data[1].permitted, cap_data[0].permitted);
		}
	}

	exit(EXIT_SUCCESS);
}

// gcc 0005-getpcaps.c -o getpcaps
