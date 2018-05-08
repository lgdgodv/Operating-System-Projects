#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    fork();
    char *filename0 = (char *) vm_map(nullptr, 0);
    strcpy(filename0, "shakespeare.txt");
    char *filename1 = (char *) vm_map (nullptr, 0);
    strcpy(filename1, "data1.bin");
	char *filename2 = (char *) vm_map(nullptr, 0);
    strcpy(filename2, "data2.bin");
    strcpy(filename0, "data1.bin");
    strcpy(filename1, "shakespeare.txt");
    cout << filename1 << endl;
    fork();
    
    char* p = (char *) vm_map(filename0, 0);
    cout << p[0];
    
    for(int i = 0; i < 4097; ++i)
    {
        char* filename5 = (char *) vm_map(nullptr, 0);
        strcpy(filename5, "data2.bin");
    }
}
