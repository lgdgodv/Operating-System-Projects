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
using std::endl;
using std::pair;
using std::vector;
using std::string;
using std::map;

struct virtualPage
{
    int swap_block_index = -1;
    bool type = false;
    // defer is the opposite of resident, defer = true -> resident  = false
    // name this as defer to indicate whether the work to do is deferred or not
    bool deferred = false;
    int page_idx = -1;
    bool read = false;
    bool write = false;
    bool reference = true;
    bool dirty = false;
    pair<int, size_t> filename_pair = {-1, 0};
    vector<page_table_entry_t*> pte_list;
};

struct process
{
    page_table_t* pt;
    // current arena address
    void* arena_addr;
    // next page table index
    int idx_count;
    int swap_block_used;
};

size_t num_memory_pages;
size_t swap_blocks_left;
int vm_limit = VM_ARENA_SIZE/VM_PAGESIZE;
pid_t current_pid;
int global_count = 0;
queue <int> evict_queue;           //multiprocess may double push
//vector <page*>    physical_page;
vector<vector<int>> swap_file_blocks;
vector<virtualPage*> physical_page;

unordered_map<page_table_entry_t*, virtualPage*> virtual_map;
// push to this map when a file back page is read into physical memory
map<pair<int, size_t>, int> file_back_map;
// push to this map when calling vm_map, file back page only
map<pair<int, size_t>, virtualPage*> back_page_entry;
unordered_map<pid_t, process*> process_map;
// given page table entry, we need to know what is it's process id
unordered_map<page_table_entry_t*, pid_t> entry_process_map;
// filename_map used to save memory, assign a memory a count int to represent a map
unordered_map<string, int> filename_map;

// find empty spot in physical memory
int findValidSpot()
{
    for(int i = 1; i < (int)physical_page.size(); ++i)
    {
        if(physical_page[i] == nullptr)
            return i;
    }
    return -1;
}

// find empty swap block in swap file
int findAvaliableSwapBlocks()
{
    for(int i = 0; i < (int)swap_file_blocks.size(); ++i)
    {
        if(swap_file_blocks[i].empty())
            return i;
    }
    return -1;
}

// used in vm_destroy, need to clean the evict queue and keep the sequence unchanged
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

// used in clock algorithm, update the physical page that contains a swap page and about to evicted
void updateSwapPageAndWrite(int swap_index, int page_idx, page_table_entry_t* pt_entry)
{
    const intptr_t phymem_addr = (const intptr_t)vm_physmem;
    file_write(nullptr, swap_index, (void*)(phymem_addr + pt_entry->ppage*VM_PAGESIZE));
    physical_page[page_idx]->swap_block_index = swap_index;
    physical_page[page_idx]->filename_pair.first = -1;
    physical_page[page_idx]->filename_pair.second = swap_index;
}

// general update read write for the specified physical page and all page table entry pointing to this page
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

// used in clock algorithm, update the swap back page bit when this page is about to be evicted
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

// used in clock algorithm, used to update file back page bit that is about to evict
void fileBackWriteToDisk(const char* filename, size_t block, int idx)
{
    const intptr_t phymem_addr = (const intptr_t)vm_physmem;
    if(physical_page[idx]->dirty)
    {
        file_write(filename, block, (void*)(phymem_addr + idx*VM_PAGESIZE));
    }
    file_back_map.erase(physical_page[idx]->filename_pair);
}

