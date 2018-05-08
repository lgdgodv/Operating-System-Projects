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

    char *p = (char *) vm_map (filename, 0);
		char *p1 = (char *) vm_map (filename, 0);
		char *p2 = (char *) vm_map (filename, 0);
		char *p3 = (char *) vm_map (filename, 0);
		char *p4 = (char *) vm_map (filename, 0);


		for (unsigned int i=0; i<3; i++) {
			cout << p[i];
		}

		for (unsigned int i=0; i<3; i++) {
			cout << p1[i];
		}

		for (unsigned int i=0; i<3; i++) {
			cout << p2[i];
		}

		for (unsigned int i=0; i<3; i++) {
			cout << p3[i];
		}

		for (unsigned int i=0; i<3; i++) {
			cout << p4[i];
		}



		for (unsigned int i=0; i<3; i++) {
			cout << p1[i];
		}

		for (unsigned int i=0; i<3; i++) {
			cout << p2[i];
		}

		char *p5 = (char *) vm_map (filename, 0);
	
		for (unsigned int i=0; i<3; i++) {
			cout << p5[i];
		}


}
