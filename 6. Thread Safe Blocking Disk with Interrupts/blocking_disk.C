/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "thread.H"
#include "scheduler.H"
#include "simple_disk.H"

extern Scheduler * SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/
int mutex;
int test_and_set(int *mutex) 
{
    int tmp = *mutex;
    *mutex = 1;
    return tmp;
}

void mutex_init(int *mutex) 
{
   *mutex = 0;
}

void mutex_lock() 
{
   while(test_and_set(&mutex));
}

void mutex_unlock()
{
   mutex = 0;
}
BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
  mutex_init(&mutex);
#ifdef INTERRUPTS_ENABLED
    head_queue = NULL;
    tail_queue = NULL;
#endif
}

void BlockingDisk::wait_until_ready()
{
	while (! SimpleDisk::is_ready()) {
#ifdef INTERRUPTS_ENABLED
        push (Thread::CurrentThread());
#else
		SYSTEM_SCHEDULER->resume(Thread::CurrentThread());
#endif
		SYSTEM_SCHEDULER->yield();
	}
}
/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  mutex_lock();
  SimpleDisk::read(_block_no, _buf);
  mutex_unlock();
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  mutex_lock();
  SimpleDisk::write(_block_no, _buf);
  mutex_unlock();
}
#ifdef INTERRUPTS_ENABLED
Thread * BlockingDisk::pop() 
{

    if (head_queue != NULL)
    {
        ready_queue *temp_queue_entry = head_queue;
        Thread * next_thread_id = temp_queue_entry->thread_id;
        // Update the head
        head_queue = head_queue->next;
        if (head_queue == NULL)
            tail_queue = NULL;

        // Delete the queue entry
        delete temp_queue_entry;
        return next_thread_id;
    }
}

void BlockingDisk::push(Thread * _thread)
{
    if (head_queue == NULL || tail_queue == NULL)
    {
        // First entry in queue
        ready_queue *new_queue_entry = new ready_queue;
        new_queue_entry->thread_id = _thread;
        head_queue = tail_queue = new_queue_entry;
        new_queue_entry->next = NULL;
    }
    else
    {
        // Queue already present. Add entry
        ready_queue *new_queue_entry = new ready_queue;
        new_queue_entry->thread_id = _thread;
        tail_queue->next = new_queue_entry;
        tail_queue = new_queue_entry;
        new_queue_entry->next == NULL;
    }
}

void BlockingDisk::handle_interrupt(REGS *_r) {
    Thread * next_thread_id = pop();
    SYSTEM_SCHEDULER->resume(next_thread_id->CurrentThread());

}
#endif

