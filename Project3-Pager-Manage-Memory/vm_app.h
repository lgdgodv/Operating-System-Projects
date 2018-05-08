/*
 * vm_app.h
 *
 * Public routines for clients of the pager
 */

#ifndef _VM_APP_H_
#define _VM_APP_H_

#include "vm_arena.h"
#include <sys/types.h>

/*
 * vm_map
 *
 * Ask for the lowest invalid virtual page in the process's arena to be
 * declared valid.  On success, vm_map returns the lowest address of the
 * new virtual page.  vm_map returns nullptr if the arena is full.
 *
 * If filename is nullptr, block is ignored, and the new virtual page is
 * backed by the swap file, is initialized to all zeroes, and is private
 * (i.e., not shared with any other virtual page).  In this case, vm_map
 * returns nullptr if the swap file is out of space.
 *
 * If filename is not nullptr, the new virtual page is backed by the specified
 * file at the specified block and is shared with other virtual pages that are
 * mapped to that file and block.  filename is a null-terminated C string and
 * must reside completely in the valid portion of the arena.  In this case,
 * vm_map returns nullptr if filename is not completely in the valid part of
 * the arena.
 * filename is specified relative to the pager's current working directory.
 */
extern void *vm_map(const char *filename, size_t block);

/* 
 * vm_yield
 *
 * Ask operating system to yield the CPU to another process.
 * The infrastructure's scheduler is non-preemptive, so a process runs until
 * it calls vm_yield() or exits.
 */
extern void vm_yield(void);

#endif /* _VM_APP_H_ */
