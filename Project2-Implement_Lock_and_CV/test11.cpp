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



////////////////need to add global locks for lock_map and free_block_list and session map






fs_direntry buffer;
fs_inode buffer1;
vector<bool> free_block_list(FS_DISKSIZE, true);
void travers_tree_direntry(uint32_t block);
void travers_tree_inode(fs_inode* root);
int port = 0;
string tmp_user;
string tmp_pass;
int on = 1;
unordered_map<string, string> user_list;
int sock;
struct sockaddr_in addr;
int rval;
int msgsock;
char buf;
unsigned int next_session = 0;
unordered_map<unsigned int, string> session_to_username;
unordered_map<unsigned int, unsigned int> session_to_sequence;

unordered_map<int, pthread_rwlock_t*> lock_map;

mutex mutex1;


void travers_tree_inode(fs_inode* root)
{
    for(int i = 0; i < (int)root->size; i++)
    {
        free_block_list[root->blocks[i]] = false;
        
        if(root->type == 'd')
        {
            travers_tree_direntry(root->blocks[i]);
        }
    }
}

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
        if(string_in[i] != '/')
        {
            tmp += string_in[i];
        }
        else
        {
            if(tmp != "" && tmp.length() <= FS_MAXFILENAME)
            {
                vector_in.push_back(tmp);
            }
            else
            {
                return 1;
            }
            tmp = "";
        }
    }

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
            if(root.owner != username)
            {
                pthread_rwlock_unlock(lock_map[old_block]);
                cout << "unlock " << old_block << endl;
                return -1;
            }
            need_test = false;
        }
        string target = route[0];
        //bool found_it = false;

        if(root.type != 'd')
        {
            pthread_rwlock_unlock(lock_map[old_block]);
                            cout << "unlock " << old_block << endl;
            return -1;
        }

        for(int i = 0; i < (int)root.size; i++)
        {
            fs_direntry buf[FS_DIRENTRIES];
            disk_readblock(root.blocks[i], (void *)buf);
            for(int j = 0; j < (int)FS_DIRENTRIES; j++)
            {
                if((int)buf[j].inode_block != 0)
                {
                    if(buf[j].name == target)
                    {
                        block = buf[j].inode_block;
                        route.erase(route.begin());

                        if(need_write && (int)route.size() == indicator)
                        {
                           pthread_rwlock_wrlock(lock_map[block]);
                        }
                        else
                        {
                            pthread_rwlock_rdlock(lock_map[block]);
                        }

                        cout << "lock " << block << endl;
                        pthread_rwlock_unlock(lock_map[old_block]);
                        cout << "unlock " << old_block << endl;
                        return BFS_helper(block,route, indicator, username, need_test, need_write);
                    }
                }
            }
        }
    }
    //error
    pthread_rwlock_unlock(lock_map[old_block]);
                                            cout << "unlock " << old_block << endl;
    return -1;
}

// BFS to search for inode needed
int BFS(vector<string> &route, int indicator, string username, bool need_write)
{
    cout << "lock 0" << endl;
    if(need_write && (int)route.size() == indicator)
    {
       pthread_rwlock_wrlock(lock_map[0]);
    }
    else
    {
        pthread_rwlock_rdlock(lock_map[0]);
    }
    return BFS_helper(0,route,indicator,username,true,need_write);
}

