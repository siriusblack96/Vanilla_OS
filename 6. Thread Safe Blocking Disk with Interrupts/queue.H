class Queue {
    private:
      Thread* tObj;
      Queue* next;
    public:
      Queue() {
        tObj = NULL;
        next = NULL;
      }
      
      Queue(Thread* t)
	  {
        tObj = t;
        next = NULL;
      }
      
      void enqueue(Thread* t) {
        if (!tObj) {
            tObj = t;  //if threadobject is NULL, assign threadObj as incoming thread :HEAD
        }
        else if (!next) {
            next = new Queue(t);
        }
        else {
            next->enqueue(t);
        }
      }
      Thread* dequeue() {
        if (!tObj)
            return NULL;
        if (next) {
            Thread* rmThrd = tObj;
            tObj = next->tObj;
            Queue* consumed = next;
            next = next->next;
            delete (void * ) consumed;
            return rmThrd; 
        }      
        Thread* tmp = tObj;
        tObj = NULL;
        return tmp;
      }
};