// clock: FIFO, update read/write bit and evict the page that is lease referenced
// return the page index that is evicted
int clock_algorithm()
{
    // update all clock entry to be reference  = 0 and push back to the end of evict queue
    int idx;
    while(physical_page[evict_queue.front()]->reference)
    {
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
    
    // find the page index that should be evicted -----------------
    
    // default value is for physical pages that does not have page table entry point to it
    idx = evict_queue.front();
    bool type = physical_page[idx]->type;
    page_table_entry_t* pt_entry = nullptr;
    // update the default value if there is page table entry pointint at this
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
            swap_index = (int)physical_page[idx]->filename_pair.second;
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
                    // get filename from filename_map
                    int temp_idx = x.first.first;
                    const char* filename;
                    for(auto &y : filename_map)
                    {
                        if(y.second == temp_idx)
                        {
                            filename = y.first.c_str();
                        }
                    }
                    size_t block = x.first.second;
                    fileBackWriteToDisk(filename, block, idx);
                    break;
                }
            }
        }
        else
        {
            // file-back file. if dirty, need to write to the disk
            int temp_idx = physical_page[idx]->filename_pair.first;
            const char* filename;
            for(auto &y : filename_map)
            {
                if(y.second == temp_idx)
                {
                    filename = y.first.c_str();
                }
            }
            size_t block = physical_page[idx]->filename_pair.second;
            fileBackWriteToDisk(filename, block, idx);
        }
    }
    
    // remove the least recent page-table entry and set the bit to be invalid: (0, 0)
    // need to update virtual page from virtual_map, all back page sharing same virtualPahe
    updateClockEvictingPage(idx);
    
    if(physical_page[idx]->type && physical_page[idx]->pte_list.empty())
    {
        back_page_entry.erase(physical_page[idx]->filename_pair);
        delete physical_page[idx];
    }
    evict_queue.pop();
    return idx;
}

// allocate page to read the swap back page or file back page into or back to memory
int allocatePageHelper(page_table_entry_t* pt_entry, bool write_flag, pair<int, size_t> store_address)
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
    
    // swap back page
    if(!virtual_map[pt_entry]->type)
    {
        result = file_read(nullptr, store_address.second, (void *)(phy_addr+idx * VM_PAGESIZE));
        if(result == -1)
        {
            physical_page[idx] = nullptr;
            return result;
        }
        physical_page[idx]->type = false;
    }
    // file back page
    else
    {
        // find the filename string from filename_map
        int temp_idx = store_address.first;
        const char* filename;
        for(auto &y : filename_map)
        {
            if(y.second == temp_idx)
            {
                filename = y.first.c_str();
            }
        }
        result = file_read(filename, store_address.second, (void *)(phy_addr+idx * VM_PAGESIZE));
        if(result == -1)
        {
            //delete physical_page[idx];
            physical_page[idx] = nullptr;
            return result;
        }
        file_back_map[physical_page[idx]->filename_pair] = idx;
        physical_page[idx]->type = true;
    }
    evict_queue.push(idx);

    updateReadWrite(idx, false, pt_entry);
    return result;
}

// given virtual address, convert into physical address, simulate MMU
// call vm_fault if virtual page is not resident or being clocked
int convertAddress(const char* virtual_addr)
{
    const intptr_t base = (intptr_t) VM_ARENA_BASEADDR;
    intptr_t filename_addr = (intptr_t)virtual_addr;
    int filename_idx = (int)(filename_addr - base) / VM_PAGESIZE;
    page_table_entry_t* pt_entry_filename = &page_table_base_register->ptes[filename_idx];
    // simulate MMU
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

// allocate memory
void vm_init(size_t memory_pages, size_t swap_blocks)
{
    num_memory_pages = memory_pages;
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
    
    // no fork
    if(process_map.find(parent_pid) == process_map.end())
    {
        process_map[child_pid] = new process;
        process_map[child_pid]->arena_addr = VM_ARENA_BASEADDR;
        process_map[child_pid]->pt = page_table;
        process_map[child_pid]->idx_count = 0;
        process_map[child_pid]->swap_block_used = 0;
    }
    // fork, need to copy page table, allocate swap_block needed, check eager swap block
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
                // share the virtual page with parent temporaly
                virtual_map[&page_table->ptes[i]] = virtual_map[parent_pt];
            }
            // swap back page: set all page table entry pointing to the same virtual page
            // write_enable = false for parent process
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
            }
        }
        
        // need to update the swap file block for blocks with parent_id
        // push the child process id into swap_file_blocks
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
    return 0;
}