// find empty spots in file server
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
    fs_inode tmp_inode;
    disk_readblock(block_in, &tmp_inode);

    if(tmp_inode.type != 'd')
    {
        pthread_rwlock_unlock(lock_map[block_in]);
        return false;
    }

    if(tmp_inode.owner != user_in && block_in != 0)
    {
        pthread_rwlock_unlock(lock_map[block_in]);
        return false;
    }

    mutex1.lock();
	  int new_block = get_free_block();
    mutex1.unlock();
    
    if(new_block == -1)
    {
        pthread_rwlock_unlock(lock_map[block_in]);
        return false;
    }

    fs_direntry tmp_direntry_array[FS_DIRENTRIES];
    fs_direntry tmp_used[FS_DIRENTRIES];
    fs_inode empty;
    empty.size = 0;
    empty.type = type_in;
    strcpy(empty.owner, user_in.c_str());
    unsigned int direntry_block = 0;
    bool find = false;
    //if parent inode has block for storing direntries


    if(tmp_inode.size != 0)
    {
        int direntry_index = -1;
        for(int i = 0; i < (int)tmp_inode.size; ++i)
        {
            disk_readblock(tmp_inode.blocks[i], tmp_used);
            for(int j = 0; j < (int)FS_DIRENTRIES; ++j)
            {
                if(tmp_used[j].name == name_in && tmp_used[j].inode_block != 0)
                {
                  free_block_list[new_block] = true;
                  pthread_rwlock_unlock(lock_map[block_in]);
                  return false;
                }
                if(find == false && tmp_used[j].inode_block == 0)
                {
                    find = true;
                    for(int k = 0; k < (int)FS_DIRENTRIES; ++k)
                    {
                        strcpy(tmp_direntry_array[k].name, tmp_used[k].name);
                        tmp_direntry_array[k].inode_block = tmp_used[k].inode_block;
                    }
                    direntry_index = j;
                    direntry_block = tmp_inode.blocks[i];
                }
            }
            if(direntry_index != -1)
            {
                break;
            }
        }
        if(direntry_index != -1)
        {
            pthread_rwlock_t* rwlock = new pthread_rwlock_t;
            pthread_rwlock_init(rwlock, NULL);
            lock_map[new_block] = rwlock;
            pthread_rwlock_wrlock(rwlock);

            disk_writeblock(new_block, &empty);
            pthread_rwlock_unlock(rwlock);

            tmp_direntry_array[direntry_index].inode_block = new_block;
            strcpy(tmp_direntry_array[direntry_index].name, name_in.c_str());
            disk_writeblock(direntry_block, &tmp_direntry_array);

            pthread_rwlock_unlock(lock_map[block_in]);
            return true;
        }
    }

    if(tmp_inode.size >= (int)FS_MAXFILEBLOCKS){
      pthread_rwlock_unlock(lock_map[block_in]);
    	return false;
    }


    //doesn't have free places to store, or parent inode has blocks for storing, but all of them are full
    mutex1.lock();
    int free_block_indx = get_free_block();
    mutex1.unlock();
    if(free_block_indx == -1)
    {
        free_block_list[new_block] = true;
        pthread_rwlock_unlock(lock_map[block_in]);
        return false;
    }


    pthread_rwlock_t* rwlock = new pthread_rwlock_t;
    pthread_rwlock_init(rwlock, NULL);
    lock_map[new_block] = rwlock;
    pthread_rwlock_wrlock(rwlock);

    disk_writeblock(new_block, &empty);
    pthread_rwlock_unlock(rwlock);

    fs_direntry buf[FS_DIRENTRIES];
    for(int i = 0; i < (int)FS_DIRENTRIES; i++)
    {
        strcpy(buf[i].name, "");
        buf[i].inode_block = 0;
    }
    strcpy(buf[0].name, name_in.c_str());
    buf[0].inode_block = new_block;
    disk_writeblock(free_block_indx, &buf);

    tmp_inode.blocks[tmp_inode.size] = free_block_indx;
    tmp_inode.size += 1;
    disk_writeblock(block_in, &tmp_inode);

    pthread_rwlock_unlock(lock_map[block_in]);
    return true;
}


