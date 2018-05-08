/*
 * vm_pager.h
 *
 * Header file for the pager
 */

#ifndef _VM_PAGER_H_
#define _VM_PAGER_H_

#include <sys/types.h>
#include "vm_arena.h"

/*
 * ******************************************
 * * Interface for student portion of pager *
 * ******************************************
 */

/*
 * vm_init
 *
 * Called when the pager starts.  It should set up any internal data structures
 * needed by the pager.
 *
 * vm_init is passed the number of physical memory pages and the number
 * of blocks in the swap file.
 */
extern void vm_init(size_t memory_pages, size_t swap_blocks);

/*
 * vm_create
 * Called when a parent process (parent_pid) creates a new process (child_pid).
 * vm_create should cause the child's arena to have the same mappings and data
 * as the parent's arena.  If the parent process is not being managed by the
 * pager, vm_create should consider the arena to be empty.
 * Note that the new process is not run until it is switched to via vm_switch.
 * Returns 0 on success, -1 on failure.
 */
extern int vm_create(pid_t parent_pid, pid_t child_pid);

/*
 * vm_switch
 *
 * Called when the kernel is switching to a new process, with process
 * identifier "pid".
 */
extern void vm_switch(pid_t pid);

/*
 * vm_fault
 *
 * Called when current process has a fault at virtual address addr.  write_flag
 * is true if the access that caused the fault is a write.
 * Returns 0 on success, -1 on failure.
 */
extern int vm_fault(const void *addr, bool write_flag);

/*
 * vm_destroy
 *
 * Called when current process exits.  This gives the pager a chance to
 * clean up any resources used by the process.
 */
extern void vm_destroy();

/*
 * vm_map
 *
 * A request by the current process for the lowest invalid virtual page in
 * the process's arena to be declared valid.  On success, vm_map returns
 * the lowest address of the new virtual page.  vm_map returns nullptr if
 * the arena is full.
 *
 * If filename is nullptr, block is ignored, and the new virtual page is
 * backed by the swap file, is initialized to all zeroes (from the
 * application's perspective), and private (i.e., not shared with any other
 * virtual page).  In this case, vm_map returns nullptr if the swap file is
 * out of space.
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
 * *****************************************************************
 * * Interface for accessing files.  Implemented by infrastructure *
 * *****************************************************************
 *
 * You may assume that no other application accesses a file used
 * by the pager while the pager is running.  You may also assume
 * that, once a file block is accessed successfully, it will remain
 * accessible.
 */

/*
 * file_read
 *
 * Read page from the specified file and block into buf.
 * If filename is nullptr, the data is read from the swap file.  buf should
 * be an address in vm_physmem.
 * Returns 0 on success; -1 on failure.
 */
extern int file_read(const char *filename, size_t block, void *buf);

/*
 * file_write
 *
 * Write page from buf to the specified file and block.
 * If filename is nullptr, the data is written to the swap file.  buf should
 * be an address in vm_physmem.
 * Returns 0 on success; -1 on failure.
 */
extern int file_write(const char *filename, size_t block, const void *buf);

/*
 * *********************************************************
 * * Public interface for the physical memory abstraction. *
 * * Defined in infrastructure.                            *
 * *********************************************************
 *
 * Physical memory pages are numbered from 0 to (memory_pages-1), where
 * memory_pages is the parameter passed in vm_init().
 *
 * Your pager accesses the data in physical memory through the variable
 * vm_physmem, e.g., ((char *)vm_physmem)[5] is byte 5 in physical memory.
 */
extern void * const vm_physmem;
/*
 * **************************************
 * * Definition of page table structure *
 * **************************************
 */

/*
 * Format of page table entry.
 *
 * read_enable=0 ==> loads to this virtual page will fault
 * write_enable=0 ==> stores to this virtual page will fault
 * ppage refers to the physical page for this virtual page (unused if
 * both read_enable and write_enable are 0)
 */
struct page_table_entry_t {
    unsigned int ppage : 20;            /* bit 0-19 */
    unsigned int read_enable : 1;       /* bit 20 */
    unsigned int write_enable : 1;      /* bit 21 */
};

/*
 * Format of page table.  Entries start at the beginning of the arena,
 * i.e., ptes[0] is the page table entry for the first virtual page in the arena
 */
struct page_table_t {
    page_table_entry_t ptes[VM_ARENA_SIZE/VM_PAGESIZE];
};

/*
 * MMU's page table base register.  This variable is defined by the
 * infrastructure, but it is controlled completely by the student's pager code.
 */
extern page_table_t *page_table_base_register;

#endif /* _VM_PAGER_H_ */
