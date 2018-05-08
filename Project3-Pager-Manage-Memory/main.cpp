#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
	char *filename = (char *) vm_map(nullptr, 0);
	strcpy(filename, "shakespeare.txt");
	char *p = (char *) vm_map (filename, 0);
	for (unsigned int i=0; i<20; i++) {
		cout << p[i];
	}
    cout << "\n end of first\n";

	char *filename1 = (char *) vm_map(nullptr, 0);
	strcpy(filename1, "shakespeare.txt");
	char *p1 = (char *) vm_map (filename1, 0);
	for (unsigned int i=0; i<20; i++) {
		cout << p1[i];
	}
    cout << "\nend of second";

	char *filename2 = (char *) vm_map(nullptr, 0);
	strcpy(filename2, "shakespeare.txt");
	char *p2 = (char *) vm_map (filename2, 0);
	for (unsigned int i=0; i<20; i++) {
		cout << p2[i];
	}
    cout << "\nend of third\n";


	char *filename3 = (char *) vm_map(nullptr, 0);
	strcpy(filename3, "shakespeare.txt");
	char *p3 = (char *) vm_map (filename3, 0);
	for (unsigned int i=0; i<20; i++) {
		cout << p3[i];
	}
    cout << "\nend of fourth\n";
    
    
    char *filename4 = (char *) vm_map(nullptr, 0);
    strcpy(filename4, "shakespeare.txt");
    char *p4 = (char *) vm_map (filename4, 0);
    for (unsigned int i=0; i<20; i++) {
        cout << p4[i];
    }
    cout << "\nend of fifth\n";
    
    
    cout << "read old swap file";
    strcpy(filename, "shakespeare.txt");
    char *p5 = (char *) vm_map (filename, 0);
    for (unsigned int i=0; i<20; i++)
    {
        cout << p5[i];
    }
    cout << "\nend of sixth\n";
    
    for (unsigned int i=0; i<20; i++)
    {
        cout << p2[i];
    }
    cout << "\nthe end\n";

    return 0;
}