void request_handler(int msgsock){
    string clear_input;
    if (msgsock == -1)
    {
        exit(EXIT_FAILURE);
    }
    //receive cleartext header
    rval = recv(msgsock, static_cast<void *>(&buf), 1, MSG_WAITALL);
    int cleartext_len = 1;
    bool fail = false;

    while(buf != '\0')
    {
        clear_input = clear_input + buf;
        rval = recv(msgsock, static_cast<void *>(&buf), 1, MSG_WAITALL);
        cleartext_len++;
        if(cleartext_len > (int)FS_MAXUSERNAME + 12)
        {
            fail = true;
            break;
        }
    }

    if(fail)
    {
        close(msgsock);
        return;
    }

    char user[FS_MAXUSERNAME + 1];
    unsigned int len;

    if(sscanf(clear_input.c_str(),"%s %u",user, &len) != 2)
    {
        close(msgsock);
        return;
    }

    string trash;
    trash = string(user) + " " + to_string(len);

    if(trash != clear_input)
    {
        close(msgsock);
        return;
    }

    if(user_list.find(string(user)) == user_list.end())
    {
        close(msgsock);
        return;
    }


    //receive real request and parse
    char tmp_buf[len];
    char* result_buf;
    unsigned int request_len;
    rval = recv(msgsock, static_cast<void *>(tmp_buf), len, MSG_WAITALL);
    result_buf = static_cast<char *>(fs_decrypt((user_list[user]).c_str(), &tmp_buf, len, &request_len));

    if(!result_buf)
    {
        close(msgsock);
        return;
    }
    char request_type[15];
    if(sscanf(result_buf,"%s",request_type) != 1)
    {
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
    cout << request_type << endl;
    // deal with request that is not fs_session, check if username exist, session is correct
    // and check whether sequence is valid

    if(strcmp (request_type, "FS_SESSION") != 0)
    {
        if(sscanf(result_buf,"%s %u %u", request_type, &session, &sequence) != 3)
        {
            close(msgsock);
            return;
        }
        if(session_to_username.find(session) == session_to_username.end() || session_to_username[session] != user || sequence <= session_to_sequence[session])
        {
            close(msgsock);
            return;
        }
        session_to_sequence[session] = sequence;
    }

    char data[FS_BLOCKSIZE + 1];

    if(strcmp (request_type, "FS_SESSION") == 0)
    {
        // if input format does not follow: FS_SESSION <session> <sequence><NULL>
        if(sscanf(result_buf,"%s %u %u",request_type, &session, &sequence) != 3)
        {
            close(msgsock);
            return;
        }

        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence);

        if(trash != result_buf)
        {
            close(msgsock);
            return;
        }


        if(next_session > std::numeric_limits<unsigned int>::max())
        {
            close(msgsock);
            return;
        }
        session = next_session;
        ++next_session;

        session_to_sequence[session] = sequence;
        session_to_username[session] = user;
    }
    else if(strcmp (request_type, "FS_READBLOCK") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1];
        unsigned int block;

        // if input format does not follow: FS_READBLOCK <session> <sequence> <pathname> <block><NULL>
        if(sscanf(result_buf,"%s %u %u %s %u",request_type, &session, &sequence, pathname, &block) != 5)
        {
            close(msgsock);
            return;
        }

        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname + " " + to_string(block);

        if(trash != result_buf)
        {
            close(msgsock);
            return;
        }

        if(block >= (int)FS_MAXFILEBLOCKS){
        	close(msgsock);
        	return;
        }

        if(string(pathname).length() > FS_MAXPATHNAME){
            close(msgsock);
            return;
        }

        vector<string> route;
        // if separate_route return 1, means that pathway is not valid
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }
        int q = BFS(route, 0,user,false);
        if(q == -1)
        {
            close(msgsock);
            return;
        }

        //pthread_rwlock_rdlock(lock_map[q]);
        fs_inode target_node;
        disk_readblock(q, &target_node);

        if(target_node.type == 'f' && target_node.size > block)
        {
            disk_readblock(target_node.blocks[block],data);
            pthread_rwlock_unlock(lock_map[q]);
        }
        else
        {
            pthread_rwlock_unlock(lock_map[q]);
            close(msgsock);
            return;
        }
    }
    else if(strcmp (request_type, "FS_WRITEBLOCK") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1];
        unsigned int block;
        // if input format does not follow: FS_WRITEBLOCK <session> <sequence> <pathname> <block><NULL><data>
        if(sscanf(result_buf,"%s %u %u %s %u",request_type, &session, &sequence, pathname, &block) != 5)
        {
            close(msgsock);
            return;
        }

        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname + " " + to_string(block);

        if(trash != result_buf)
        {
            close(msgsock);
            return;
        }

        if(string(pathname).length() > FS_MAXPATHNAME){
            close(msgsock);
            return;
        }

        if(block >= (int)FS_MAXFILEBLOCKS){
        	close(msgsock);
        	return;
        }

        int data_idx = 0;
        int start_place;
        int character_count = 0;

        for(int i = 0; i < (int)request_len; i++)
        {
            if(result_buf[i] == '\0')
            {
                start_place = i;
                break;
            }
        }
        for(int i = start_place + 1; i < (int)request_len; ++i){
            data[data_idx] = result_buf[i];
            data_idx++;
            ++character_count;
        }
        if(character_count != FS_BLOCKSIZE)
        {
            close(msgsock);
            return;
        }
        vector<string> route;
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }
        int q = BFS(route, 0,user,true);
        if(q == -1)
        {
            close(msgsock);
            return;
        }

        //pthread_rwlock_wrlock(lock_map[q]);
        fs_inode target_node;
        disk_readblock(q, &target_node);
        if(target_node.type == 'f' && target_node.size >= block)
        {
            bool need_update = false;
            if(target_node.size == block)
            {
                mutex1.lock();
                int good_block = get_free_block();
                mutex1.lock();
                
                if(good_block == -1)
                {
                    pthread_rwlock_unlock(lock_map[q]);
                    close(msgsock);
                    return;
                }
                target_node.size++;
                target_node.blocks[block] = good_block;
                need_update = true;
            }
            disk_writeblock(target_node.blocks[block],data);
            if(need_update)
            {
                disk_writeblock(q,&target_node);
            }
            pthread_rwlock_unlock(lock_map[q]);
        }
        else
        {
            pthread_rwlock_unlock(lock_map[q]);
            close(msgsock);
            return;
        }
    }
    else if(strcmp(request_type, "FS_CREATE") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1];
        char type;

        // check if format match: FS_CREATE <session> <sequence> <pathname> <type><NULL>
        if(sscanf(result_buf,"%s %u %u %s %c",request_type, &session, &sequence, pathname,&type) != 5)
        {
            close(msgsock);
            return;
        }

        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname + " " + type;

        if(trash != result_buf)
        {
            close(msgsock);
            return;
        }

        if(string(pathname).length() > FS_MAXPATHNAME){
            close(msgsock);
            return;
        }

        if(type != 'd' && type != 'f'){
            close(msgsock);
            return;
        }
        vector<string> route;
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }
        //cout << "session   " << session << endl;
        //cout << "sequence  " << sequence << endl;
        int q = BFS(route, 1,user,true);

        if(q == -1)
        {
            close(msgsock);
            return;
        }

        int z = create(q, route[route.size()-1], type, string(user));
        if(z == 0)
        {
            close(msgsock);
            return;
        }
    }
    else if(strcmp(request_type, "FS_DELETE") == 0)
    {
        char pathname[FS_MAXPATHNAME + 1];

        // check if format match: FS_DELETE <session> <sequence> <pathname><NULL>
        if(sscanf(result_buf,"%s %u %u %s",request_type, &session, &sequence, pathname) != 4)
        {
            close(msgsock);
            return;
        }
        trash = string(request_type) + " " + to_string(session) + " " + to_string(sequence) + " " + pathname;
        // error checking: throw trash message
        if(trash != result_buf)
        {
            close(msgsock);
            return;
        }
        // error checking: pathname too long
        if(string(pathname).length() > FS_MAXPATHNAME){
            close(msgsock);
            return;
        }
        vector<string> route;
        // return 1: invalid pathname
        if(seperate_route(pathname, route))
        {
            close(msgsock);
            return;
        }
        // get parent inode
        int q = BFS(route, 1,user,true);
        if(q == -1)
        {
            close(msgsock);
            return;
        }

        //pthread_rwlock_wrlock(lock_map[q]);

        fs_inode parent_node;
        fs_inode block_be_deleted;
        disk_readblock(q, &parent_node);

        // seems useless?? BFS has already check this
        if(strcmp(parent_node.owner, user) != 0 && q != 0)
        {
            pthread_rwlock_unlock(lock_map[q]);
            close(msgsock);
            return;
        }
        // added by junyue!
        if(parent_node.type == 'f')
        {
            pthread_rwlock_unlock(lock_map[q]);
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
            pthread_rwlock_unlock(lock_map[q]);
            close(msgsock);
            return;
        }

        pthread_rwlock_wrlock(lock_map[target_block]);
        disk_readblock(target_block, &block_be_deleted);
        // seems useless?
        if(string(block_be_deleted.owner) != user)
        {
            pthread_rwlock_unlock(lock_map[target_block]);
            pthread_rwlock_unlock(lock_map[q]);
            close(msgsock);
            return;
        }
        // fs_direntry not empty
        if(block_be_deleted.type == 'd' && block_be_deleted.size != 0)
        {
            pthread_rwlock_unlock(lock_map[target_block]);
            pthread_rwlock_unlock(lock_map[q]);
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
	    free_block_list[direntry_block] = true;
        }
        // for file: go through every data block assicoate with current file
        /*for(int i = 0; i < (int)block_be_deleted.size; ++i)
        {
            free_block_list[block_be_deleted.blocks[i]] = true;
        }*/
        // if fs_direntry is not empty, update fs_direntry
        // else: update parent node
        if(count != 0)
        {
            disk_writeblock(direntry_block, &tmp_direntry_array);
        }
        else
        {
            disk_writeblock(q, &parent_node);
        }


        pthread_rwlock_unlock(lock_map[target_block]);
        pthread_rwlock_destroy(lock_map[target_block]);
        delete lock_map[target_block];
        pthread_rwlock_unlock(lock_map[q]);


        for(int i = 0; i < (int)block_be_deleted.size; ++i)
        {
            free_block_list[block_be_deleted.blocks[i]] = true;
        }
	free_block_list[target_block] = true;
    }
    else
    {
        close(msgsock);
        return;
    }

    return_string = to_string(session) + ' ' + to_string(sequence) + '\0';

    if(strcmp(request_type, "FS_READBLOCK") == 0)
    {
        for(int i = 0; i < (int)FS_BLOCKSIZE; i++)
        {
            return_string += data[i];
        }
    }

    return_string_buf = static_cast<char *>(fs_encrypt((user_list[user]).c_str(), (void*)return_string.c_str(), return_string.length(), &return_size));
    clear_header = to_string(return_size) + '\0';
    send(msgsock, clear_header.c_str(), clear_header.length(), MSG_NOSIGNAL);
    send(msgsock, return_string_buf, return_size, MSG_NOSIGNAL);
    close(msgsock);
    return;
}

int main(int argc, char *argv[])
{
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
