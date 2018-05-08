#include <iostream>
#include <cstdlib>
#include "fs_server.h"


using namespace std;


int fs_session(const char *username, const char *password,
                      unsigned int *session_ptr, unsigned int sequence)

{
	return 0;
}

int fs_readblock(const char *username, const char *password,
                   unsigned int session, unsigned int sequence,
                   const char *pathname, unsigned int offset, void *buf)
{

	return 0;
}


int fs_writeblock(const char *username, const char *password,
                         unsigned int session, unsigned int sequence,
                         const char *pathname, unsigned int offset,
                         const void *buf)
{

	return 0;
}

int fs_create(const char *username, const char *password,
                     unsigned int session, unsigned int sequence,
                     const char *pathname, char type)
{

	return 0;
}


int fs_delete(const char *username, const char *password,
                     unsigned int session, unsigned int sequence,
                     const char *pathname)
{


	return 0;
}

