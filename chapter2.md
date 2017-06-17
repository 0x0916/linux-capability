# Process Capabilities and File Capabilities

## Process Capabilities

每个线程都有五个capability set：

* Permitted
* Inheritable
* Effective
* Ambient
* Bounding

### Permitted set

permitted：它表示进程的effective权能集可以设置的超集，即effective的值只能是permitted的子集。同时它也限制了可以添加到inheritable中的权能集。
drop掉permitted权能集上的一个权能后，该权能就在也不能被添加到permitted权能集中了，（除非通过execv执行set-user-id-root程序或者程序具有file Capability)

### Effective set

内核使用该权能集对操作进行检查。

### Inheritable set

### Bounding set

capabilities bounding set 是一个安全机制，用来限制通过execve获得的权能：

使用方式如下：

* 在execve时，bounding和file permitted相与，赋值给进程的permitted set;
* 如果一个权能不在bounding set中，进程则不能添加那个该权能添加到自己的inheritable set中。

### Ambient set

当为非privileged时，ambient Capability 通过execve调用后，该ambient set不会变化。

我的理解，只有当permitted和inheritable中都没有该权能时，ambient中也不会该权能。？？？

当permitted或者inheritable中的某个权能清除后，对应的ambient也的该权能也会自动清楚。

执行一个设置了set-user-ID或者set-group-ID 位的程序 或者 具有file Capability的程序，进程的ambient set湖北清空。

当execve时，进程的ambient capabilities会被添加到permitted set上，并且赋值给了effective set。


### NOTE:

通过fork创建出的子进程会继承父进程的Capability 集。
内核接口/proc/sys/kernel/cap_last_cap导出了内核支持的最大的Capability。
系统调用：capget capset 来查看和设置进程的permitted、effective、inheritable
而bounding、ambient需要通过prctl去设置。

## File Capabilities

文件的Capability保存在文件的扩展属性security.capability上中。文件的Capability和进程的Capability一起来决定在执行execve后，进程的Capability。

file Capability有如下几种：

* Permitted：该Capabilities会自动添加到进程的permitted上。
* Inheritable：该Capability跟进程的inheritable一起，决定进程的inheritable。
* Effective：只是一个单独的位。
  如果设置了这个位：execve后，进程的permitted会设置到进程的effective中;
  如果没有设置了该位，execve后，进程的permitted不会设置到进程的effective中;
  使能了该位，意味着文件的任何permitted和inheritable Capability在execve后，会添加到进程的permitted Capability，同时也会添加到effective Capability中。

### NOTE:

getcap setcap来自于[libcap](https://git.kernel.org/pub/scm/linux/kernel/git/morgan/libcap.git)包中，用来获取和设置文件的Capability
