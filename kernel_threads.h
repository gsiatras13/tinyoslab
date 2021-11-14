#ifndef __KERNEL_THREADS_H
#define __KERNEL_THREADS_H

/**
  
*/ 

#include "tinyos.h"
#include "kernel_sched.h"
#include "util.h"





typedef struct process_thread_control_block {
  TCB* tcb;                      /** Connected tcb */


  Task task;                     /** Task to be executed */
  int argl;                      
  void* args;

  int exitval;

  int exited;                    /** Flag (1 = exited) */
  int detached;                  
  CondVar exit_cv;

  int refcount;                  /** Flag to know when to erase ptcb */

  rlnode ptcb_list_node;
                             




} PTCB;


#endif
