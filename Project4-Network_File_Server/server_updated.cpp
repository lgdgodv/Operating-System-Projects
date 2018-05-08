#include <iostream>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <unordered_map>
#include "fs_server.h"
#include <vector>
#include <cstring>
#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>        // gethostbyname(), struct hostent
#include <netinet/in.h>    // struct sockaddr_in
using namespace std;

vector<bool> free_block_list(FS_DISKSIZE, true);
void travers_tree_direntry(uint32_t block);
void travers_tree_inode(fs_inode* root);

unordered_map<string, string> user_list;
int sock;
struct sockaddr_in addr;
int rval;
int msgsock;
long long next_session = 0;
unordered_map<unsigned int, string> session_to_username;
unordered_map<unsigned int, unsigned int> session_to_sequence;

unordered_map<int, pthread_rwlock_t*> lock_map;


mutex mutex1;

// get one free_block from free_block_list data structure
int get_free_block()
{
    for(int i = 0; i < (int)free_block_list.size();i++)
    {
        if(free_block_list[i])
        {
            free_block_list[i] = false;
            return i;
        }
    }
    return -1;
}

// traverse inode to read the existing file server
void travers_tree_inode(fs_inode* root)
{
    for(int i = 0; i < (int)root->size; i++)
    {
        free_block_list[root->blocks[i]] = false;
        // call traverse_direntry helper to go through directory pointed by current inode
        if(root->type == 'd')
        {
            travers_tree_direntry(root->blocks[i]);
        }
    }
}

// under each directory inode, traverse the fs_directry to update free_block
void travers_tree_direntry(uint32_t block)
{
    fs_direntry buf[FS_DIRENTRIES];
    disk_readblock((unsigned int) block, (void *)buf);
    for(int i = 0; i < (int)FS_DIRENTRIES; i++)
    {
        if((int)buf[i].inode_block != 0)
        {
            fs_inode node;
            disk_readblock((unsigned int) buf[i].inode_block, &node);
            // init rwlock
            pthread_rwlock_t* rwlock = new pthread_rwlock_t;
            pthread_rwlock_init(rwlock, NULL);
            lock_map[buf[i].inode_block] = rwlock;
            free_block_list[buf[i].inode_block] = false;
            travers_tree_inode(&node);
        }
    }
}

// given a string input pathway, output a vector of all filename/directory inside pathway
int seperate_route(string string_in, vector<string> &vector_in)
{
    string tmp = "";
    //more spaces than needed count as error
    if((int)string_in.length() == 0 || string_in[0] != '/')
    {
        //error return 1
        return 1;
    }
    for(int i = 1; i < (int)string_in.length(); ++i)
    {
        // continue getting filename until hit /
        if(string_in[i] != '/')
        {
            tmp += string_in[i];
        }
        // stop getting currnet filename, error checking of length of filename
        else
        {
            if(tmp != "" && tmp.length() <= FS_MAXFILENAME)
            {
                vector_in.push_back(tmp);
            }
            // filename too long
            else
            {
                return 1;
            }
            tmp = "";
        }
    }
    // filename/directory name is non-empty or within size limit
    if(tmp != "" && tmp.length() <= FS_MAXFILENAME)
    {
        vector_in.push_back(tmp);
    }
    else
    {
        return 1;
    }
    return 0;
}

