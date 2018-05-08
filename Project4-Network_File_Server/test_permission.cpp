#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, seq=0;
	unsigned int session2, seq2=0;

    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amonWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amonWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amonWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amonWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amonWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amonWe hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted amon";

    char readdata[FS_BLOCKSIZE];

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
	fs_session("user2", "password2", &session2, seq2++);
    cout << fs_create("user1", "password1", session, seq++, "/file", 'f') << endl;
    cout << fs_writeblock("user1", "password1", session, seq++, "/file", 0, writedata) << endl;

    cout << fs_readblock("user2", "password2", session2, seq2++, "/file", 0, readdata) << endl;
    cout << fs_writeblock("user2", "password2", session2, seq2++, "/file", 1, writedata) << endl;
    cout << fs_readblock("user2", "password2", session2, seq2++, "/file", 0, readdata) << endl;
    cout << fs_readblock("user2", "password2", session2, seq2++, "/file", 0, readdata) << endl;
    cout << fs_readblock("user1", "password1", session, seq++, "/file", 2, readdata) << endl;
    cout << fs_readblock("user1", "password1", session, seq++, "/file", 0, readdata) << endl;


}
