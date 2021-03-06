#include <stdio.h>
#include <sys/capability.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	int ret;
	struct vfs_cap_data file_cap_data;
	int file_cap_version;

	if (argc != 2) {
		fprintf(stderr ,"Usage: %s pathname\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ret = getxattr(argv[1], "security.capability", &file_cap_data, sizeof(struct vfs_cap_data));
	if (ret < 0) {
		perror("getxattr");
		exit(EXIT_FAILURE);
	}
	// Print the version
	file_cap_version = file_cap_data.magic_etc & VFS_CAP_REVISION_MASK;
	switch (file_cap_version) {
	case VFS_CAP_REVISION_1:
		printf("file cap version 1: 0x%08x\n", file_cap_version);
		break;
	case VFS_CAP_REVISION_2:
		printf("file cap version 2: 0x%08x\n", file_cap_version);
		break;
	default:
		printf("unknown file cap version: 0x%08x\n", file_cap_version);
		break;
	}
	// Printf the effective flag
	printf("file effective cap flag: %d\n", file_cap_data.magic_etc & VFS_CAP_FLAGS_EFFECTIVE);

	// Print permitted and inheritable
	printf("file cap permitted:   0x%08x %08x\n",
			file_cap_data.data[1].permitted,
			file_cap_data.data[0].permitted);
	printf("file cap inheritable: 0x%08x %08x\n",
			file_cap_data.data[1].inheritable,
			file_cap_data.data[0].inheritable);
	exit(EXIT_SUCCESS);
}
/*
# gcc -o 0008-get-file-cap 0008-get-file-cap.c 
# ./0008-get-file-cap /usr/bin/traceroute6.iputils 
file cap version 2: 0x02000000
file effective cap flag: 1
file cap permitted:   0x00000000 00002000
file cap inheritable: 0x00000000 00000000
# getcap /usr/bin/traceroute6.iputils
/usr/bin/traceroute6.iputils = cap_net_raw+ep
*/
