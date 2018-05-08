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

    if (argc != 3) 
    {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);
	cout << "eric sb" << endl;
    fs_clientinit(server, server_port);
    cout << "wumuting headsome" << endl;
    fs_session("user1", "password1", &session, seq++);
    cout << "eric headsome" << endl;
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/faker", 'd');
    fs_create("user1", "password1", session, seq++, "/faker/wu", 'd');
    fs_create("user1", "password1", session, seq++, "/faker/nig", 'f');
    fs_create("user1", "password1", session, seq++, "/faker/wu/eric", 'd');
    fs_create("user1", "password1", session, seq++, "/faker/wu/junyue", 'd');
    fs_create("user1", "password1", session, seq++, "/faker/wu/junyue/junyue", 'd');
    fs_create("user1", "password1", session, seq++, "/faker/wu/junyue/junyue/junyue", 'f');
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 0, readdata);
    fs_writeblock("user1", "password1", session, seq++, "/faker/wu/junyue/junyue/junyue", 0, writedata);
    fs_readblock("user1", "password1", session, seq++, "/faker/wu/junyue/junyue/junyue", 0, readdata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 0, readdata);
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir");
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_writeblock("user1", "password1", session, seq++, "/dir/file", 0, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file", 0, readdata);
    fs_delete("user1", "password1", session, seq++, "/dir/file");
    fs_delete("user1", "password1", session, seq++, "/dir");
    fs_delete("user1", "password1", session, seq++, "/faker/wu/junyue/junyue/junyue");
    fs_create("user1", "password1", session, seq++, "/faker/wu/junyue/junyue/junyue", 'f');
    fs_create("user1", "password1", session, seq++, "/faker/wu/junyue/junyue/junyue1", 'f');
    fs_create("user1", "password1", session, seq++, "/faker/wu/junyue/junyue2", 'f');
    fs_create("user1", "password1", session, seq++, "/faker/wu/junyue3", 'f');
    fs_writeblock("user1", "password1", session, seq++, "/faker/wu/junyue/junyue2", 0, writedata);
    fs_readblock("user1", "password1", session, seq++, "/faker/wu/junyue/junyue2", 0, readdata);
    fs_delete("user1", "password1", session, seq++, "/faker/wu/junyue/junyue2");
}
