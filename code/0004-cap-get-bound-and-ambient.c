#include <stdio.h>
#include <stdlib.h>
#include <sys/capability.h>
#include <sys/prctl.h>

int main(int argc, char **argv) {
	int ret;
	int cap = 0;
	int cap_data[2] = {0};

	// get bounding capability of the current process
	while ((ret = prctl(PR_CAPBSET_READ, cap)) >= 0) {
		if (ret == 1) {
			if (cap > 31)
				cap_data[1] |= (1<<(cap-32));
			else
				cap_data[0] |= (1<<cap);
		}
		cap++;
	}
	printf("CapBnd: 0x%08x%08x\n", cap_data[1], cap_data[0]);

	cap_data[0] = 0;
	cap_data[1] = 0;
	cap = 0;
	// get ambient capability of the current process
	while ((ret = prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_IS_SET, cap)) >= 0) {
		if (ret == 1) {
			if (cap > 31)
				cap_data[1] |= (1<<(cap-32));
			else
				cap_data[0] |= (1<<cap);
		}
		cap++;
	}
	printf("CapAmb: 0x%08x%08x\n", cap_data[1], cap_data[0]);

	exit(EXIT_SUCCESS);
}

// gcc 0004-cap-get-bound-and-ambient.c -o 0004-cap-get-bound-and-ambient
