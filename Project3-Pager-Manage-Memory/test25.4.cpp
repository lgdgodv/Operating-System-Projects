
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    fork();
    fork();
    char *filename = (char *) vm_map(nullptr, 0); //600000
    char *filename1 = (char *) vm_map(nullptr, 0); // 600001
    strcpy(filename1, "shakespeare.txt");
    char *filename2 = (char *) vm_map(nullptr, 0); // 600002
    strcpy(filename2, "shakespeare.txt");
    char *filename3 = (char *) vm_map(nullptr, 0); // 600003
    strcpy(filename3, "shakespeare.txt");
    fork();
    vm_yield();
    fork();
    vm_yield();
    fork();
    vm_yield();
    char *p3 = (char *) vm_map (filename3, 0); // 600004
    for (unsigned int i=0; i<20; i++)
    {
        cout << p3[i];
    }
    cout << endl;
    
    // test the clock when 0x2 is accessed again
    strcpy(filename2, "shakespeare.txt");
    char *p1 = (char *) vm_map (filename1, 0); // 600005
    
    strcpy(filename2, "shakespeare.txt");
    strcpy(filename, "shakespeare.txt");
    
    for (unsigned int i=0; i<20; i++)
    {
        cout << p1[i];
    }
    cout << endl;
    
    char *p = (char *) vm_map (filename, 0); // 600006
    for (unsigned int i=0; i<20; i++)
    {
        cout << p[i];
    }
    cout << endl;
    fork();
    cout << endl;
    cout << "the end\n";
    strcpy(p1, "hey");
    strcpy(filename, "data1.bin");
    strcpy(filename1, "data1.bin");
    strcpy(filename2, "data1.bin");
    fork();
    for (unsigned int i=0; i<20; i++)
    {
        cout << p1[i];
    }
}
