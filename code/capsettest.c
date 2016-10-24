#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/capability.h>

extern int errno;

void whoami(void) {
	printf("uid=%i euid=%d gid=%i\n", getuid(), geteuid(), getgid());
}

void list_caps() {
	// cap_get_proc() allocates a capability state in working stroage, sets
	// its state to taht of the calling process, and returns a pointer to thei newly
	// create capability state.
	cap_t caps = cap_get_proc();
	ssize_t	y = 0;

	printf("The process %d was give capabilities %s\n", 
		(int)getpid(), cap_to_text(caps, &y));

	fflush(0);

	// free the releaseable memory
	if (cap_free(caps) == -1) {
		perror("cap_free()");
	}
}

int main(int argc, char **argv) {
	int stat;
	whoami();

	stat = setuid(geteuid());
	pid_t   parent_pid = getpid();

	if (!parent_pid) {
		perror("parent_pid == 0");
		return 1;
	}

	// cap_init() creates a capability state in working storage and returns
	// a pointer to the capability state. the initial value of all flags
	// are cleared.
	cap_t caps = cap_init();

	// CAP_IS_SUPPORTED(cap_value_t cap) evaluates to true if the system supports the specified capability cap.
	// If the system does not supports the specified capability, the function returns 0.
	// this macro works by testing for an error condition with cap_get_bound()
	if (!CAP_IS_SUPPORTED(CAP_NET_RAW)) {
		perror("do not support CAP_NET_RAW");
		return 1;
	}

	cap_value_t	cap_list[5] =
		 {CAP_NET_RAW, CAP_NET_BIND_SERVICE, CAP_SETUID, CAP_SETGID, CAP_SETPCAP};
	unsigned num_caps = 5;
	cap_set_flag(caps, CAP_EFFECTIVE, num_caps, cap_list, CAP_SET);
	cap_set_flag(caps, CAP_INHERITABLE, num_caps, cap_list, CAP_SET);
	cap_set_flag(caps, CAP_PERMITTED, num_caps, cap_list, CAP_SET);

	if (cap_set_proc(caps) == -1) {
		perror("cap_set_proc()");
		return EXIT_FAILURE;
	}
	list_caps();

	printf("dropping caps\n");
	cap_clear(caps);
	if (cap_set_proc(caps) == -1) {
		perror("cap_set_proc()");
		return EXIT_FAILURE;
	}
	list_caps();

	if (cap_free(caps) == -1) {
		perror("cap_free()");
		return EXIT_FAILURE;
	}

	return 0;

}

//    $ gcc  capsettest.c  -o capsettest  -lcap

//    $ sudo ./capsettest 
//    uid=0 euid=0 gid=0
//    The process 20852 was give capabilities = cap_setgid,cap_setuid,cap_setpcap,cap_net_bind_service,cap_net_raw+eip
//    dropping caps
//    The process 20852 was give capabilities =

