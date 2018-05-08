#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using namespace std;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);//6000

    /* Write the name of the file that will be mapped */
    strcpy(filename, "shakespeare.txt");//6000
		cout << filename << endl;
    /* Map a page from the specified file */
    char *p = (char *) vm_map (filename, 0);//60001

    /* Print the first speech from the file */
    for (unsigned int i=0; i<2561; i++) {
			p[i] = 'A';
      cout << p[i];
      
    }

    char *filename1 = (char *) vm_map(nullptr, 0);//60002
    /* Write the name of the file that will be mapped */
    strcpy(filename1, "shakespeare.txt");
    
    char *filename2 = (char *) vm_map(nullptr, 0);//60002
    /* Write the name of the file that will be mapped */
    strcpy(filename2, "shakespeare.txt");
    
    char *filename3 = (char *) vm_map(nullptr, 0);//60002
    /* Write the name of the file that will be mapped */
    strcpy(filename3, "shakespeare.txt");
    
    char *filename4 = (char *) vm_map(nullptr, 0);//60002
    /* Write the name of the file that will be mapped */
    strcpy(filename4, "shakespeare.txt");
    
    char *filename5 = (char *) vm_map(nullptr, 0);//60002
    /* Write the name of the file that will be mapped */
    strcpy(filename5, "shakespeare.txt");
    
    char *filename6 = (char *) vm_map(nullptr, 0);//60002
    /* Write the name of the file that will be mapped */
    strcpy(filename6, "shakespeare.txt");
    
    cout << filename1 << endl;
    
    /* Map a page from the specified file */
    char *p1 = (char *) vm_map (filename1, 0);
    for (unsigned int i=0; i<2561; i++) {
      p1[i] = 'B';
			cout << p1[i];
    }


}
