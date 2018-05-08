// test case for invalid username

#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[])
{
    char *server;
    int server_port;
    unsigned int session, seq=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);
    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
    fs_create("user1", "password1", session, seq++, "/dir", 'd');

    // ------------------------------------------------------------------------------------
    // test for incorrect fs_create type
    fs_create("user1", "password1", session, seq++, "/dir/file", 'w');

    // correct
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    // ------------------------------------------------------------------------------------
    // test for create directory and filename under filename
    fs_create("user1", "password1", session, seq++, "/dir/file/wrong", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file/wrong", 'f');

    // ------------------------------------------------------------------------------------
    // test for read/write from directory 
    fs_writeblock("user1", "password1", session, seq++, "/dir", 0, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir", 0, readdata);

    // ------------------------------------------------------------------------------------
    // test for incorrect offset for read/write from file
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 5, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 7, readdata);

}
