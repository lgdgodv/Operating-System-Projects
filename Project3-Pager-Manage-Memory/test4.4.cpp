#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
	cout << "WU" << endl;
	/* Allocate swap-backed page from the arena */
	char *filename = (char *) vm_map(nullptr, 0);
	/* Write the name of the file that will be mapped */
	strcpy(filename, "shakespeare.txt");
	cout << filename << endl;
	cout << "ting" << endl;
	/* Map a page from the specified file */
	char *p = (char *) vm_map (filename, 0);
	cout << "headsome" << endl;
	/* Print the first speech from the file */
	for (unsigned int i=0; i<2561; i++) {
		cout << p[i];
	}

	char *filename1 = (char *) vm_map(nullptr, 0);
	/* Write the name of the file that will be mapped */
	cout << "here" << endl;
	strcpy(filename1, "shakespeare.txt");
	/* Map a page from the specified file */
	char *p1 = (char *) vm_map (filename1, 0);
	cout << "not here\n";
	for (unsigned int i=0; i<2561; i++) {
		cout << p1[i];
	}


	char *filename2 = (char *) vm_map(nullptr, 0);
	/* Write the name of the file that will be mapped */
	cout << "here" << endl;
	strcpy(filename2, "shakespeare.txt");
	/* Map a page from the specified file */
	char *p2 = (char *) vm_map (filename2, 0);
	cout << "not here\n";
	for (unsigned int i=0; i<2561; i++) {
		cout << p2[i];
	}



	char *filename3 = (char *) vm_map(nullptr, 0);
	/* Write the name of the file that will be mapped */
	cout << "here" << endl;
	strcpy(filename3, "shakespeare.txt");
	/* Map a page from the specified file */
	char *p3 = (char *) vm_map (filename3, 0);
	cout << "end\n";
	for (unsigned int i=0; i<2561; i++) {
		cout << p3[i];
	}
    
    
    char *filename4 = (char *) vm_map(nullptr, 0);
    /* Write the name of the file that will be mapped */
    cout << "here" << endl;
    strcpy(filename4, "shakespeare.txt");
    /* Map a page from the specified file */
    char *p4 = (char *) vm_map (filename4, 0);
    cout << "end\n";
    for (unsigned int i=0; i<2561; i++) {
        cout << p4[i];
    }
    
    
    cout << "read old swap file";
    strcpy(filename, "shakespeare.txt");
    char *p5 = (char *) vm_map (filename, 0);
    cout << "end\n";
    for (unsigned int i=0; i<2561; i++)
    {
        cout << p5[i];
    }

    return 0;
}







