#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"
#include "kernel_threads.h"
#include "kernel_cc.h"
#include "kernel_streams.h"

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
    if(args!=NULL) {
    ptcb->args = malloc(argl);
    memcpy(ptcb->args, args, argl);
  }
  else{
    ptcb->args=NULL;
  }
  

  ptcb->exited = 0;
  ptcb->detached = 0;
  ptcb->refcount = 0;
  ptcb->exit_cv = COND_INIT;
  ptcb->exitval=CURPROC->exitval;
  rlnode* node = rlnode_init(& ptcb->ptcb_list_node, ptcb);

  

  if(task != NULL){
    /** Initialize a new tcb
     and make connection with ptcb*/
    ptcb->tcb = spawn_thread(CURPROC, start_thread);
    ptcb->tcb->ptcb = ptcb;
    
    /** pcb - ptcb connection (add ptcb to pcb's list)*/
    rlist_push_back(& CURPROC->ptcb_list, node);
    CURPROC->thread_count++;
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
  PTCB* ptcb = (PTCB*)tid;
  /** @brief Checks */
  /** 
  @brief Check if joinable
  */
  if (ptcb->detached == 1){
    goto finishError;
  }

  /** 
  @brief Check if it joins itself
  */
  if(tid == (Tid_t) CURTHREAD->ptcb){
    goto finishError;
  }

  /** 
  @brief Check if T2 belongs to the same PCB
  */
  if(rlist_find(& CURPROC->ptcb_list, ptcb, NULL) == NULL){
    goto finishError;
  }


  /** 
  @brief Check if it is exited
  */
  if(ptcb->exited == 1){
    goto finishError;
  }


  /** 
  @brief After the checks
  */
  ptcb->refcount++;
  while(ptcb->exited != 1 && ptcb->detached != 1){
    kernel_wait(& ptcb->exit_cv,SCHED_USER);
  }

  ptcb->refcount--;
  if(ptcb->exited == 1 || ptcb->detached ==1){
    goto finishError;
  }

  goto finishNormal;
  
    
  



  finishNormal:
    rlist_remove(& ptcb->ptcb_list_node);
    free(ptcb);
    return 0;  
  

  finishError:
    return -1;
  
	
}






/**
  @brief Detach the given thread.
  */
int sys_ThreadDetach(Tid_t tid)
{
  PTCB* ptcb = (PTCB*)tid;
   /** 
  @brief Check if thread belongs to pcb
  */
  if(rlist_find(& CURPROC->ptcb_list, ptcb, NULL) == NULL){
    goto finishError;
  }

  /** 
  @brief Check if it is exited
  */
  if(ptcb->exited == 1){
    goto finishError;
  }

  ptcb->detached = 1;
  kernel_broadcast(& ptcb->exit_cv);
  ptcb->refcount = 0;
  goto finishNormal;



	finishNormal:
    return 0;  
  

  finishError:
    return -1;
}



/**
  @brief Terminate the current thread.
  */
void sys_ThreadExit(int exitval)
{

  PCB *curproc = CURPROC;  /* cache for efficiency */
  PTCB *current_thread = CURTHREAD->ptcb;
  
   /** after check for last thread terminate ptcb also */
  curproc->main_thread--;
  current_thread->exited = 1;
  current_thread->exitval = exitval;
  kernel_broadcast(& current_thread->exit_cv);
  
  

  /** check if last thread */
  if(curproc->thread_count == 0){
  

  /* First, store the exit status */
  

  if(get_pid(curproc)!=1){
  /* Reparent any children of the exiting process to the 
      initial task */
  PCB* initpcb = get_pcb(1);
  while(!is_rlist_empty(& curproc->children_list)) {
    rlnode* child = rlist_pop_front(& curproc->children_list);
    child->pcb->parent = initpcb;
    rlist_push_front(& initpcb->children_list, child);
  }

  /* Add exited children to the initial task's exited list 
      and signal the initial task */
  if(!is_rlist_empty(& curproc->exited_list)) {
    rlist_append(& initpcb->exited_list, &curproc->exited_list);
    kernel_broadcast(& initpcb->child_exit);
  }

  /* Put me into my parent's exited list */
  rlist_push_front(& curproc->parent->exited_list, &curproc->exited_node);
  kernel_broadcast(& curproc->parent->child_exit);
  }
  

  assert(is_rlist_empty(& curproc->children_list));
  assert(is_rlist_empty(& curproc->exited_list));
  

  /* 
    Do all the other cleanup we want here, close files etc. 
   */

  /* Release the args data */
  if(curproc->args) {
    free(curproc->args);
    curproc->args = NULL;
  }

  /* Clean up FIDT */
  for(int i=0;i<MAX_FILEID;i++) {
    if(curproc->FIDT[i] != NULL) {
      FCB_decref(curproc->FIDT[i]);
      curproc->FIDT[i] = NULL;
    }
  }

  /* Disconnect my main_thread */
  curproc->main_thread = NULL;

  /* Now, mark the process as exited. */
  curproc->pstate = ZOMBIE;
  
}
  

 
/* Bye-bye cruel world */
  kernel_sleep(EXITED, SCHED_USER);

  
}



