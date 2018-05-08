// testcase for not increasing sequence number

#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, session2, seq=0;

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
    fs_session("user2", "password2", &session2, seq++);

    // ------------------------------------------------------------------------------------
    // test for user1 using user2's session number
    fs_create("user1", "password1", session2, seq++, "/test", 'd');
    fs_create("user2", "password2", session, seq++, "/test/test", 'd');

    // correct: user 1 create file
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');

    // ------------------------------------------------------------------------------------
    // test for duplicate filename
    fs_create("user1", "password1", session2, seq++, "/dir", 'd');
    fs_create("user2", "password2", session2, seq++, "/dir", 'f');
    fs_create("user1", "password1", session2, seq++, "/dir/file", 'd');
    fs_create("user2", "password2", session2, seq++, "/dir/file", 'f');

    // ------------------------------------------------------------------------------------
    // test for user2 want to create file to dir, which is created by user1
    fs_create("user2", "password2", session2, seq++, "/dir/file2", 'd');
    fs_create("user2", "password2", session2, seq++, "/dir/file2", 'f');

    // correct: user1 write to block 0
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);

    // ------------------------------------------------------------------------------------
    // test for user2 want to read/write to file created by user1
    fs_writeblock("user2", "password2", session2, seq++, "/dir/file", 0, writedata);
    fs_readblock("user2", "password2", session2, seq++, "/dir/file", 0, readdata);

    // ------------------------------------------------------------------------------------
    // test for user2 want to delete file created by user1
    fs_delete("user2", "password2", session2, seq++, "/dir/file");  

    // correct: user 1 delete file
    fs_delete("user1", "password1", session, seq++, "/dir/file");  

    // ------------------------------------------------------------------------------------
    // test for user2 want to delete directory created by user1
    fs_delete("user2", "password2", session2, seq++, "/dir");  

    // ------------------------------------------------------------------------------------
    // test for user2 want to access incorrect filename under user1 directory
    fs_create("user2", "password2", session2, seq++, "/dir/wrong/test", 'f');
    fs_writeblock("user2", "password2", session2, seq++, "/dir/wrong/test", 0, writedata);
    fs_readblock("user2", "password2", session2, seq++, "/dir/wrong/test", 0, readdata);
    fs_delete("user2", "password2", session2, seq++, "/dir/wrong/test");

    cout << "the end\n";
}
