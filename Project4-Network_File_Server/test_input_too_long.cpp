// test case for invalid username

#include <iostream>
#include <cstdlib>
#include "fs_client.h"

using namespace std;

int main(int argc, char *argv[])
{
    char *server;
    int server_port;
    unsigned int session, session2, seq=0;


    if (argc != 3) {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);
    fs_clientinit(server, server_port);
    fs_session("user1", "password1", &session, seq++);
    fs_create("user1", "password1", session, seq++, "/dir", 'd');

    // username too long
    fs_session("user1", "password1", &session2, seq++);

    // password too long
    fs_session("user1", "password1", &session2, seq++);

    // filename/directory name too long
    cout << fs_create("user1", "password1", session, seq++, "/password1assword1assword1assword1password1assword1assword1assword1password1assword1assword1", 'd') << endl;
    cout << fs_create("user1", "password1", session, seq++, "/password1assword1assword1assword1password1assword1assword1assword1password1assword1assword1", 'f')<< endl;
    cout << fs_create("user1", "password1", session, seq++, "/dir/dirdirdirdirdirdirdirdirdirdir", 'd')<< endl;
    cout << fs_create("user1", "password1", session, seq++, "/dir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir", 'd')<< endl;
    cout << fs_create("user1", "password1", session, seq++, "/dir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir", 'd')<< endl;
    cout << fs_create("user1", "password1", session, seq++, "/dir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir", 'd')<< endl;
    cout << fs_create("user1", "password1", session, seq++, "/dir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir/dirdirdirdirdirdirdirdirdirdir/filefilefilefilefilefilefilefilefileifle", 'd')<< endl;

}
