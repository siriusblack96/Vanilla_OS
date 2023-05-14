#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   //assert(false);
   PageTable::kernel_mem_pool = _kernel_mem_pool;
   PageTable::process_mem_pool = _process_mem_pool;
   PageTable::shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   //assert(false);
    unsigned long *directly_mapped_PT = (unsigned long *)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);
    unsigned long shared_frames = ( PageTable::shared_size / PAGE_SIZE);
    unsigned long m_address = 0;
    int i = 0;

    while (i < shared_frames) 
    {
        directly_mapped_PT[i] = m_address | 2 | 1 ;
        m_address += PAGE_SIZE;
        i = i + 1;
    }

    page_directory = (unsigned long *)(kernel_mem_pool->get_frames(1)*PAGE_SIZE);
    page_directory[0] = (unsigned long)directly_mapped_PT | 2 | 1;

    m_address = 0;
    i = 1;
    while (i< shared_frames) 
    {
        page_directory[i] = m_address | 2;
        i++;
    }
    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long)page_directory);
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
    unsigned long page_address = read_cr2();
    unsigned long PD_address   = page_address >> 22;
    unsigned long PT_address   = page_address >> 12;
    unsigned long *current_PD = (unsigned long *) read_cr3();
    unsigned long error_code = _r->err_code;
    unsigned long *page_table = NULL;
    unsigned long mask_addr = 0;
    int i = 0;

    if (!(1&error_code)) 
    {
        if (1&current_PD[PD_address]) 
        {
            page_table = (unsigned long *)(current_PD[PD_address] & 0xFFFFF000);
            page_table[PT_address & 0x3FF] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE) | 2 | 1 ;

        } 
        else 
        {
            current_PD[PD_address] = (unsigned long)((kernel_mem_pool->get_frames(1)*PAGE_SIZE) | 2 | 1);
            page_table = (unsigned long *)(current_PD[PD_address] & 0xFFFFF000);
	
            while (i<1024) 
            {
                page_table[i] = mask_addr | 4 ;
                i++;
            }
            page_table[PT_address & 0x3FF] = (PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE) | 2 | 1;

        }
    }
  Console::puts("handled page fault\n");
}

