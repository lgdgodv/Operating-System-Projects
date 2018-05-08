/*
 * disk.h -- public interface to disk scheduler output functions.
 *
 */
#ifndef _DISK_H
#define _DISK_H

extern void print_request(unsigned int requester, unsigned int track);
extern void print_service(unsigned int requester, unsigned int track);

#endif /* _DISK_H */
