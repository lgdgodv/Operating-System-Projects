#include <sys/types.h>
#include "vm_arena.h"
#include "vm_pager.h"
#include <iostream>
#include <map>
#include <cstring>
#include <queue>
#include <utility>
#include <vector>
#include <sstream>
#include <string>
#include <unordered_map>
using std::unordered_map;
using std::queue;
using std::cout;
using std::endl;
using std::pair;
using std::vector;
using std::string;

struct filenamePair
{
    string filename = "";
    size_t block = 0;
};

struct virtualPage
{
    int swap_block_index = -1;
    bool type = false;
    bool deferred = false;
    int page_idx = -1;
    bool read = false;
    bool write = false;
    bool reference = true;
    bool dirty = false;
    filenamePair filename_pair;
    vector<page_table_entry_t*> pte_list;
};

struct process
{
    page_table_t* pt;
    void* arena_addr;
    int idx_count;
    int swap_block_used;
};

size_t num_memory_pages;
size_t num_swap_blocks;
size_t swap_blocks_left;
int vm_limit = VM_ARENA_SIZE/VM_PAGESIZE;
pid_t current_pid;
queue <int> evict_queue;           //multiprocess may double push
//vector <page*>    physical_page;
vector<vector<int>> swap_file_blocks;
vector<virtualPage*> physical_page;

unordered_map<page_table_entry_t*, virtualPage*> virtual_map;
unordered_map<filenamePair*, int> file_back_map;
unordered_map<filenamePair*, virtualPage*> back_page_entry;
unordered_map<pid_t, process*> process_map;
unordered_map<page_table_entry_t*, pid_t> entry_process_map;

int findValidSpot()
{
    for(int i = 1; i < (int)physical_page.size(); ++i)
    {
        if(physical_page[i] == nullptr)
            return i;
    }
    return -1;
}

int findAvaliableSwapBlocks()
{
    for(int i = 0; i < (int)swap_file_blocks.size(); ++i)
    {
        if(swap_file_blocks[i].empty())
            return i;
    }
    return -1;
}

void clear_evict_queue(int clear_place)
{
    int temp = (int)evict_queue.size();              //evict_queue real size
    for(int i = 0; i < temp; ++i)
    {
        int a = evict_queue.front();
        evict_queue.pop();
        if(a != clear_place)
        {
            evict_queue.push(a);
        }
    }
}

void updateSwapPageAndWrite(int swap_index, int page_idx, page_table_entry_t* pt_entry)
{
    const intptr_t phymem_addr = (const intptr_t)vm_physmem;
    file_write(nullptr, swap_index, (void*)(phymem_addr + pt_entry->ppage*VM_PAGESIZE));
    physical_page[page_idx]->swap_block_index = swap_index;
    physical_page[page_idx]->filename_pair.filename = "";
    physical_page[page_idx]->filename_pair.block = swap_index;
}

void updateReadWrite(int page_idx, bool write_flag, page_table_entry_t* pt)
{
    physical_page[page_idx]->page_idx = page_idx;
    physical_page[page_idx]->reference = true;
    physical_page[page_idx]->deferred = false;
    physical_page[page_idx]->read = true;
    physical_page[page_idx]->write = false;

    if(write_flag)
    {
        physical_page[page_idx]->write = true;
        physical_page[page_idx]->dirty = true;
    }
    
    for(int i = 0; i < (int)physical_page[page_idx]->pte_list.size(); ++i)
    {
        page_table_entry_t* pt_entry = physical_page[page_idx]->pte_list[i];
        pt_entry->ppage = page_idx;
        pt_entry->read_enable = 1;
        pt_entry->write_enable = 0;
        pt_entry->write_enable = write_flag;
    }
}

void updateClockEvictingPage(int idx)
{
    physical_page[idx]->dirty = false;
    physical_page[idx]->read = 0;
    physical_page[idx]->write = 0;
    physical_page[idx]->reference = false;
    physical_page[idx]->deferred = true;
    physical_page[idx]->page_idx = -1;
    for(int i = 0; i < (int)physical_page[idx]->pte_list.size(); ++i)
    {
        page_table_entry_t* cur_entry = physical_page[idx]->pte_list[i];
        cur_entry->read_enable = 0;
        cur_entry->write_enable = 0;
    }
}

