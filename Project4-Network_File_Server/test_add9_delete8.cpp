#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, seq=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amon";

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
	fs_create("user1", "password1", session, seq++, "/dir/file1", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file2", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file3", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file4", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file5", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file6", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file7", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file8", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file9", 'f');


    fs_delete("user1", "password1", session, seq++, "/dir/file1");
    fs_delete("user1", "password1", session, seq++, "/dir/file2");
    fs_delete("user1", "password1", session, seq++, "/dir/file3");
    fs_delete("user1", "password1", session, seq++, "/dir/file4");
    fs_delete("user1", "password1", session, seq++, "/dir/file5");
    fs_delete("user1", "password1", session, seq++, "/dir/file6");
    fs_delete("user1", "password1", session, seq++, "/dir/file7");
    fs_delete("user1", "password1", session, seq++, "/dir/file8");

    fs_writeblock("user1", "password1", session, seq++, "/dir/file9", 0, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file9", 0, readdata);

    // error checking test
    fs_create("user1", "password1", session, seq++, "/test", 'f');
    fs_writeblock("user1", "password1", session, seq++, "/test", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file9", 1, writedata);
    cout << "error checking: \n";
    cout << fs_delete("user1", "password1", session, seq++, "/test/file9") << endl;

    // create again
    // fs_create("user1", "password1", session, seq++, "/dir", 'd');
    // fs_create("user1", "password1", session, seq++, "/dir/file1", 'f');
    // fs_create("user1", "password1", session, seq++, "/dir/file2", 'f');
    // fs_create("user1", "password1", session, seq++, "/dir/file3", 'f');
    // fs_create("user1", "password1", session, seq++, "/dir/file4", 'f');
    // fs_create("user1", "password1", session, seq++, "/dir/file5", 'f');
    // fs_create("user1", "password1", session, seq++, "/dir/file6", 'f');
    // fs_create("user1", "password1", session, seq++, "/dir/file7", 'f');
    // fs_create("user1", "password1", session, seq++, "/dir/file8", 'f');

    fs_delete("user1", "password1", session, seq++, "/dir/file9");
    cout << "valid delete\n";
    cout << fs_delete("user1", "password1", session, seq++, "/dir") << endl;


}
