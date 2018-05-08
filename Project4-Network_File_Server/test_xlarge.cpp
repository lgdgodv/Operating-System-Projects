#include <iostream>
#include <cstdlib>
#include <thread>
#include "fs_client.h"

using namespace std;

unsigned int session,session2, seq=0;
    const char *writedata = "We hold these truths to be self-evident, that all men are created equal, that they are endowed by their Creator with certain unalienable Rights, that among these are Life, Liberty and the pursuit of Happiness. -- That to secure these rights, Governments are instituted among Men, deriving their just powers from the consent of the governed, -- That whenever any Form of Government becomes destructive of these ends, it is the Right of the People to alter or to abolish it, and to institute new Government, laying its foundation on such principles and organizing its powers in such form, as to them shall seem most likely to effect their Safety and Happiness.";

    char readdata[FS_BLOCKSIZE];

void large_test()
{



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

    fs_create("user1", "password1", session, seq++, "/dir/file10", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file11", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file12", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file13", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file14", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file15", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file16", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file17", 'd');

    fs_create("user1", "password1", session, seq++, "/dir/file10/test1", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test2", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test3", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test4", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test5", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test6", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test7", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test8", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9", 'd');

    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner1", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner2", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner3", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner4", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner5", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner6", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner7", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner8", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/inner9", 'f');

    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/innDir", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/whatever", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/blur", 'f');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/hey", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever", 'd');
    fs_create("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 'f');

    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 1, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 2, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 3, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 4, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 5, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 6, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 7, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 8, writedata);
    fs_readblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 8, readdata);

    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/inner1", 0, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/inner1", 1, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/inner1", 2, writedata);
    fs_writeblock("user1", "password1", session, seq++, "/dir/file10/test9/inner1", 3, writedata);

    // correct:
    fs_readblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 2, readdata);

    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner1");
    // error checking:
    fs_readblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 0, readdata);

    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner2");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner3");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner4");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner5");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner6");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner7");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner8");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner9");

    // test double delete
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner2");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner3");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner4");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner5");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner6");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner7");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner8");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/inner9");

    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever");
    // error checking:
    fs_readblock("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever/whatever", 0, readdata);

    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/whatever/whatever");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/whatever");

    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/innDir");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/blur");
    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9/hey");

    fs_delete("user1", "password1", session, seq++, "/dir/file10/test9");


}

void very_good()
{
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

int main(int argc, char *argv[])

{
    char *server;
    int server_port;

    if (argc != 3) 
    {
        cout << "error: usage: " << argv[0] << " <server> <serverPort>\n";
        exit(1);
    }
    server = argv[1];
    server_port = atoi(argv[2]);
    fs_clientinit(server, server_port);


	int i = 0;
    while(i<50)
    {
        thread myThread = thread(large_test);
		thread YourThread = thread(very_good);
        myThread.join();
YourThread.join();
		i++;
    }


    
}
