# Capabiltiy example

## 使用capget查看内核 capabilities的版本号

由于历史原因，capability有三个版本，我们可以通过capget检测内核中国年capabilities的版本号：

```c
#include <stdio.h>
#include <string.h>
#include <sys/capability.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	int ret;
	struct __user_cap_header_struct	cap_header;

	ret = capget(&cap_header, NULL);
	if (ret < 0) {
		fprintf(stderr, "capget error: %s", strerror(errno));
	}

	switch(cap_header.version) {
	case  _LINUX_CAPABILITY_VERSION_1:
		printf("cap version 1: 0x%x\n", cap_header.version);
		break;
	case  _LINUX_CAPABILITY_VERSION_2:
		printf("cap version 2: 0x%x\n", cap_header.version);
		break;
	case  _LINUX_CAPABILITY_VERSION_3:
		printf("cap version 3: 0x%x\n", cap_header.version);
		break;
	default:
		printf("unknown version: 0x%x\n", cap_header.version);
		break;
	}

	exit(EXIT_SUCCESS);
}

```

编译运行如下：

```bash
# gcc 0001-get-kernel-cap-version.c -o 0001-get-kernel-cap-version
```

## 查看可以设置的最大的Capability的值

```c
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

```
编译运行如下：

```bash
# gcc 0003-cap-last-cap.c -o 0003-cap-last-cap
```


## 使用capget获取当前进程的permitted、effective、inheritable 

```c
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
```



## 使用prctl获取当前进程的bounding、ambient


```c

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
```



## 获取任意进程的permitted、effective、inheritable


```c
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
```


## 使用capset设置进程的permitted、effective、inheritable

```c

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
```


## 获取file Capability的版本

```c

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

	exit(EXIT_SUCCESS);
}

/*
# gcc -o 0007-get-file-cap-version 0007-get-file-cap-version.c 
# ./0007-get-file-cap-version /usr/bin/traceroute6.iputils 
file cap version 2: 0x02000000
*/
```

## 使用getxattr获取文件的capabilities

```c

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
```


## 使用getxattr设置文件的capabilities

```c

#include <stdio.h>
#include <sys/capability.h>
#include <stdlib.h>
#include <sys/xattr.h>
#include <sys/types.h>

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

	// Set the file cap: CAP_CHOWN CAP_FSETID
	printf("set file permitted cap: CAP_CHOWN and CAP_FSETID\n");
	file_cap_data.data[0].permitted |= 0x11;

	ret = setxattr(argv[1], "security.capability", &file_cap_data, sizeof(struct vfs_cap_data), 0);
	if (ret < 0) {
		perror("setxattr");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
/*
# gcc -o 0009-set-file-cap 0009-set-file-cap.c
# getcap /usr/bin/traceroute6.iputils
/usr/bin/traceroute6.iputils = cap_net_raw+ep
# ./0009-set-file-cap /usr/bin/traceroute6.iputils
file cap version 2: 0x02000000
file effective cap flag: 1
file cap permitted:   0x00000000 00002000
file cap inheritable: 0x00000000 00000000
set file permitted cap: CAP_CHOWN and CAP_FSETID
# getcap /usr/bin/traceroute6.iputils
/usr/bin/traceroute6.iputils = cap_chown,cap_fsetid,cap_net_raw+ep
*/
```
