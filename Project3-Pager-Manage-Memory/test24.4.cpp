
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    char *filename = (char *) vm_map(nullptr, 0); //600000
    strcpy(filename, "shakespeare.txt");
    fork();
    char *filename1 = (char *) vm_map(nullptr, 0); // 600001
    strcpy(filename1, "shakespeare.txt");
    char *filename2 = (char *) vm_map(nullptr, 0); // 600002
    strcpy(filename2, "shakespeare.txt");
    char *filename3 = (char *) vm_map(nullptr, 0); // 600003
    strcpy(filename3, "shakespeare.txt");
    cout << filename[0] << endl;
    strcpy(filename, "shakespeare.txt");
}
