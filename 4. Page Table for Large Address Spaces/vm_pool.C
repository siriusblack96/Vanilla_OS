/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"


/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address, unsigned long _size, ContFramePool *_frame_pool, PageTable *_page_table) {
   base_address = _base_address;
   size = _size;
   frame_pool = _frame_pool;
   page_table = _page_table;
   vm_pool_next_ptr= NULL;
   region_count= 0;
   
   page_table->register_pool(this);
   
   /*Using the first entry to store details regarding base_address and PAGE_SIZE*/
   virtual_memory_region *ptr_region = (virtual_memory_region*)base_address;
   ptr_region[0].base_address= base_address;
   ptr_region[0].length= PageTable::PAGE_SIZE;
   regions = ptr_region;
   remaining_size-=PageTable::PAGE_SIZE; // First PAGE is taken.
   region_count++; 
     
   Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) 
{
    // If reguested size is greater than the size remaining
    if(_size > remaining_size) {
	Console::puts("VMPOOL: No enough region space \n");
        assert(false);
    }
	
    /* Allocating the regions continuously, based on region_count 
     * which is maintained when a region is allocated 
     */
    unsigned long num_pages = (_size /PageTable::PAGE_SIZE) + (( _size %PageTable::PAGE_SIZE) > 0 ? 1 : 0);
    regions[region_count].base_address = regions[region_count-1].base_address +  regions[region_count-1].length;
    regions[region_count].length = num_pages*PageTable::PAGE_SIZE;
    region_count++;
    remaining_size-=num_pages*PageTable::PAGE_SIZE;
 
    Console::puts("Allocated region of memory.\n");
    
    //return the allocated base_address
    return regions[region_count-1].base_address;
}

void VMPool::release(unsigned long _start_address) {
    // find the region in which address is present.
    int region = -1;
    int i = 1;
    while (i<region_count) {
        if(regions[i].base_address  == _start_address)
            region = i;
        i++;	
    }	 

    // free all the page entries
    unsigned long num_pages = regions[region].length / PageTable::PAGE_SIZE ;
    for (;num_pages > 0; num_pages--) {
        page_table->free_page(_start_address);
        _start_address+=PageTable::PAGE_SIZE;
     }
	 
    /* remove the region contents in the region array, 
     * to use them later for new region allocation
     */
    i = region;
    while (i<region_count) {
        regions[i]=regions[i+1];
        i++;
    }
				
    region_count--;
    remaining_size+=regions[region].length;
    Console::puts("Released region of memory.\n");	
}

bool VMPool::is_legitimate(unsigned long _address) 
{
    /* checks if fault address is within the range of addresses
     * before handling fault
     */
    if((_address > (base_address + size)) || (_address <  base_address))
        return false;
    return true;
    Console::puts("Address is part of an allocated region.\n");
}
