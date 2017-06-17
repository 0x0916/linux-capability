#  执行execve时进程capability的变化


```
P'(ambient)   =  (file is privileged) ? 0 : P(ambient)
P'(permitted) = (P(inheritable) & F(inheritable)) | 
                           (F(permitted) & cap_bset)  |
                            P'(ambient)

P'(effective)     = F(effective)? P'(permitted) : P'(ambient)
P'(inheritable)   = P(inheritable)  (unchanged)


P  a thread capability set before the execve
P' a thread capability set after the execve
F  a file capabilities set
cap_bset   the value of the capability bounding set
A privileged file is one that has capabilities or has the set-user-ID or set-group-ID bit set.
```

* 什么是 privileged file： 
	- 具有file capabilities的文件    或者
	- 设置了set-user-ID或者set-group-ID的文件
