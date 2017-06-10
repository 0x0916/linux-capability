#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/capability.h>

void list_caps() {
	int ret;
	struct __user_cap_header_struct	cap_header;
	struct __user_cap_data_struct cap_data[2];
	ret = capget(&cap_header, NULL);
	if (ret < 0) {
		fprintf(stderr, "capget error: %s", strerror(errno));
	}
	memset(cap_data, 0, sizeof(cap_data));
	cap_header.pid = getpid();
	ret = capget(&cap_header, &cap_data[0]);
	if (ret < 0) {
		fprintf(stderr, "capget error: %s", strerror(errno));
	} else {
		printf("Capabilities of process %8d is:", cap_header.pid);
		printf(" CapInh: 0x%08x%08x", cap_data[1].inheritable, cap_data[0].inheritable);
		printf(" CapEff: 0x%08x%08x", cap_data[1].effective, cap_data[0].effective);
		printf(" CapPrm: 0x%08x%08x", cap_data[1].permitted, cap_data[0].permitted);
		printf("\n");
	}
}

int set_caps(struct __user_cap_data_struct cap_data[2]) {
	int ret = 0;
	struct __user_cap_header_struct	cap_header;
	ret = capget(&cap_header, NULL);
	if (ret < 0) {
		fprintf(stderr, "capget error: %s", strerror(errno));
	}

	cap_header.pid = getpid();
	ret = capset(&cap_header, &cap_data[0]);
	if (ret < 0) {
		fprintf(stderr, "capget error: %s", strerror(errno));
	}
	return ret;
}

int main(int argc, char **argv) {
	struct __user_cap_data_struct cap_data[2];

	list_caps();

	// NOTE: set a capability in the effecitve or inheritable sets that is not
	// in the permitted set will cause EPERM error.
	cap_data[0].inheritable = 0x000000ff;
	cap_data[0].effective = 0x000000ff;
	cap_data[0].permitted = 0x00000fff;
	cap_data[1].inheritable = 0x00000001;
	cap_data[1].effective = 0x00000003;
	cap_data[1].permitted = 0x0000000f;
	set_caps(cap_data);
	list_caps();

	exit(EXIT_SUCCESS);
}

// gcc 0006-set-current-process-cap.c -o 0006-set-current-process-cap
