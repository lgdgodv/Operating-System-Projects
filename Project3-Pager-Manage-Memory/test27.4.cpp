#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0); //600000
    //filename[4095] = 'n';
    strcpy(filename, "data1.bin");
    filename[4087] = 'd';
    filename[4088] = 'a';
    filename[4089] = 't';
    filename[4090] = 'a';
    filename[4091] = '1';
    filename[4092] = '.';
    filename[4903] = 'b';
    filename[4094] = 'i';
    filename[4095] = 'n';
    char *p = (char *) vm_map (filename+4087, 0); // 600004
    cout << p[0] << endl;
    char *p1 = (char *) vm_map((char *) 0x600002000, 0);
}
