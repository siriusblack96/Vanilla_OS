/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define enabled Machine::interrupts_enabled()
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "machine.H"

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
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  queueSize = 0;
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    if (enabled)
    {
        Machine::disable_interrupts();
    }
  
    if(queueSize!=0) 
    {
        queueSize = queueSize - 1;
        Thread *currentThread = readyQueue.dequeue(); 	
        if(!enabled)
        {
            Machine::enable_interrupts();
        }
        Thread::dispatch_to(currentThread);
    }
}

void Scheduler::resume(Thread * _thread) {
    if (enabled)
        Machine::disable_interrupts();

    readyQueue.enqueue(_thread);
    queueSize = queueSize + 1;
  
    if(!enabled)
        Machine::enable_interrupts();
}

void Scheduler::add(Thread * _thread) {
    if (enabled)
        Machine::disable_interrupts();

    readyQueue.enqueue(_thread);
    queueSize = queueSize + 1;
 
    if(!enabled)
    Machine::enable_interrupts();
}

void Scheduler::terminate(Thread * _thread) {
    Console::puts("TERMINATE\n");
    bool threadFound = false;
    int counter = 0;
    
    if (enabled)
        Machine::disable_interrupts();
    
    while (counter < queueSize) {
        Thread* temp=readyQueue.dequeue();
        if (temp->ThreadId()==_thread->ThreadId()){
            threadFound=true;
        } 
        else {
            readyQueue.enqueue(temp);
        }
        counter++;
    }
	
    if(threadFound) 
    {
        queueSize = queueSize - 1;		
    }
	
    if(!enabled)
        Machine::enable_interrupts();
}

RRScheduler::RRScheduler() {
  hz = 5 ;
  ticks = 0;
  queueSize = 0;
  InterruptHandler::register_handler(0, this);
  set_frequency(hz);	
}

void RRScheduler::set_frequency(int _hz) {
    hz = _hz;
    int divisor = 1193180 / _hz;
    Machine::outportb(0x43, 0x34);
    Machine::outportb(0x40, divisor & 0xFF);
    Machine::outportb(0x40, divisor >> 8);
}

void RRScheduler::handle_interrupt(REGS *_r) {
    ticks++;

    if (ticks >= hz ){
      ticks = 0;
      Console::puts("50 ms second has passed\n");
      //resume(Thread::CurrentThread());               
      yield();
    }
}

void RRScheduler::yield() {
    Machine::outportb(0x20, 0x20);

    if (enabled)
        Machine::disable_interrupts();
    
 
    if(queueSize!=0){
        Thread *currentThread = readyRRQueue.dequeue(); 	
        ticks = 0;
        queueSize = queueSize - 1;
        if(!enabled)
            Machine::enable_interrupts();
        
        Thread::dispatch_to(currentThread);
  }
}

void RRScheduler::resume(Thread * _thread) {
  if (enabled)
    Machine::disable_interrupts();


  readyRRQueue.enqueue(_thread);
  queueSize = queueSize + 1;
  
  if(!enabled)
    Machine::enable_interrupts();

}

void RRScheduler::add(Thread * _thread) {
    if (enabled)
        Machine::disable_interrupts();

    readyRRQueue.enqueue(_thread);
    queueSize = queueSize + 1;
 
    if(!enabled)
        Machine::enable_interrupts();

}

void RRScheduler::terminate(Thread * _thread) {
    Console::puts("TERMINATE\n");
    bool threadFound = false;
    int counter = 0;
    
    if (enabled)
        Machine::disable_interrupts();

    while (counter < queueSize)
    {
        Thread* temp=readyRRQueue.dequeue();
        if (temp->ThreadId()==_thread->ThreadId())
        {
            threadFound=true;
        } 
        else 
        {
            readyRRQueue.enqueue(temp);
        }
        counter++;
    }
    if(threadFound)
        queueSize = queueSize - 1;

	
   // if (!Machine::interrupts_enabled())
   if (!enabled)
        Machine::enable_interrupts();
}
