// testcase for not increasing sequence number

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
    fs_create("user1", "password1", session, 10, "/dir", 'd');
    fs_create("user1", "password1", session, 20, "/dir/file", 'f');
    fs_create("user1", "password1", session, 30, "/faker", 'd');

    cout << "testing1\n";
    // ------------------------------------------------------------------------------------
    // testing for incorrect sequence number, sequence should be increasing
    fs_create("user1", "password1", session, 25, "/faker/wu", 'd');
    fs_writeblock("user1", "password1", session, 23, "/dir/file", 0, writedata);
    fs_readblock("user1", "password1", session, 24, "/dir/file", 0, readdata);
    fs_delete("user1", "password1", session, 26, "/dir/file");

    cout << "testing2\n";
    // ------------------------------------------------------------------------------------
    // test for incorrect session number, sequence is correct, 31-34 increasing
    fs_create("user1", "password1", session+1, 31, "/faker/wu", 'd');
    fs_writeblock("user1", "password1", session+1, 32, "/dir/file", 0, writedata);
    fs_readblock("user1", "password1", session+1, 33, "/dir/file", 0, readdata);
    fs_delete("user1", "password1", session+1, 34, "/dir/file");

    cout << "testing3\n";
    // ------------------------------------------------------------------------------------
    // test for whether incorrect request is executed or not
    // if not executed, the following code should be valid, since the pathname is not taken
    seq = 35;
    fs_create("user1", "password1", session, seq++, "/faker/wu", 'd');
    fs_create("user1", "password1", session, seq++, "/faker/nig", 'f');

    cout << "testing4\n";
    // ------------------------------------------------------------------------------------
    // test for incorrect username, nothing should returned
    fs_create("junyuew", "password1", session, seq++, "/dir/test", 'f');
    fs_writeblock("junyuew", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_readblock("junyuew", "password1", session, seq++, "/dir/file", 0, readdata);
    fs_delete("junyuew", "password1", session, seq++, "/dir/file");

    cout << "testing5\n";
    // ------------------------------------------------------------------------------------
    // test for incorrect password, nothing should returned
    fs_create("user1", "wrong", session, seq++, "/dir/test", 'f');
    fs_writeblock("user1", "wrong", session, seq++, "/dir/file", 0, writedata);
    fs_readblock("user1", "wrong", session, seq++, "/dir/file", 0, readdata);
    fs_delete("user1", "wrong", session, seq++, "/dir/file");

    cout << "testing6\n";
    // ------------------------------------------------------------------------------------
    // test for duplicate pathway: under filename
    fs_create("user1", "password1", session, seq++, "/dir/file", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    cout << "testing7\n";
    // ------------------------------------------------------------------------------------
    // test for duplicate pathway: under directory
    fs_create("user1", "wrong", session, seq++, "/dir", 'd');
    fs_create("user1", "wrong", session, seq++, "/dir", 'f');
    cout << "testing8\n";
}
