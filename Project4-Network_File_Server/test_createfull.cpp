#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, seq=0;

    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
	fs_create("user1", "password1", session, seq++, "/home", 'd');
    fs_create("user1", "password1", session, seq++, "/dir1", 'd');
	fs_create("user1", "password1", session, seq++, "/home1", 'd');
    fs_create("user1", "password1", session, seq++, "/dir2", 'd');
	fs_create("user1", "password1", session, seq++, "/home2", 'd');
    fs_create("user1", "password1", session, seq++, "/dir3", 'd');
	fs_create("user1", "password1", session, seq++, "/home3", 'd');
    fs_create("user1", "password1", session, seq++, "/dir4", 'd');
	fs_create("user1", "password1", session, seq++, "/home4", 'd');
    fs_create("user1", "password1", session, seq++, "/dir5", 'd');
	fs_create("user1", "password1", session, seq++, "/home5", 'd');
    fs_create("user1", "password1", session, seq++, "/dir6", 'd');
	fs_create("user1", "password1", session, seq++, "/home6", 'd');
    fs_create("user1", "password1", session, seq++, "/dir7", 'd');
	fs_create("user1", "password1", session, seq++, "/home7", 'd');

}
