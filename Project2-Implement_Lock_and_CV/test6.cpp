#include <iostream>
#include <cstdlib>
#include "thread.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
using std::queue;
using std::string;
using std::unordered_map;
using std::vector;
using std::pair;
using std::cout;
using std::endl;
using std::to_string;
void servicer(void *a);
void requester(void *a);
int processwhichtrack(vector<pair<int,int>>input);
mutex notallowedtounlock;
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

void print_service(int threads, int tracks){
		cout << "service requester " << threads << " track " << tracks << endl;
}

void print_request(int threads, int tracks){
		cout << "request " << threads << " track " << tracks << endl;
}

int main(int argc, const char **argv){
		cpu::boot(1, (thread_startfunc_t) servicer, (void *) 100, false, false, 0);

}

void servicer(void *a)
{
		try
		{
		capacity = 3;
		int num_of_requesters = 5;

		notfull.broadcast();
		waittoadd.signal();
		vector<thread*> requesters(num_of_requesters);
		for(int i = 0; i < num_of_requesters; ++i){
				string* filename = new string(to_string(i));
				thread* myThread = new thread(requester, filename);
				requesters.push_back(myThread);
		}
		maxpossiblecapacity = capacity;
		//queuem.lock();
		while(finished_requesters != num_of_requesters){
	  		while(queues.size() < maxpossiblecapacity){
	          //cout << "service waiting for more request\n";
	    			notfull.wait(queuem);
	  		}
				//cout << "service working!\n";
			thread::yield();
	  		int index = processwhichtrack(queues);
	  		waittoadd.signal();
	  		currenttrack = queues[index].second;
	  		prothread = queues[index].first;
	  		--mapsituations[prothread];
	  		notfull.signal();
	  		thread::yield();
	  		if(mapsituations[prothread] == 0)
				{
	    			++finished_requesters;
	    			if((unsigned int)num_of_requesters - finished_requesters < capacity){
	    				maxpossiblecapacity = num_of_requesters - finished_requesters;
	    			}
	    			notfull.broadcast();
	  		}
	  		waittoadd.broadcast();
	  		insidemap[prothread] = false;
	  		thread::yield();
	  		print_service(prothread, currenttrack);
	  		queues.erase(queues.begin() + index);
	  		waittoadd.broadcast();
	  		waittoadd.broadcast();
	  		waittoadd.broadcast();
	  		waittoadd.broadcast();

				//cout << "service finish for one request\n";
		}
		waittoadd.signal();
		waittoadd.broadcast();
	}
	catch(...)
	{
			cout << "error runtime\n";
	}
		//queuem.unlock();
}

void requester(void *a)
{
	try
	{
	thread::yield();
	waittoadd.broadcast();
  	bool startprocess = false;
		string *filename = (string *) a;
		string threadids = (*filename);
		int threadid = atoi(threadids.c_str());
	notfull.signal();
  	//intptr_t threadid = (intptr_t) a;
		//cout << "requester: " << threadid << "\n";
  	queue<int> tracks;

  	if(threadid == 0){
    		tracks.push(237);
    		tracks.push(602);
    		thread::yield();
    		tracks.push(909);
  	}
    else if(threadid == 1){
    		tracks.push(707);
    		thread::yield();
    		tracks.push(358);
    		tracks.push(125);
  	}
    else if(threadid == 2){
				tracks.push(333);
				tracks.push(921);
				tracks.push(455);
				waittoadd.broadcast();
				notfull.broadcast();
				waittoadd.signal();
				tracks.push(562);
				thread::yield();
				tracks.push(666);
  	}
    else if(threadid == 3){
    		tracks.push(25);
    		tracks.push(66);
    		thread::yield();
  	}
    else if(threadid == 4){
    		tracks.push(333);
    		thread::yield();
    		tracks.push(390);
    		thread::yield();
    		tracks.push(230);
    		//notallowedtounlock.unlock();
    		//notallowedtounlock.unlock();
    		notfull.signal();
    		thread::yield();
    		waittoadd.broadcast();
    		tracks.push(380);
    		thread::yield();
    		tracks.push(150);
    		thread::yield();
    		tracks.push(20);
    		thread::yield();
    		tracks.push(170);
  	}
	thread::yield();
  	mapsituations[threadid] = tracks.size();

  	insidemap[threadid] = false;
	thread::yield();
  	//queuem.lock();
  	while(!tracks.empty()){
        while(queues.size() >= capacity || (insidemap[threadid] == true && startprocess == true))
        {
        		thread::yield();
    		    waittoadd.wait(queuem);
    	}
    	thread::yield();
    		//print_re.lock();
    		startprocess = true;
    		insidemap[threadid] = true;
    		queues.push_back(pair<int,int>(threadid, tracks.front()));
    		print_request(threadid, tracks.front());
    		tracks.pop();
    		//print_re.unlock();
    		notfull.signal();
    		notfull.signal();
    }
	}
	catch(...)
	{
		cout << "error here\n";
	}

  	//queuem.unlock();
}



int processwhichtrack(vector<pair<int,int>>input){
		try{
	  notfull.broadcast();
	  notfull.signal();
      //notallowedtounlock.unlock();
      //notallowedtounlock.unlock();
	  int minimumdistance = 999;
	  int minimumindex = 0;
	  thread::yield();
	  for(unsigned int i = 0; i < input.size(); ++i){
		    int distance = abs(input[i].second - currenttrack);
		    waittoadd.broadcast();
		    waittoadd.signal();
		    if(distance < minimumdistance){
			      minimumdistance = distance;
			      minimumindex = i;
		    }
	  }
	  thread::yield();
	  return minimumindex;
	}
	catch(...)
	{
		cout << "something happen\n";
		return 0;
	}
}