// BFS HELPER to find the inode that we want to get
// indicator = 1, find parent inode of current pathway, indicator = 0, find until the last item in pathway
// need test indicate whether we need to check pathway belongs to current input username
// will return with the lock error will unlock
int BFS_helper(int block, vector<string> &route, int indicator, string username, bool need_test, bool need_write)
{
    int old_block = block;
    if((int)route.size() == indicator)
    {
        return block;
    }
    else
    {
        fs_inode root;
        disk_readblock(block, &root);
        if(need_test && block != 0)
        {
            if(string(root.owner) != username)
            {
                // release the lock before error handling

                pthread_rwlock_t* temp_lock;
                mutex1.lock();
                temp_lock = lock_map[old_block];
                mutex1.unlock();

                pthread_rwlock_unlock(temp_lock);
                return -1;
            }
            need_test = false;
        }
        string target = route[0];

        // to proceed, must be a directory type, since the following are going to next layer
        if(root.type != 'd')
        {
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[old_block];
            mutex1.unlock();
            pthread_rwlock_unlock(temp_lock);
            return -1;
        }

        for(int i = 0; i < (int)root.size; i++)
        {
            fs_direntry buf[FS_DIRENTRIES];
            disk_readblock(root.blocks[i], (void *)buf);
            // go through every direntry pointed by this inode
            for(int j = 0; j < (int)FS_DIRENTRIES; j++)
            {
                if((int)buf[j].inode_block != 0)
                {
                    if(string(buf[j].name) == target)
                    {
                        block = buf[j].inode_block;
                        route.erase(route.begin());
                        // grab the write lock for next stage
                        if(need_write && (int)route.size() == indicator)
                        {
                            pthread_rwlock_t* temp_lock;
                            mutex1.lock();
                            temp_lock = lock_map[block];
                            mutex1.unlock();
                            pthread_rwlock_wrlock(temp_lock);
                        }
                        // grab reader lock to traverse to next layer
                        else
                        {
                            pthread_rwlock_t* temp_lock;
                            mutex1.lock();
                            temp_lock = lock_map[block];
                            mutex1.unlock();
                            pthread_rwlock_rdlock(temp_lock);
                        }
                        pthread_rwlock_t* temp_lock;
                        mutex1.lock();
                        temp_lock = lock_map[old_block];
                        mutex1.unlock();

                        pthread_rwlock_unlock(temp_lock);
                        return BFS_helper(block,route, indicator, username, need_test, need_write);
                    }
                }
            }
        }
    }
    //error
    // unlock reader lock before return

    pthread_rwlock_t* temp_lock;
    mutex1.lock();
    temp_lock = lock_map[old_block];
    mutex1.unlock();

    pthread_rwlock_unlock(temp_lock);
    return -1;
}

// BFS to search for inode needed
int BFS(vector<string> &route, int indicator, string username, bool need_write)
{
    if(need_write && (int)route.size() == indicator)
    {

        pthread_rwlock_t* temp_lock;
        mutex1.lock();
        temp_lock = lock_map[0];
        mutex1.unlock();

        pthread_rwlock_wrlock(temp_lock);
    }
    else
    {
        pthread_rwlock_t* temp_lock;
        mutex1.lock();
        temp_lock = lock_map[0];
        mutex1.unlock();

        pthread_rwlock_rdlock(temp_lock);
    }
    return BFS_helper(0,route,indicator,username,true,need_write);
}

// find empty spots in current fs_direntry array
int find_empty_spot(fs_direntry buf_in[])
{
    for(int i = 0; i < (int)FS_DIRENTRIES; ++i)
    {
        if(buf_in[i].inode_block == 0)
        {
            return i;
        }
    }
    return -1;
}

