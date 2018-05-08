#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "shakespeare.txt");
		cout << filename << endl;
    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
			cout << p[i];
    }

    char *filename1 = (char *) vm_map(nullptr, 0);
    /* Write the name of the file that will be mapped */
    strcpy(filename1, "shakespeare.txt");
    /* Map a page from the specified file */
    char *p1 = (char *) vm_map (filename1, 0);
    char *filename2 = (char *) vm_map(nullptr, 0);
    char *filename3 = (char *) vm_map(nullptr, 0);
    for (unsigned int i=0; i<2561; i++) {
			cout << p1[i];
    }


}
