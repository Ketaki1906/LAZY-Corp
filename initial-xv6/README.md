# xv6

## Lazy Fork
1. For this specification, we had to implement a Copy-On-Write (COW) fork for xv6. Its functionalities are as follows -
- When the parent process forks, instead of making a full copy of all the memory pages, we’ll have both processes share the same pages initially.
- These shared pages will be marked as read-only and flagged as “copy-on-write.” This means the processes can share memory until one of them tries to make changes.
- If a process tries to write to one of these shared pages, the RISC-V CPU will raise a page-fault exception.
- At that point, the kernel will jump in to make a duplicate of the page just for that process and map it as read/write, allowing the process to modify its own copy without affecting the other.

2. For implementing Copy-On-Write (COW) fork, following changes were made -
- Used a struct consisting of an array to keep the count of the number of processes referering to a particular memory page (defined in 'kernel/kalloc.c' and initiallised in kinit() and kalloc() functions in 'kernel/kalloc.c'). 
- The refrence count is incremented and decremented using the functions incr_ref() and decr_ref() whenever required.
- In uvmcopy() function ('kernel/vm.c'), we map the child process to the same memory page as the parent process instead of creating it's copy. 
- Whenever a page fault occurs, we check if it's due to COW and handle it using handle_cow_pagefaults() fucntion defined in 'kernel/trap.c'  