void fileBackWriteToDisk(const char* filename, size_t block, int idx)
{
    const intptr_t phymem_addr = (const intptr_t)vm_physmem;
    if(physical_page[idx]->dirty)
    {
        file_write(filename, block, (void*)(phymem_addr + idx*VM_PAGESIZE));
    }
    file_back_map.erase(&physical_page[idx]->filename_pair);
}

int clock_algorithm()
{
    // update all clock entry to be reference  = 0 and push back to the end of evict queue
    int idx;
    //cout << "clock algorithm: \n";
    while(physical_page[evict_queue.front()]->reference)
    {
        //cout << "clock turn: " << evict_queue.front() << endl;
        int temp_idx = evict_queue.front();
        physical_page[temp_idx]->reference = false;
        //read write = 0
        physical_page[temp_idx]->read = 0;
        physical_page[temp_idx]->write = 0;
        for(int i = 0; i < (int)physical_page[temp_idx]->pte_list.size(); ++i)
        {
            physical_page[temp_idx]->pte_list[i]->read_enable = 0;
            physical_page[temp_idx]->pte_list[i]->write_enable = 0;
        }
        evict_queue.pop();
        evict_queue.push(temp_idx);
    }
    //cout << "end of clock: evicting page: " << evict_queue.front() << endl;
    // swap file
    idx = evict_queue.front();
    bool type = physical_page[idx]->type;
    page_table_entry_t* pt_entry = nullptr;
    if(!physical_page[idx]->pte_list.empty())
    {
        pt_entry = physical_page[idx]->pte_list[0];
    }
    if(type == false)
    {
        int swap_index;
        if(physical_page[idx]->swap_block_index == -1)
        {
            // no swap block available, write to swap filee with the block available found
            swap_index = findAvaliableSwapBlocks();
            physical_page[idx]->swap_block_index = swap_index;
            swap_file_blocks[swap_index].push_back(current_pid);
            updateSwapPageAndWrite(swap_index, idx, pt_entry);
        }
        else if(physical_page[evict_queue.front()]->dirty)
        {
            swap_index = (int)physical_page[idx]->filename_pair.block;
            updateSwapPageAndWrite(swap_index, idx, pt_entry);
        }
    }
    // file-back
    else
    {
        // no page_table_entry pointing at this back page file
        if(pt_entry == nullptr)
        {
            for(auto &x : file_back_map)
            {
                if(x.second == idx)
                {
                    const char* filename = x.first->filename.c_str();
                    size_t block = x.first->block;
                    fileBackWriteToDisk(filename, block, idx);
                    break;
                }
            }
        }
        else
        {
            // file-back file. if dirty, need to write to the disk
            const char* filename = physical_page[idx]->filename_pair.filename.c_str();
            size_t block = physical_page[idx]->filename_pair.block;
            fileBackWriteToDisk(filename, block, idx);
        }
    }
    
    // remove the least recent page-table entry and set the bit to be invalid: (0, 0)
    // need to update virtual page from virtual_map, all back page sharing same virtualPahe
    updateClockEvictingPage(idx);
    
    if(physical_page[idx]->type && physical_page[idx]->pte_list.empty())
    {
        back_page_entry.erase(&physical_page[idx]->filename_pair);
        //delete physical_page[idx]->filename_pair;
        delete physical_page[idx];
    }
    evict_queue.pop();
    return idx;
}

int allocatePageHelper(page_table_entry_t* pt_entry, bool write_flag, filenamePair* store_address)
{
    int idx = findValidSpot();
    int result = 0;
    if(idx == -1)
    {
        idx = clock_algorithm();
        if(idx == -1)
            return -1;
    }
    physical_page[idx] = virtual_map[pt_entry];
    const intptr_t phy_addr = (intptr_t)vm_physmem;
    // read the page from swap file or disk
    if(!virtual_map[pt_entry]->type)
    {
        result = file_read(nullptr, store_address->block, (void *)(phy_addr+idx * VM_PAGESIZE));
        if(result == -1)
        {
            //delete physical_page[idx];
            physical_page[idx] = nullptr;
            return result;
        }
        physical_page[idx]->type = false;
    }
    else
    {
        result = file_read(store_address->filename.c_str(), store_address->block, (void *)(phy_addr+idx * VM_PAGESIZE));
        if(result == -1)
        {
            
            //delete physical_page[idx];
            physical_page[idx] = nullptr;
            return result;
        }
        file_back_map[&physical_page[idx]->filename_pair] = idx;
        physical_page[idx]->type = true;
    }
    evict_queue.push(idx);

    updateReadWrite(idx, false, pt_entry);
    return result;
}

