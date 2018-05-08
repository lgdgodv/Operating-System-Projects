/*
 * vm_arena.h
 *
 * Constants describing the arena provided by the pager.
 */

#ifndef _VM_ARENA_H_
#define _VM_ARENA_H_

#include <stdint.h>

/* page size for the machine */
static const unsigned int VM_PAGESIZE = 4096;

/* virtual address at which the application's arena starts */
static void * const VM_ARENA_BASEADDR = (void *) 0x600000000;

/* size (in bytes) of arena */
static const uintptr_t VM_ARENA_SIZE = 0x01000000;

#endif /* _VM_ARENA_H_ */
