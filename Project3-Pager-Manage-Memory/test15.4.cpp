#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0); // 600000
    strcpy(filename, "shakespeare.txt");
    char *p = (char *) vm_map (filename, 0); // 600001
    for (unsigned int i=0; i<20; i++) 
    {
		cout << p[i];
    }
    cout << endl;

    strcpy(p, "data1.bin");
    char *p1 = (char*) vm_map(p, 1); // 600002
    char *p2 = (char*) vm_map(p, 1); // 600003
    //fork();


    for (unsigned int i=0; i<20; i++) 
    {
        cout << p2[i];
    }
    cout << endl;

    strcpy(p1, "data2.bin");
    //fork();

    char *p3 = (char*) vm_map(p2, 3); // 600004
    for (unsigned int i=0; i<20; i++) 
    {
        cout << p3[i];
    }
    cout << endl;
    //fork();

    strcpy(p3, "shakespeare.txt"); 
    char *p4 = (char*) vm_map(p3, 0); // 600005
    for (unsigned int i=0; i<20; i++) 
    {
        cout << p4[i];
    }
    cout << endl;
    //fork();

    char *filename1 = (char *) vm_map(nullptr, 0); // 600006
    strcpy(filename1, "shakespeare.txt");
    char *filename2 = (char *) vm_map(nullptr, 0); // 600007
    strcpy(filename2, "shakespeare.txt");
    char *filename3 = (char *) vm_map(nullptr, 0); // 600008
    strcpy(filename3, "shakespeare.txt");
    char *filename4 = (char *) vm_map(nullptr, 0); // 600009
    strcpy(filename4, "shakespeare.txt");
    cout << "first program exitting\n";

    char *file = (char *) vm_map(nullptr, 0); // 60000a
    char *file1 = (char *) vm_map(nullptr, 0); // 60000b
    file[4090] = 'd';
    file[4091] = 'a';
    file[4092] = 't';
    file[4093] = 'a';
    file[4094] = '1';
    file[4095] = '.';
    file1[0] = 'b';
    file1[1] = 'i';
    file1[2] = 'n';
    file1[3] = '\0';

    cout << (file+4090) << endl;

    char *p5 = (char*) vm_map(file+4090, 0); // 60000c
    for (unsigned int i=0; i<20; i++) 
    {
        cout << p5[i];
    }
    cout << endl;
    return 0;
}
