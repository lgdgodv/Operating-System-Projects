
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0); //600000
    char *filename1 = (char *) vm_map(nullptr, 0); //600001
    char *filename2 = (char *) vm_map(nullptr, 0); //600002
    strcpy(filename, "data2.bin"); //0x1
    strcpy(filename1, "data1.bin"); //0x2
    strcpy(filename2, "data1.bin"); //0x3
    fork();
    char *filename3 = (char *) vm_map(nullptr, 0); //600003
    strcpy(filename3, "data1.bin");
    // doing clock rotation: evict 0x1
    char *p = (char *) vm_map (filename2, 0);

    // write to forked dirty page which is in memory
    strcpy(filename2, "data1.bin");

    // write to forked dirty page which is clocked
    strcpy(filename1, "data1.bin");

    // try to evict the forked page
    strcpy(filename, "data2.bin");
    cout << p[0] << endl; // evict page 0x1

    char *p1 = (char *) vm_map (filename, 0);
    cout << p1[0] << endl;
}
