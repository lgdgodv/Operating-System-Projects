#include "thread.h"
#include <iostream>
#include "disk.h"
#include <vector>
#include <fstream>
#include <queue>
#include <stdlib.h>
#include <unordered_map>
using namespace std;


void servicer(void *a);
void requester(void *a);
int processwhichtrack(vector<pair<int,int>>input);

mutex print_re;
mutex print_se;
mutex queuem;
cv notfull;
cv waittoadd;
unsigned int capacity;
unsigned int maxpossiblecapacity;
int num_of_requesters;

vector<pair<int,int>>queues;
unordered_map<int, int>mapsituations;
unordered_map<int, bool>insidemap;
int prothread = -1;
int currenttrack = 0;
int tmp = 0;
int finished_requesters = 0;

struct arguments{
	int argc;
	const char **argv;
};


int main(int argc, const char **argv){
	arguments args = {argc, argv};
	cpu::boot(servicer, &args, 0);
}



void servicer(void *a)
{
	arguments *args = (arguments *) a;
	capacity = atoi(args->argv[1]);
	num_of_requesters = (args->argc) - 2;
	
	
	vector<thread*> requesters(num_of_requesters);
	for(int i = 0; i < num_of_requesters; ++i){
		string* filename = new string((args->argv[2+i]));
		thread* myThread = new thread(requester, filename);
		requesters.push_back(myThread);  
	}
	maxpossiblecapacity = capacity;
	
	queuem.lock();
	while(finished_requesters != num_of_requesters){
		while(queues.size() < maxpossiblecapacity){	
			notfull.wait(queuem);
		}
		int index = processwhichtrack(queues);
		currenttrack = queues[index].second;
		prothread = queues[index].first;
		--mapsituations[prothread];
		if(mapsituations[prothread] == 0){
			++finished_requesters;
			if((unsigned int)num_of_requesters - finished_requesters < capacity){
				maxpossiblecapacity = num_of_requesters - finished_requesters;
			}
		}
		insidemap[prothread] = false;
		print_service(prothread, currenttrack);
		queues.erase(queues.begin() + index);
		waittoadd.broadcast();
	}
	queuem.unlock();
}

void requester(void *a){
	bool startprocess = false;
	
	string *filename = (string *) a;
	string threadids = (*filename).substr(7, (*filename).length()-7);
	int threadid = atoi(threadids.c_str());
	
	queue<int> tracks;
	
	ifstream fin;
	string ti;
	fin.open(*(filename));
	while(getline(fin,ti)){
		if(fin.eof()){
			break;
		}
		tracks.push(atoi(ti.c_str()));
	}
	
	mapsituations[threadid] = tracks.size();
	insidemap[threadid] = false;
	
	queuem.lock();
	while(!tracks.empty()){
		while(queues.size() >= capacity || (insidemap[threadid] == true && startprocess == true)){
			waittoadd.wait(queuem);			
		}
		print_re.lock();
		startprocess = true;
		insidemap[threadid] = true;
		queues.push_back(pair<int,int>(threadid, tracks.front()));
		print_request(threadid, tracks.front());
		tracks.pop();
		print_re.unlock();
		notfull.signal();
	}
	
	queuem.unlock();
}



int processwhichtrack(vector<pair<int,int>>input){
	int minimumdistance = 999;
	int minimumindex = 0;	
	for(unsigned int i = 0; i < input.size(); ++i){
		int distance = abs(input[i].second - currenttrack);
		if(distance < minimumdistance){
			minimumdistance = distance;
			minimumindex = i;
		}
	}
	return minimumindex;
}





