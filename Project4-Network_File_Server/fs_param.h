/*
 * fs_param.h
 *
 * File server parameters (used by both clients and server).
 */

#ifndef _FS_PARAM_H_
#define _FS_PARAM_H_

/*
 * File system parameters
 */

/*
 * Size of a disk block (in bytes)
 */
static const unsigned int FS_BLOCKSIZE = 512;

/*
 * Maximum # of data blocks in a file or directory.  Computed so that
 * an inode is exactly 1 block.
 */
static const unsigned int FS_MAXFILEBLOCKS = 124;

/*
 * Maximum length of a file or directory name, not including the null terminator
 */
static const unsigned int FS_MAXFILENAME = 59;

/*
 * Maximum length of a full pathname, not including the null terminator
 */
static const unsigned int FS_MAXPATHNAME = 128;

/*
 * Maximum length of a user name, not including the null terminator
 */
static const unsigned int FS_MAXUSERNAME = 10;

/*
 * Maximum length of a password, not including the null terminator
 */
static const unsigned int FS_MAXPASSWORD = 15;

#endif /* _FS_PARAM_H_ */
