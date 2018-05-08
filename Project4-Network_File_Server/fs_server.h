/*
 * fs_server.h
 *
 * Header file for the file server.
 */

#ifndef _FS_SERVER_H_
#define _FS_SERVER_H_

#include <sys/types.h>
#include <mutex>
#include <sys/socket.h>
 #include <unistd.h>

#include "fs_param.h"
#include "fs_crypt.h"
#include "fs_client.h"


/*
 * Size of the disk (in blocks)
 */
static const unsigned int FS_DISKSIZE = 4096;

/*
 * Definitions for on-disk data structures.
 */
struct fs_direntry {
    char name[FS_MAXFILENAME + 1];         // name of this file or directory
    uint32_t inode_block;                  // disk block that stores the inode for
                                           // this file or directory
};

struct fs_inode {
    char type;                             // file ('f') or directory ('d')
    char owner[FS_MAXUSERNAME + 1];
    uint32_t size;                         // size of this file or directory
                                           // in blocks
    uint32_t blocks[FS_MAXFILEBLOCKS];     // array of data blocks for this
                                           // file or directory
};

/*
 * Number of direntries that can fit in one block
 */
static const unsigned int FS_DIRENTRIES = (FS_BLOCKSIZE / sizeof(fs_direntry));

/*
 * Mutexes to prevent garbled output from a multi-threaded file server.
 * Your file server must wrap all calls to cout inside a critical section
 * protected by cout_lock.
 */
extern std::mutex cout_lock;

/*
 * Global variable to control debugging output for send and close.
 */
extern bool fs_quiet;

/*
 * Interface to the disk.
 *
 * Disk blocks are numbered from 0 to (FS_DISKSIZE-1).
 * disk_readblock and disk_writeblock are both thread safe, i.e., multiple
 * threads can safely make simultaneous calls to these functions.
 */

/*
 * disk_readblock
 *
 * Copies disk block "block" into buf.  Asserts on failure.
 */
extern void disk_readblock(unsigned int block, void *buf);

/*
 * disk_writeblock
 *
 * Copies buf to disk block "block".  Asserts on failure.
 */
extern void disk_writeblock(unsigned int block, const void *buf);

/*
 * Global variable to control debugging output for disk operations.
 */
extern bool disk_quiet;

#endif /* _FS_SERVER_H_ */