// fs_create helper
int create(int block_in, string name_in, char type_in, string user_in)
{
    //pthread_rwlock_wrlock(lock_map[block_in]);
    fs_inode parent_inode;
    disk_readblock(block_in, &parent_inode);

    // checking whether parent node is directory type, throw error when not
    if(parent_inode.type != 'd' || (string(parent_inode.owner) != user_in && block_in != 0))
    {
        pthread_rwlock_t* temp_lock;
        mutex1.lock();
        temp_lock = lock_map[block_in];
        mutex1.unlock();

        pthread_rwlock_unlock(temp_lock);
        return false;
    }

    // lock before access free_block_list
    mutex1.lock();
    int new_block = get_free_block();
    mutex1.unlock();
    // no free block available
    if(new_block == -1)
    {
        pthread_rwlock_t* temp_lock;
        mutex1.lock();
        temp_lock = lock_map[block_in];
        mutex1.unlock();

        pthread_rwlock_unlock(temp_lock);
        return false;
    }

    fs_direntry tmp_direntry_array[FS_DIRENTRIES];
    fs_direntry tmp_used[FS_DIRENTRIES];
    // empty is the inode that will be created
    fs_inode empty;
    empty.size = 0;
    empty.type = type_in;
    strcpy(empty.owner, user_in.c_str());
    unsigned int direntry_block = 0;
    bool find = false;

    //if parent inode has block for storing direntries
    if(parent_inode.size != 0)
    {
        int direntry_index = -1;
        // go through every block stroing direntries to see whether there are spots available
        for(int i = 0; i < (int)parent_inode.size; ++i)
        {
            disk_readblock(parent_inode.blocks[i], tmp_used);
            for(int j = 0; j < (int)FS_DIRENTRIES; ++j)
            {
                // check duplicate filename/directory name, loop from the beginning to the ending
                if(tmp_used[j].inode_block != 0 && tmp_used[j].name == name_in)
                {
                    mutex1.lock();
                    free_block_list[new_block] = true;
                    mutex1.unlock();

                    pthread_rwlock_t* temp_lock;
                    mutex1.lock();
                    temp_lock = lock_map[block_in];
                    mutex1.unlock();

                    pthread_rwlock_unlock(temp_lock);
                    return false;
                }
                // find first emtpy spot, take this spot and continue to see if there exist duplicate filename
                if(find == false && tmp_used[j].inode_block == 0)
                {
                    find = true;
                    for(int k = 0; k < (int)FS_DIRENTRIES; ++k)
                    {
                        strcpy(tmp_direntry_array[k].name, tmp_used[k].name);
                        tmp_direntry_array[k].inode_block = tmp_used[k].inode_block;
                    }
                    direntry_index = j;
                    direntry_block = parent_inode.blocks[i];
                }
            }
        }
        // find empty spot in fs_direntry and no duplicate filename/directory name
        if(find == true)
        {
            // add new lock to lock map
            pthread_rwlock_t* rwlock = new pthread_rwlock_t;                                                                                    //may lock problem
            pthread_rwlock_init(rwlock, NULL);
            // lock to protect accessing lock_map
            mutex1.lock();
            lock_map[new_block] = rwlock;
            mutex1.unlock();
            // get write lock for creating this new child
            pthread_rwlock_wrlock(rwlock);
            disk_writeblock(new_block, &empty);
            // unlock child lock
            pthread_rwlock_unlock(rwlock);
            tmp_direntry_array[direntry_index].inode_block = new_block;
            // set the direntry name to new name specified by user
            strcpy(tmp_direntry_array[direntry_index].name, name_in.c_str());
            disk_writeblock(direntry_block, &tmp_direntry_array);
            // unlock the parent fs_direntry block, need to hold the lock until child inode has been append
            // create need to update child, then parent

            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[block_in];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            return true;
        }
    }

    // outside if: no available spot found in available fs_direntry list
    // either the fs_direntry list is full or the inode does not have fs_direntry list
    // need to create new one at the end of inode block list

    // no space for parent inode block
    if(parent_inode.size >= (int)FS_MAXFILEBLOCKS)
    {
        mutex1.lock();
        free_block_list[new_block] = true;
        mutex1.unlock();

        pthread_rwlock_t* temp_lock;
        mutex1.lock();
        temp_lock = lock_map[block_in];
        mutex1.unlock();

        pthread_rwlock_unlock(temp_lock);
        return false;
    }

    //doesn't have free places to store, or parent inode has blocks for storing, but all of them are full
    mutex1.lock();
    int free_block_indx = get_free_block();
    mutex1.unlock();
    // no more free blocks, release the previous taken free block
    if(free_block_indx == -1)
    {
        mutex1.lock();
        free_block_list[new_block] = true;
        mutex1.unlock();

        pthread_rwlock_t* temp_lock;
        mutex1.lock();
        temp_lock = lock_map[block_in];
        mutex1.unlock();

        pthread_rwlock_unlock(temp_lock);
        return false;
    }

    // create new rwlock for new block, create child first and update parent next
    pthread_rwlock_t* rwlock = new pthread_rwlock_t;
    pthread_rwlock_init(rwlock, NULL);
    // protect access to lock map
    mutex1.lock();
    lock_map[new_block] = rwlock;
    mutex1.unlock();
    // protect writing to disk
    pthread_rwlock_wrlock(rwlock);
    disk_writeblock(new_block, &empty);
    pthread_rwlock_unlock(rwlock);

    // create new fs_direntry list, initialize to empty
    fs_direntry buf[FS_DIRENTRIES];
    for(int i = 0; i < (int)FS_DIRENTRIES; i++)
    {
        strcpy(buf[i].name, "");
        buf[i].inode_block = 0;
    }
    // set the first fs_direntry to be newly created one
    strcpy(buf[0].name, name_in.c_str());
    buf[0].inode_block = new_block;
    disk_writeblock(free_block_indx, &buf);
    parent_inode.blocks[parent_inode.size] = free_block_indx;
    parent_inode.size += 1;
    disk_writeblock(block_in, &parent_inode);
    // unlock before return

    pthread_rwlock_t* temp_lock;
    mutex1.lock();
    temp_lock = lock_map[block_in];
    mutex1.unlock();

    pthread_rwlock_unlock(temp_lock);
    return true;
}