int convertAddress(const char* virtual_addr)
{
    const intptr_t base = (intptr_t) VM_ARENA_BASEADDR;
    intptr_t filename_addr = (intptr_t)virtual_addr;
    int filename_idx = (int)(filename_addr - base) / VM_PAGESIZE;
    page_table_entry_t* pt_entry_filename = &page_table_base_register->ptes[filename_idx];
    
    if(pt_entry_filename->read_enable == 0)
    {
        // simulate the MMU and trigger vm_fault
        int result = vm_fault(virtual_addr, 0);
        if(result == -1)
            return -1;
    }
    int offset = ((filename_addr - (intptr_t)VM_ARENA_BASEADDR)% VM_PAGESIZE);
    int realName_page = pt_entry_filename->ppage * VM_PAGESIZE + offset;
    
    return realName_page;
}


void vm_init(size_t memory_pages, size_t swap_blocks)
{
    num_memory_pages = memory_pages;
    num_swap_blocks = swap_blocks;
    swap_blocks_left = swap_blocks;
    physical_page.resize(memory_pages);
    //vector<int> temp;
    swap_file_blocks.resize(swap_blocks);
    for(int i = 0; i < (int)physical_page.size(); i++)
    {
        physical_page[i] = nullptr;
    }
    // pinning memory
    virtualPage* pinPage = new virtualPage;
    pinPage->page_idx = 0;
    pinPage->read = 1;
    for(int i = 0; i < (int)VM_PAGESIZE; ++i)
    {
        ((char*)vm_physmem)[i] = '\0';
    }
    physical_page[0] = pinPage;
}


