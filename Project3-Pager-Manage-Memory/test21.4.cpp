
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
    strcpy(filename, "shakespeare.txt");
}
