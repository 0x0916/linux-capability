# Capabiltiy 示例

## Capability的设定和清除

下面的示例程序给当前的进程设定Capability，最后我们清除掉所设置的Capability，源代码如下：

```c
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
```

编译运行如下：

```bash
# gcc capsettest.c -o capsettest -lcap
# ./capsettest 
uid=0 euid=0 gid=0
The process 21428 was give capabilities = cap_setgid,cap_setuid,cap_setpcap,cap_net_bind_service,cap_net_raw+eip
dropping caps
The process 21428 was give capabilities =
```

* 我们对该进程增加了5种能力,随后又清除了所有能力。
* 首先通过cap_init()初始化存放cap能力值的状态，随后通过cap_set_flag函数的调用，将三种位图的能力设置给了变量caps，再通过cap_set_proc(caps)设定当前进程的能力值，通过cap_get_proc()返回当前进程的能力值，最后通过cap_free(caps)释放能力值。
* cap_set_flag函数的原型是

```
int cap_set_flag(cap_t cap_p, cap_flag_t flag, int ncap,const cap_value_t *caps, cap_flag_value_t value);
我们这里的调用语句是:cap_set_flag(caps, CAP_PERMITTED, num_caps, capList, CAP_SET);
第一个参数cap_p是存放能力值的变量，是被设定值.这里是caps。
第二个参数flag是是三种能力位图，这里是CAP_PERMITTED。
第三个参数ncap是要设定能力的个数，这里是num_caps，也就是5。
第四个参数*caps是要设定的能力值，这里是capList数组，也就是CAP_NET_RAW, CAP_NET_BIND_SERVICE , CAP_SETUID, CAP_SETGID,CAP_SETPCAP。
第五个参数value是决定要设定还是清除，这里是CAP_SET。
```
* cap_set_proc函数的原型是

```
int cap_set_proc(cap_t cap_p);
cap_set_proc函数通过cap_p中的能力值设定给当前的进程。
```

* cap_get_proc函数的原型是

```
cap_t cap_get_proc(void);
cap_get_proc函数返回当前进程的能力值给cap变量。
```

* cap_free函数的原型是

```cap_free(caps);
cap_free函数清理/释放cap变量。
```

* 如果我们fork()了子进程，那么子进程继承父进程的所有能力。
* 不能单独设定CAP_EFFECTIVE，CAP_INHERITABLE位图，必须要和CAP_PERMITTED联用，且CAP_PERMITTED一定要是其它两个位图的超集。
* 如果两次调用cap_set_proc函数，第二次调用的值力值不能少于或多于第一次调用，如第一次我们授权chown,setuid能力，第二次只能是chown,setuid不能是其它的能力值。
* 普通用户不能给进程设定能力。

## 通过capget和capset来获取和设定进程的Capability

下面的程序通过capget函数来获取当前进程的Capability。代码如下：

```c
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/capability.h>
#include <errno.h>


int main()
{
	struct __user_cap_header_struct cap_header_data;
	cap_user_header_t cap_header = &cap_header_data;

	struct __user_cap_data_struct cap_data_data;
	cap_user_data_t cap_data = &cap_data_data;

	cap_header->pid = getpid();
	cap_header->version = _LINUX_CAPABILITY_VERSION_3;

	if (capget(cap_header, cap_data) < 0) {
		perror("FAILED capget()");
		exit(1);
	}

	printf("Cap data 0x%x, 0x%x, 0x%x\n", cap_data->effective, cap_data->permitted, cap_data->inheritable);
	return 0;
}
```

运行如下：

```bash
# gcc -o capget capget.c 
# root用户
# ./capget 
Cap data 0xffffffff, 0xffffffff, 0x0
# 非root用户
$ ./capget 
Cap data 0x0, 0x0, 0x0
```

这说明了默认情况下，root运行的程序什么权限都有，而普通用户则什么权限都没有。