// called as thread input function
void request_handler(int msgsock)
{
    char buf_one;
    string clear_input;
    if (msgsock == -1)
    {
        exit(EXIT_FAILURE);
    }
    //receive cleartext header, only read one bit of input
    rval = recv(msgsock, static_cast<void *>(&buf_one), 1, MSG_WAITALL);
    int cleartext_len = 1;
    bool fail = false;

    // continue reading unilt meet \0 or cleartext input > 22 bit
    // input length: 10 (username) + space + 10 (max unsigned size) + \0
    while(buf_one != '\0')
    {
        clear_input = clear_input + buf_one;
        rval = recv(msgsock, static_cast<void *>(&buf_one), 1, MSG_WAITALL);
        cleartext_len++;
        if(cleartext_len > (int)FS_MAXUSERNAME + 12)
        {
            fail = true;
            break;
        }
    }
    // cleartext too long
    if(fail)
    {
        close(msgsock);
        return;
    }
    char user[FS_MAXUSERNAME + 1]  =  {0};
    unsigned int len;
    // check if cleartext contain username, size
    if(sscanf(clear_input.c_str(),"%10s %u",user, &len) != 2)
    {
        close(msgsock);
        return;
    }
    // reconstruct the original string and compare with result buffer
    string trash;
    trash = string(user) + " " + to_string(len);
    // read-in buffer does not match || username not found
    if((trash != clear_input) || (user_list.find(string(user)) == user_list.end()))
    {
        close(msgsock);
        return;
    }

    //receive real request and parse
    char* tmp_buf = new char[len];
    char* result_buf;
    unsigned int request_len;
    rval = recv(msgsock, static_cast<void *>(tmp_buf), len, MSG_WAITALL);
    result_buf = static_cast<char *>(fs_decrypt((user_list[user]).c_str(), tmp_buf, len, &request_len));
    delete[] tmp_buf;
    // fail to decrypt
    if(!result_buf)
    {
        close(msgsock);
        return;
    }
    // get request type, if no request type, close connection
    char request_type[15] =  {0};
    if(sscanf(result_buf,"%14s",request_type) != 1)
    {
    	delete[] result_buf;
        close(msgsock);
        return;
    }
    //needed to send information back
    unsigned int return_size;
    string return_string;
    char* return_string_buf;
    string clear_header;
    unsigned int session = 0;
    unsigned int sequence = 0;
    // deal with request that is not fs_session, check if username exist, session is correct
    // and check whether sequence is valid
    if(strcmp (request_type, "FS_SESSION") != 0)
    {
        // request other than fs_session must follow this format
        if(sscanf(result_buf,"%14s %u %u", request_type, &session, &sequence) != 3)
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }
        mutex1.lock();
        // session does not exist || username session does not match || sequence is not increasing
        if(session_to_username.find(session) == session_to_username.end() || session_to_username[session] != user 
            || sequence <= session_to_sequence[session])
        {
            mutex1.unlock();
            delete[] result_buf;
            close(msgsock);
            return;
        }
        // only seemingly valid request wiil take sequence
        session_to_sequence[session] = sequence;
        mutex1.unlock();
    }

    char data[FS_BLOCKSIZE + 1] =  {0};

    if(strcmp (request_type, "FS_SESSION") == 0)
    {
        // if input format does not follow: FS_SESSION <session> <sequence><NULL>
        if(sscanf(result_buf,"%14s %u %u",request_type, &session, &sequence) != 3)
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }
        // reconstruct the original string
        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence);

        if(trash != result_buf)
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }
        
        delete[] result_buf;
        mutex1.lock();
        // not enough session available
        if(next_session > std::numeric_limits<unsigned int>::max())
        {
            mutex1.unlock();
            close(msgsock);
            return;
        }
        // link the new session to this username
        session = next_session;
        ++next_session;
        session_to_sequence[session] = sequence;
        session_to_username[session] = user;
        mutex1.unlock();
    }
    else if(strcmp (request_type, "FS_READBLOCK") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1] =  {0};
        unsigned int block;

        // if input format does not follow: FS_READBLOCK <session> <sequence> <pathname> <block><NULL>
        if(sscanf(result_buf,"%14s %u %u %128s %u",request_type, &session, &sequence, pathname, &block) != 5)
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }

        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname + " " + to_string(block);

        // pattern does not match || block too lagre || pathname too long
        if((trash != result_buf) || (block >= (int)FS_MAXFILEBLOCKS) || (string(pathname).length() > FS_MAXPATHNAME))
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }

		delete[] result_buf;
        vector<string> route;
        // if separate_route return 1, means that pathway is not valid
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }
        
        int q = BFS(route, 0,user,false);
        // pathname does not exist
        if(q == -1)
        {
            close(msgsock);
            return;
        }

        //pthread_rwlock_rdlock(lock_map[q]);
        fs_inode target_node;
        disk_readblock(q, &target_node);

        if(string(target_node.owner) != user)        
        {
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            close(msgsock);
            return;
        }

        if(target_node.type == 'f' && target_node.size > block)
        {
            disk_readblock(target_node.blocks[block],data);

            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
        }
        else
        {
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            close(msgsock);
            return;
        }
    }
    else if(strcmp (request_type, "FS_WRITEBLOCK") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1] =  {0};
        unsigned int block;
        // if input format does not follow: FS_WRITEBLOCK <session> <sequence> <pathname> <block><NULL><data>
        if(sscanf(result_buf,"%14s %u %u %128s %u",request_type, &session, &sequence, pathname, &block) != 5)
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }

        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname + " " + to_string(block);

        // pattern does not match || block too lagre || pathname too long
        if((trash != result_buf) || (string(pathname).length() > FS_MAXPATHNAME) || (block >= (int)FS_MAXFILEBLOCKS))
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }

        int data_idx = 0;
        int start_place;
        int character_count = 0;

        // loop until the start place of input data
        for(int i = 0; i < (int)request_len; i++)
        {
            if(result_buf[i] == '\0')
            {
                start_place = i;
                break;
            }
        }

        // junyue!!!!!
        // first: we need to know the size of input data, if data is too long or too short, close connection
        if(request_len - start_place - 1 != FS_BLOCKSIZE)
        {   
        	delete[] result_buf;
            //cout << "not valid" << endl;
            close(msgsock);
            return;
        }
        // if data size is exactly one block, then read the data into our data char array
        for(int i = start_place + 1; i < (int)request_len; ++i)
        {
            data[data_idx] = result_buf[i];
            data_idx++;
            ++character_count;
        }
        
        delete[] result_buf;

        vector<string> route;
        // invalid pathname
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }

        // find the last layer inode
        int q = BFS(route, 0,user,true);
        // pathname does not exist
        if(q == -1)
        {
            close(msgsock);
            return;
        }
        fs_inode target_node;
        disk_readblock(q, &target_node);

        if(string(target_node.owner) != user)        
        {
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            close(msgsock);
            return;
        }

        // error checking: can only write to file and block must be smaller than current size
        // or immediately after current last block
        if(target_node.type == 'f' && target_node.size >= block)                    //maybe?
        {
            bool need_update = false;
            // new block needed to be allocated for current filename
            if(target_node.size == block)
            {
                mutex1.lock();
                int good_block = get_free_block();
                mutex1.unlock();
                // no free block
                if(good_block == -1)
                {
                    pthread_rwlock_t* temp_lock;
                    mutex1.lock();
                    temp_lock = lock_map[q];
                    mutex1.unlock();

                    pthread_rwlock_unlock(temp_lock);
                    close(msgsock);
                    return;
                }
                // no need to check whether size fit, since offset indicate whether overflow
                // offset = maxfileblock will be ok, if offset > maxfileblock, close connection ahead
                target_node.size++;
                target_node.blocks[block] = good_block;
                need_update = true;
            }
            // write data to block specified by user
            disk_writeblock(target_node.blocks[block],data);
            // update current filename inode by appending new block at the end
            if(need_update)
            {
                disk_writeblock(q,&target_node);
            }
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
        }
        // invalid offset or try to write to direntry
        else
        {
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            close(msgsock);
            return;
        }
    }
    else if(strcmp(request_type, "FS_CREATE") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1] =  {0};
        char type;

        // check if format match: FS_CREATE <session> <sequence> <pathname> <type><NULL>
        if(sscanf(result_buf,"%14s %u %u %128s %c",request_type, &session, &sequence, pathname,&type) != 5)
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }

        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname + " " + type;
        // pattern does not match || pathname too long || create type not file or directory
        if((trash != result_buf) || (string(pathname).length() > FS_MAXPATHNAME) || (type != 'd' && type != 'f'))
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }
        delete[] result_buf;
        vector<string> route;
        // invalid pathname
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }
        // find parent inode block
        int q = BFS(route, 1,user,true);
        // pathname does not exist
        if(q == -1)
        {
            close(msgsock);
            return;
        }

        int z = create(q, route[route.size()-1], type, string(user));           //pay attention
        // z = 0, error happened during fs_create
        if(z == 0)                                                              
        {
            close(msgsock);
            return;
        }
    }
    else if(strcmp(request_type, "FS_DELETE") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1] =  {0};

        // check if format match: FS_DELETE <session> <sequence> <pathname><NULL>
        if(sscanf(result_buf,"%14s %u %u %128s",request_type, &session, &sequence, pathname) != 4)
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }
        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname;
        // error checking: throw trash message
        if((trash != result_buf) || (string(pathname).length() > FS_MAXPATHNAME)) 
        {
        	delete[] result_buf;
            close(msgsock);
            return;
        }
        delete[] result_buf;
        vector<string> route;
        // return 1: invalid pathname
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }
        // get parent inode
        int q = BFS(route, 1, user, true);
        // pathname does not exist
        if(q == -1)
        {
            close(msgsock);
            return;
        }

        //pthread_rwlock_wrlock(lock_map[q]);

        fs_inode parent_node;
        fs_inode block_be_deleted;
        disk_readblock(q, &parent_node);

        // parent inode is a file type or permission not correct for non-root pathname
        if((strcmp(parent_node.owner, user) != 0 && q != 0) || (parent_node.type == 'f'))
        {
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            close(msgsock);
            return;
        }

        //target last part of filename
        string target = route[route.size()-1];
        bool find = false;
        fs_direntry tmp_direntry_array[FS_DIRENTRIES];
        int direntry_block = 0;
        int target_index = -1;
        int target_block = 0;

        // find the target file/directory that we are looking for
        for(int i = 0; i < (int)parent_node.size; i++)
        {
            disk_readblock(parent_node.blocks[i], &tmp_direntry_array);
            for(int j = 0; j < (int)FS_DIRENTRIES; j++)
            {
                if(tmp_direntry_array[j].name == target && tmp_direntry_array[j].inode_block != 0)
                {
                    target_block = tmp_direntry_array[j].inode_block;
                    //free_block_list[target_block] = true;
                    find = true;
                    tmp_direntry_array[j].inode_block = 0;
                    direntry_block = parent_node.blocks[i];
                    target_index = i;
                    break;
                }
            }
            if(find == true)
            {
                break;
            }
        }

        // if we fail to find the node, error handling
        if(find == false)
        {
            pthread_rwlock_t* temp_lock;
            mutex1.lock();
            temp_lock = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            close(msgsock);
            return;
        }


        pthread_rwlock_t* temp_lock1;
        mutex1.lock();
        temp_lock1 = lock_map[target_block];
        mutex1.unlock();

        pthread_rwlock_wrlock(temp_lock1);

        disk_readblock(target_block, &block_be_deleted);
        // seems useless?
        if(string(block_be_deleted.owner) != user)
        {
            pthread_rwlock_t* temp_lock;
            pthread_rwlock_t* temp_lock2;
            mutex1.lock();
            temp_lock = lock_map[target_block];
            temp_lock2 = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            pthread_rwlock_unlock(temp_lock2);

            close(msgsock);
            return;
        }
        // fs_direntry not empty
        if(block_be_deleted.type == 'd' && block_be_deleted.size != 0)
        {
            pthread_rwlock_t* temp_lock;
            pthread_rwlock_t* temp_lock2;
            mutex1.lock();
            temp_lock = lock_map[target_block];
            temp_lock2 = lock_map[q];
            mutex1.unlock();

            pthread_rwlock_unlock(temp_lock);
            pthread_rwlock_unlock(temp_lock2);

            close(msgsock);
            return;
        }

        //whether this array of direntries is totally empty
        int count = 0;
        for(int i = 0; i < (int)FS_DIRENTRIES; ++i)
        {
            if(tmp_direntry_array[i].inode_block != 0)
            {
                ++count;
            }
        }
        //need to shrink parent's inode's blocks
        if(count == 0)
        {
            for(int i = target_index; i < (int)parent_node.size - 1; ++i)
            {
                parent_node.blocks[i] = parent_node.blocks[i+1];
            }
            --parent_node.size;
        }
        // fs_direntry does not need to shrink
        if(count != 0)
        {
            disk_writeblock(direntry_block, &tmp_direntry_array);
        }
        // need to shrink: update parent inode since child fs_direntry need to be removed
        else
        {
            disk_writeblock(q, &parent_node);
            mutex1.lock();
            // set the empty direntry_block to be free, top->down free
            free_block_list[direntry_block] = true;
            mutex1.unlock();
        }

        pthread_rwlock_t* temp_lock;
        mutex1.lock();
        temp_lock = lock_map[target_block];
        mutex1.unlock();


        pthread_rwlock_unlock(temp_lock);
        pthread_rwlock_destroy(temp_lock);

        mutex1.lock();
        delete lock_map[target_block];

        // set the target to be free
        free_block_list[target_block] = true;
        // will only come to here when target to be deleted is a filename
        for(int i = 0; i < (int)block_be_deleted.size; ++i)
        {
            // if target is filename & still have block associated with it
            // free all block
           free_block_list[block_be_deleted.blocks[i]] = true;
        }
        mutex1.unlock();

        pthread_rwlock_t* temp_lock2;
        mutex1.lock();
        temp_lock2 = lock_map[q];
        mutex1.unlock();
        
        pthread_rwlock_unlock(temp_lock2);
    }
    else
    {
        close(msgsock);
        return;
    }

    return_string = to_string(session) + ' ' + to_string(sequence) + '\0';
    // append the read data to response
    if(strcmp(request_type, "FS_READBLOCK") == 0)
    {
        for(int i = 0; i < (int)FS_BLOCKSIZE; i++)
        {
            return_string += data[i];
        }
    }
    // response construction
    return_string_buf = static_cast<char *>(fs_encrypt((user_list[user]).c_str(), (void*)return_string.c_str(), return_string.length(), &return_size));
    clear_header = to_string(return_size) + '\0';
    send(msgsock, clear_header.c_str(), clear_header.length(), MSG_NOSIGNAL);
    send(msgsock, return_string_buf, return_size, MSG_NOSIGNAL);
    close(msgsock);
    return;
}

