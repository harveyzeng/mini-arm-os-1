#ifndef __THREADS_H__
#define __THREADS_H__

void thread_start();
int thread_create(void (*run)(void*), void* userdata,int priority,char* name);
void thread_kill(int thread_id);
void thread_self_terminal();
void ps_function();
#endif
