# Capability 概述

为了进行权限检查，传统的Linux系统将进程分为了两类：privileged 进程（effective UID为0） 和 unprivileged 进程（effective UID为非0）。privileged 进程可以绕过所有的内核权限检查，而unprivileged 进程要进行严格的权限检查。

从2.2版本的内核开始，linux对权限进行了细分，分为很多各种不同的权限，我们称之为 capability。

capability 可以作用在进程上，也可以作用在程序文件上。

每个进程都有四个和capability有关的位图：

* Inheritable
* Permitted
* Effective
