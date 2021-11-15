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
  @brief Create a new thread in the current process.
  */
Tid_t sys_CreateThread(Task task, int argl, void* args)
{

  /** Acquire ptcb(malloc?) and itialize it */
  PTCB* ptcb = (PTCB*)xmalloc(sizeof(PTCB));
  


  ptcb->task = task;
  ptcb->argl = argl;
  ptcb->args= args;
  

  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->refcount = 0;
  ptcb->exit_cv = COND_INIT;


  

  if(task != NULL){
    /** Initialize a new tcb
     and make connection with ptcb*/

    ptcb->tcb = spawn_thread(CURPROC, start_thread);
    ptcb->tcb->ptcb = ptcb;
    
    /** pcb - ptcb connection (add ptcb to pcb's list)
     */
    rlnode* node = rlnode_init(& ptcb->ptcb_list_node, ptcb);
    rlist_push_back(& CURPROC->ptcb_list, node);
    pcb->thread_count++;
    ptcb->refcount = 1;
    /** Wake up tcb (add to sched) */
    wakeup(ptcb->tcb);
    return (Tid_t) ptcb;

}


	else return NOTHREAD;
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