int vm_create(pid_t parent_pid, pid_t child_pid)
{
    //need to handle eager swap reservation
    page_table_t* page_table = new page_table_t;
    for(int i = 0; i < vm_limit; ++i)
    {
        page_table->ptes[i].read_enable = 0;
        page_table->ptes[i].write_enable = 0;
    }
    
    if(process_map.find(parent_pid) == process_map.end())
    {
        process_map[child_pid] = new process;
        process_map[child_pid]->arena_addr = VM_ARENA_BASEADDR;
        process_map[child_pid]->pt = page_table;
        process_map[child_pid]->idx_count = 0;
        process_map[child_pid]->swap_block_used = 0;
    }
    else
    {
        size_t eager_blocks = process_map[parent_pid]->swap_block_used;
        if(swap_blocks_left < eager_blocks)
        {
            return -1;
        }
        process_map[child_pid] = new process;

        // eager swap block
        process_map[child_pid]->swap_block_used = (int)eager_blocks;
        swap_blocks_left -= eager_blocks;

        // copy process arena address and page table index assign new page table
        process_map[child_pid]->arena_addr = process_map[parent_pid]->arena_addr;
        process_map[child_pid]->pt = page_table;
        process_map[child_pid]->idx_count = process_map[parent_pid]->idx_count;

        // copy page table entry from parent to child, set read_enable only
        for(int i = 0; i < process_map[parent_pid]->idx_count; ++i)
        {
            page_table_entry_t* parent_pt = &process_map[parent_pid]->pt->ptes[i];
            
            page_table->ptes[i].ppage = parent_pt->ppage;
            page_table->ptes[i].read_enable = parent_pt->read_enable;
            entry_process_map[&page_table->ptes[i]] = child_pid;
            // file back page: point to same virtualPage, copy write bit
            if(virtual_map[parent_pt]->type == true)
            {
                page_table->ptes[i].write_enable = parent_pt->write_enable;
                virtual_map[parent_pt]->pte_list.push_back(&page_table->ptes[i]);
                virtual_map[&page_table->ptes[i]] = virtual_map[parent_pt];
            }
            // swap back page: set all page table entry pointing to the same virtual page
            // not write_enable
            else
            {
                page_table->ptes[i].write_enable = 0;
                virtual_map[parent_pt]->write = 0;
                
                // update to be write_enable = 0 for all page table entry pointint to this swap page
                for(int j = 0; j < (int)virtual_map[parent_pt]->pte_list.size(); ++j)
                {
                    virtual_map[parent_pt]->pte_list[j]->write_enable = 0;
                }
                virtual_map[parent_pt]->pte_list.push_back(&page_table->ptes[i]);
                virtual_map[&page_table->ptes[i]] = virtual_map[parent_pt];
                
                // need to update the swap file block for blocks with parent_id
                for(int j = 0; j < (int)swap_file_blocks.size(); ++j)
                {
                    for(int k = 0; k < (int)swap_file_blocks[j].size(); ++k)
                    {
                        if(swap_file_blocks[j][k] == parent_pid)
                        {
                            swap_file_blocks[j].push_back(child_pid);
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}


void *vm_map(const char *filename, size_t block)
{
    if(process_map[current_pid]->idx_count >= vm_limit)
    {
        return nullptr;
    }
    if(filename)
    {
        if((intptr_t)filename < (intptr_t)VM_ARENA_BASEADDR)
            return nullptr;
        if((intptr_t)filename > ((intptr_t)VM_ARENA_BASEADDR + (intptr_t)VM_ARENA_SIZE))
            return nullptr;
    }
    // create new page table entry
    int spot_idx = process_map[current_pid]->idx_count;
    page_table_entry_t* pt_entry = &page_table_base_register->ptes[spot_idx];
    virtualPage* page;
    if(!filename)
    {
        page = physical_page[0];
        //page->filename_pair = new filenamePair;
        if(swap_blocks_left == 0)
            return nullptr;
        ++process_map[current_pid]->swap_block_used;
        --swap_blocks_left;
        pt_entry->read_enable = 1;
        pt_entry->write_enable = 0;
        pt_entry->ppage = 0;
    }
    else
    {
        //valid string read enable: swap file exist in the physical memory
        string realname = "";
        bool not_end = true;
        const char* cur_virtual_addr = filename - (intptr_t)filename % (intptr_t)VM_PAGESIZE;
        
        // find the real filename in string, terminated by \0
        while(not_end)
        {
            int realname_int = convertAddress(filename);
            if(realname_int == -1)
                return nullptr;
            for(int i = realname_int; i < (int)VM_PAGESIZE * (1+ realname_int/(int)VM_PAGESIZE); ++i)
            {
                if(*(((char*)vm_physmem)+i) != '\0')
                {
                    realname += *(((char*)vm_physmem)+i);
                }
                else
                {
                    not_end = false;
                    break;
                }
            }
            if (not_end)
            {
                filename = cur_virtual_addr + VM_PAGESIZE;
                if((intptr_t)filename > ((intptr_t)VM_ARENA_BASEADDR + process_map[current_pid]->idx_count * (int)VM_PAGESIZE))
                    return nullptr;
            }
        }

        bool findBackPageEntry = false;
        bool findFileBackMap = false;
        int page_idx = 0;
        filenamePair* ptr;
        
        for(auto &x : back_page_entry)
        {
            if(x.first->filename == realname && x.first->block == block)
            {
                findBackPageEntry = true;
                ptr = x.first;
                break;
            }
        }
        for(auto &x : file_back_map)
        {
            if(x.first->filename == realname && x.first->block == block)
            {
                findFileBackMap = true;
                page_idx = x.second;
                break;
            }
        }
        if(!findBackPageEntry)
        {
            page = new virtualPage;
            page->filename_pair.filename = realname;
            page->filename_pair.block = block;
            page->type = true;
            back_page_entry[&page->filename_pair] = page;
        }
        else
        {
            page = back_page_entry[ptr];
        }
        if(findFileBackMap)
        {
            // can find this in the memory
            pt_entry->ppage = page->page_idx;
            pt_entry->read_enable = physical_page[page_idx]->read;
            pt_entry->write_enable = physical_page[page_idx]->write;
        }
        else
        {
            // does not exist in the memory
            page->deferred = true;
            pt_entry->read_enable = 0;
            pt_entry->write_enable = 0;
        }
    }
    page->pte_list.push_back(pt_entry);
    virtual_map[pt_entry] = page;
    process_map[current_pid]->idx_count++;
    //cout << pt_entry << "   " ;
    entry_process_map[pt_entry] = current_pid;
    //cout <<     entry_process_map[pt_entry] << endl;
    // find next available spot in page table for current process id
    // update arena map value to + fff
    void* temp = process_map[current_pid]->arena_addr;
    const intptr_t temp_int = (const intptr_t)temp;
    process_map[current_pid]->arena_addr = (void *)(temp_int + VM_PAGESIZE);
    return temp;
}

int copy_on_write(page_table_entry_t* pt_entry)
{
    //create temp buffer
    char buffer[VM_PAGESIZE];
    //copy to temp buffer
    for(int i = 0 ; i < (int)VM_PAGESIZE; ++i)
    {
        buffer[i] = (((char*)vm_physmem)[pt_entry->ppage * VM_PAGESIZE + i]);
    }
    //erase itself in the old one
    for(int i = 0; i < (int)virtual_map[pt_entry]->pte_list.size(); i++)
    {
        if(virtual_map[pt_entry]->pte_list[i] == pt_entry)
        {
            virtual_map[pt_entry]->pte_list.erase(virtual_map[pt_entry]->pte_list.begin() + i);
            break;
        }
    }
    
    for(int i = 0; i < (int)virtual_map[pt_entry]->pte_list.size(); i++)
    {
        virtual_map[pt_entry]->pte_list[i]->write_enable = false;
        virtual_map[pt_entry]->write = false;
    }
    
    if((int)virtual_map[pt_entry]->pte_list.size() == 1)
    {
        if(virtual_map[pt_entry]->pte_list[0]->read_enable == 1)
        {
            if(virtual_map[pt_entry]->dirty)
            {
                virtual_map[pt_entry]->write = 1;
                virtual_map[pt_entry]->pte_list[0]->write_enable = 1;
            }
        }
    }
    
    virtual_map[pt_entry]->reference = true;
    int idx = findValidSpot();
    if(idx == -1)
    {
        idx = clock_algorithm();
        if(idx == -1)
            return -1;
    }
    
    virtualPage* new_page = new virtualPage;
    new_page-> swap_block_index = findAvaliableSwapBlocks();
    new_page-> page_idx = idx;
    new_page-> read = true;
    new_page-> write = true;
    new_page-> dirty = true;
    new_page-> reference = true;
    new_page -> filename_pair.block = new_page->swap_block_index;
    new_page->pte_list.push_back(pt_entry);
    for(int i = 0; i < (int)swap_file_blocks[virtual_map[pt_entry]->swap_block_index].size(); i++)
    {
        int swap_idx = virtual_map[pt_entry]->swap_block_index;
        if(swap_file_blocks[swap_idx][i] == current_pid)
        {
            swap_file_blocks[swap_idx].erase(swap_file_blocks[swap_idx].begin() + i);
            break;
        }
    }
    swap_file_blocks[new_page-> swap_block_index].push_back(current_pid);
    pt_entry->ppage = idx;
    virtual_map[pt_entry] = new_page;
    physical_page[idx] = virtual_map[pt_entry];
    for(int i = 0; i < (int)VM_PAGESIZE; ++i)
    {
        ((char*)vm_physmem)[idx * VM_PAGESIZE + i] = buffer[i];
    }
    evict_queue.push(idx);
    return 0;
}

int vm_fault(const void *addr, bool write_flag)
{
    const intptr_t addr_int = (intptr_t)addr;
    const intptr_t base = (intptr_t) VM_ARENA_BASEADDR;
    int index_top = (int)(addr_int - base) / VM_PAGESIZE;
    if(index_top >= process_map[current_pid]->idx_count || addr_int < base || index_top >= vm_limit)
        return -1;
    page_table_entry_t* pt_entry = &page_table_base_register->ptes[index_top];
    // 1, 0
    if (pt_entry->read_enable)
    {
        // swapfile that on pin page
        pt_entry->write_enable = 1;
        if(virtual_map[pt_entry]->type == false)
        {
            if((int)virtual_map[pt_entry]->pte_list.size() > 1)
            {
                int result = copy_on_write(pt_entry);
                if(result == -1)
                    return -1;
            }
            else
            {
                if (pt_entry->ppage == 0)
                {
                    /*
                    int idx = findValidSpot();
                    if(idx == -1)
                    {
                        idx = clock_algorithm();
                        if(idx == -1)
                            return -1;
                    }
                    physical_page[idx] = virtual_map[pt_entry];
                    for(int i = idx*VM_PAGESIZE; i < (idx+1)*(int)VM_PAGESIZE; ++i)
                    {
                        ((char*)vm_physmem)[i] = '\0';
                    }
                    evict_queue.push(idx);
                    pt_entry->ppage = idx;
                     */
                    int result = copy_on_write(pt_entry);
                    if(result == -1)
                        return -1;
                }
                else
                {
                    updateReadWrite(pt_entry->ppage, true, pt_entry);
                }
            }
        }
        else
        {
            updateReadWrite(pt_entry->ppage, true, pt_entry);
        }
    }
    // 0,0
    else
    {
        // swapfile 0,0, resident = 1, reference = 0, clock rotate issue
        if(virtual_map[pt_entry]->deferred == false)
        {
            updateReadWrite(pt_entry->ppage, false, pt_entry);
        }
        // in the defer work, resident = false
        else
        {
            //cout << "defer = true, first time load or evicted by clock";
            int result = allocatePageHelper(pt_entry, false, &virtual_map[pt_entry]->filename_pair);
            if(result == -1)
                return -1;
        }
        
        bool control = false;
        if(write_flag || virtual_map[pt_entry]->dirty)
        {
            control = true;
        }
        
        if(control)
        {
            int result = vm_fault(addr, write_flag);
            if(result == -1)
                return -1;
        }
    }
    return 0;
}


void vm_destroy()
{
    for(int i = 0; i < (int)swap_file_blocks.size(); ++i)
    {
        for(int j = 0; j < (int)swap_file_blocks[i].size(); ++j)
        {
            if(current_pid == swap_file_blocks[i][j])
            {
                ++swap_blocks_left;                                        //care
                swap_file_blocks[i].erase(swap_file_blocks[i].begin() + j);
                break;
            }
        }
    }
    
    // don't need to do vm_limit, s
    for(int i = 0; i < process_map[current_pid]->idx_count ; ++i)
    {
        page_table_entry_t* entry = &page_table_base_register->ptes[i];
        
        for(int j = 0; j < (int)virtual_map[entry]->pte_list.size();j++)
        {
            if(virtual_map[entry]->pte_list[j] == entry)
            {
                virtual_map[entry]->pte_list.erase(virtual_map[entry]->pte_list.begin() + j);
                break;
            }
        }
        //back file
        if(virtual_map[entry]->type)
        {
            if(virtual_map[entry]->pte_list.empty() && virtual_map[entry]->deferred)
            {
                back_page_entry.erase(&virtual_map[entry]->filename_pair);
                delete virtual_map[entry];
            }
        }
        else
        {
            if((int)virtual_map[entry]->pte_list.size() == 0 && virtual_map[entry]->page_idx != 0)
            {
                if(!virtual_map[entry]->deferred)
                {
                    int idx = virtual_map[entry]->page_idx;
                    physical_page[idx] = nullptr;
                    clear_evict_queue(idx);
                }
                delete virtual_map[entry];
            }
            if((int)virtual_map[entry]->pte_list.size() == 1 && virtual_map[entry]->page_idx != 0)
            {
                if(virtual_map[entry]->pte_list[0]->read_enable == 1)
                {
                    if(virtual_map[entry]->dirty)
                    {
                        virtual_map[entry]->write = 1;
                        virtual_map[entry]->pte_list[0]->write_enable = 1;
                    }
                }
            }
        }
        virtual_map.erase(entry);
        entry_process_map.erase(entry);
    }
    delete process_map[current_pid]->pt;
    delete process_map[current_pid];
    process_map.erase(current_pid);
}

void vm_switch(pid_t pid)
{
    current_pid = pid;
    page_table_base_register = process_map[pid]->pt;
}

extern int file_read(const char *filename, size_t block, void *buf);
extern int file_write(const char *filename, size_t block, const void *buf);
extern void * const vm_physmem;
extern page_table_t *page_table_base_register;