int main(int argc, char *argv[])
{
    int port = 0;
    string tmp_user;
    string tmp_pass;
    int on = 1;

    //block number to how many direntry inside the array of direntry
    if(argc == 2)
    {
        port = atoi(argv[1]);
    }
    while(cin >> tmp_user)
    {
        cin >> tmp_pass;
        if(tmp_user.length() > FS_MAXUSERNAME || tmp_pass.length() > FS_MAXPASSWORD)
        {
            continue;
        }
        user_list[tmp_user] = tmp_pass;
    }

    //traverse
    fs_inode root;
    disk_readblock(0, &root);
    travers_tree_inode(&root);
    free_block_list[0] = false;
    pthread_rwlock_t* rwlock = new pthread_rwlock_t;
    pthread_rwlock_init(rwlock, NULL);
    lock_map[0] = rwlock;


    //create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        //cout << "opening socket fail" << endl;
        exit(EXIT_FAILURE);
    }
    //set socket reuse
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    //bind to a port
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    bind(sock, (struct sockaddr*) &addr, sizeof(addr));
    //listen
    rval = listen(sock, 10);
    if (rval == -1)
    {
        //cout << "listening to socket error" << endl;
        exit(EXIT_FAILURE);
    }

    socklen_t length = sizeof(addr);                                    //get correct port number
    rval = getsockname(sock, reinterpret_cast<sockaddr *>(&addr), &length);

    port = ntohs(addr.sin_port);
    cout << "\n@@@ port " << port << endl;

    while(true)
    {
        msgsock = accept(sock, 0, 0);
        thread myThread = thread(request_handler, msgsock);
        myThread.detach();
    }
    return 0;
}
