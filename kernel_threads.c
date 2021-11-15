#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_threads.h"

#define CURCORE (cctx[cpu_core_id])
#define CURTHREAD (CURCORE.current_thread)




void start_thread(){
  int exitval;

  Task call =  CURTHREAD->ptcb->task;
  int argl = CURTHREAD->ptcb->argl;
  void* args = CURTHREAD->ptcb->args;

  exitval = call(argl,args);
  ThreadExit(exitval);
}


/** 
  @brief test commit.
  */


/** 
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{
  

   

  /** Acquire ptcb(malloc?) and connections */
  PTCB* ptcb = (PTCB*)xmalloc(sizeof(PTCB));
  rlnode_init(&ptcb->ptcb_list_node, ptcb); 

  /* Set the main thread's function */
  ptcb->task = task;
  /* Copy the arguments to new storage, owned by the new process */
  ptcb->argl = argl;
  if(args!=NULL) {
    ptcb->args = malloc(argl);
    memcpy(ptcb->args, args, argl);
  }
  else{
    ptcb->args=NULL;
  }

  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->exit_cv = COND_INIT;

  

  if(task != NULL){
    /** Initialize a new tcb*/
    ptcb->tcb = spawn_thread(CURPROC, ptcb, start_thread);
    /* PCB* pcb = ptcb->tcb->owner_pcb;
       rlnode* node = rlnode_init(& ptcb->ptcb_list_node, ptcb);
       rlist_push_front(& pcb->ptcb_list, node);
       ptcb->refcount++;
       pcb->thread_count++;
    */s
    ptcb->refcount = 1;
    /** Wake up tcb (add to sched) */
    wakeup(ptcb->tcb);
}


	return ptcb;
}



/**
  @brief Return the Tid of the current thread.
 */
Tid_t sys_ThreadSelf()
{
	return (Tid_t) cur_thread()->ptcb;
}

/**
  @brief Join the given thread.
  */
int sys_ThreadJoin(Tid_t tid, int* exitval)
{
  /** 
  @brief Check if joinable
  */

  /** 
  @brief Use kernel_wait()

  */

  

	return -1;
}

/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
	return -1;
}

/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{

}



