int handle_cow_pagefaults (pagetable_t pagetable, uint64 va);
void incr_ref (uint64 pa);
int decr_ref (uint64 pa);
int get_ref_count(uint64 pa);