void *vm_map(const char *filename, size_t block)
{
    // error checking
    if(process_map[current_pid]->idx_count >= vm_limit)
    {
        return nullptr;
    }
    // if filename is not nullptr, do error checking associated with file back page
    if(filename)
    {
        if((intptr_t)filename < (intptr_t)VM_ARENA_BASEADDR)
        {
            return nullptr;
        }
        if((intptr_t)filename >= ((intptr_t)VM_ARENA_BASEADDR + process_map[current_pid]->idx_count * (int)VM_PAGESIZE))
        {
            return nullptr;
        }
    }
    else
    {
        // check eager swap block
	if(swap_blocks_left == 0)
	{
	    return nullptr;
	}
    }
    // create new page table entry
    int spot_idx = process_map[current_pid]->idx_count;
    page_table_entry_t* pt_entry = &page_table_base_register->ptes[spot_idx];
    virtualPage* page;
    
    // cannot find filename, make current page table entry point to pinpage
    if(!filename)
    {
        page = physical_page[0];
        if(swap_blocks_left == 0)
        {
            return nullptr;
        }
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
        
        // find the real filename in string, terminated by \0, continue reading even if next page
        while(not_end)
        {
            int realname_int = convertAddress(filename);
            if(realname_int == -1)
            {
                return nullptr;
            }
            // start read the string from the given address, read untill the page end
            for(int i = realname_int; i < (int)VM_PAGESIZE * (1+ realname_int/(int)VM_PAGESIZE); ++i)
            {
                //break when see the terminate
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
            // if not end go to the the nest page in the virtual memory
            if (not_end)
            {
                filename = cur_virtual_addr + VM_PAGESIZE;
                if((intptr_t)filename >= ((intptr_t)VM_ARENA_BASEADDR + process_map[current_pid]->idx_count * (int)VM_PAGESIZE))
                    return nullptr;
            }
        }
        
        int filename_idx = -1;
        // check to see if the filename have already exist in the map
        if(filename_map.find(realname) == filename_map.end())
        {
            filename_map[realname] = global_count;
            global_count ++;
        }
        filename_idx = filename_map[realname];

        pair<int, size_t> ptr;
        ptr.first = filename_idx;
        ptr.second = block;
        
        // check if the (filename,block) exsit, if exist share the virtual page, create a new one if not
        if(back_page_entry.find(ptr) == back_page_entry.end())
        {
            page = new virtualPage;
            page->filename_pair = ptr;
            page->type = true;
            back_page_entry[ptr] = page;
        }
        else
        {
            page = back_page_entry[ptr];
        }
        
        // if the (filename,block) has already been read into physical memory, update page table entry
        if(file_back_map.find(ptr) != file_back_map.end())
        {
            // can find this in the memory
            pt_entry->ppage = page->page_idx;
            pt_entry->read_enable = physical_page[file_back_map[ptr]]->read;
            pt_entry->write_enable = physical_page[file_back_map[ptr]]->write;
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
    entry_process_map[pt_entry] = current_pid;
    // find next available spot in page table for current process id and update arena map value to + fff
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
    
    // set all page table entry to be not write enable
    for(int i = 0; i < (int)virtual_map[pt_entry]->pte_list.size(); i++)
    {
        virtual_map[pt_entry]->pte_list[i]->write_enable = false;
        virtual_map[pt_entry]->write = false;
    }
    
    // if only one swap page left, check dirty, if dirty, set write enble
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
    
    // reference the old page
    virtual_map[pt_entry]->reference = true;
    int idx = findValidSpot();
    if(idx == -1)
    {
        idx = clock_algorithm();
        if(idx == -1)
            return -1;
    }
    
    // create new virtualPage for the "copy on write" instance
    virtualPage* new_page = new virtualPage;
    new_page-> swap_block_index = findAvaliableSwapBlocks();
    new_page-> page_idx = idx;
    new_page-> read = true;
    new_page-> write = true;
    new_page-> dirty = true;
    new_page-> reference = true;
    new_page -> filename_pair.second = new_page->swap_block_index;
    new_page->pte_list.push_back(pt_entry);

    // update swap file block
    if(virtual_map[pt_entry]->swap_block_index != -1)
    {
        for(int i = 0; i < (int)swap_file_blocks[virtual_map[pt_entry]->swap_block_index].size(); i++)
        {
            int swap_idx = virtual_map[pt_entry]->swap_block_index;
            if(swap_file_blocks[swap_idx][i] == current_pid)
            {
                swap_file_blocks[swap_idx].erase(swap_file_blocks[swap_idx].begin() + i);
                break;
            }
        }
    }
    swap_file_blocks[new_page-> swap_block_index].push_back(current_pid);
    pt_entry->ppage = idx;
    virtual_map[pt_entry] = new_page;
    physical_page[idx] = virtual_map[pt_entry];
    // read the data back from temporary buffer
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
    // error checking
    if(index_top >= process_map[current_pid]->idx_count || addr_int < base || index_top >= vm_limit)
        return -1;
    page_table_entry_t* pt_entry = &page_table_base_register->ptes[index_top];
    // handle write fault
    if (pt_entry->read_enable)
    {
        // swapfile that on pin page
        pt_entry->write_enable = 1;
        // swap back page
        if(virtual_map[pt_entry]->type == false)
        {
            // need to trigger copy on write, since multiplt process point to same swap back file
            if((int)virtual_map[pt_entry]->pte_list.size() > 1)
            {
                int result = copy_on_write(pt_entry);
                if(result == -1)
                    return -1;
            }
            else
            {
                // pinpage to other physical page, copy on write will handle normally
                if (pt_entry->ppage == 0)
                {
                    int result = copy_on_write(pt_entry);
                    if(result == -1)
                        return -1;
                }
                // swap back page update read/write/dirty bit
                else
                {
                    updateReadWrite(pt_entry->ppage, true, pt_entry);
                }
            }
        }
        // file back page
        else
        {
            updateReadWrite(pt_entry->ppage, true, pt_entry);
        }
    }
    // handle read fault, if write_flag is true, call vm_fault again
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
            int result = allocatePageHelper(pt_entry, false, virtual_map[pt_entry]->filename_pair);
            if(result == -1)
                return -1;
        }
        
        // only three case will be back to write vm_fault, write_flag = true
        // or clocked: 1) file back page dirty 2) only one page table entry pointing to swap back page and it's dirty
        bool control = write_flag;
        if(virtual_map[pt_entry]->type == false && (int)virtual_map[pt_entry]->pte_list.size() == 1 && virtual_map[pt_entry]->dirty)
        {
            control = true;
        }
        if(virtual_map[pt_entry]->type && virtual_map[pt_entry]->dirty)
        {
            control = true;
        }
        
        // back to deal with write fault
        if(control)
        {
            int result = vm_fault(addr, true);
            if(result == -1)
            {
                return -1;
            }
        }
    }
    return 0;
}


void vm_destroy()
{
    // clear current process id from swap_file_block
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
    
    // don't need to do vm_limit, loop from page table
    for(int i = 0; i < process_map[current_pid]->idx_count ; ++i)
    {
        page_table_entry_t* entry = &page_table_base_register->ptes[i];
        
        // clear page table entry from a vector that record that record all page table entry point to this
        for(int j = 0; j < (int)virtual_map[entry]->pte_list.size();j++)
        {
            if(virtual_map[entry]->pte_list[j] == entry)
            {
                virtual_map[entry]->pte_list.erase(virtual_map[entry]->pte_list.begin() + j);
                break;
            }
        }
        // file back page
        if(virtual_map[entry]->type)
        {
            // delete back_page_entry if the filename_pair does not in physical memory and no other pointing
            if(virtual_map[entry]->pte_list.empty() && virtual_map[entry]->deferred)
            {
                back_page_entry.erase(virtual_map[entry]->filename_pair);
                delete virtual_map[entry];
            }
        }
        // swap back page
        else
        {
            // if pinpage, get the swap block left back
	    if(virtual_map[entry]->page_idx == 0)
	    {
		++swap_blocks_left;
	    }
            // not pinpage, and vector of all page table entry pointint to this is empty, free memory & evict queue
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
            // not pinpage, only one page table entry pointint to this, update write enable if dirty originally
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
