#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    fork();
    char *filename = (char *) vm_map(nullptr, 0);
    strcpy(filename, "shakespeare.txt");
    char *p = (char *) vm_map (filename, 0);
    for (unsigned int i=0; i<20; i++) 
    {
		cout << p[i];
    }
    fork();
    fork();
    fork();
    strcpy(filename, "data1.bin");
    return 0;
}
