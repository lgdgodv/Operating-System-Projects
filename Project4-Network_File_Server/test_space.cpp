#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[])

{
    char *server;
    int server_port;
    unsigned int session, seq = 0;


    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);

    fs_clientinit(server, server_port);
    cout << "wumuting headsome" << endl;
    fs_session("user1", "password1", &session, seq++);
    cout << "eric headsome" << endl;
    fs_create("user1", "password1", session, seq++, "/dir", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/fi le", 'f');
	fs_create("user1", "password1", session, seq++, "/dir/fil\0e", 'f');
	fs_create("user1", "password1", session, seq++, "/dir/file2", 'f');
}
