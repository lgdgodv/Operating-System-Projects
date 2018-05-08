#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
	//10 pin
 	char *filename0 = (char *) vm_map(nullptr, 0);
    char *filename1 = (char *) vm_map(nullptr, 0);

	strcpy(filename1, "shakespeare.txt");

	fork();
	fork();
	
	strcpy(filename1, "shakespeare.txt");

}
