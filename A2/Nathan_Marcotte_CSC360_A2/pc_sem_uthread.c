/* Nathan Marcotte
 * CSC 360 Spring 2019
 * V00876934
 * pc_sem_uthread.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
int items = 0;

uthread_sem_t lock;
uthread_sem_t can_prod;
uthread_sem_t can_cons;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    uthread_sem_wait(can_prod); //wait until it can produce
    
        uthread_sem_wait(lock); //lock 

            items++;
            histogram[items]++;
            assert(items<=MAX_ITEMS);

        uthread_sem_signal(lock); // open lock
    
    uthread_sem_signal(can_cons); // signals that consumer can consume
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
     uthread_sem_wait(can_cons); // wait until given signal to consume
    
        uthread_sem_wait(lock); // lock

          items--;
          histogram[items]++;
          assert(items>=0);

        uthread_sem_signal(lock); // open lock
    
    uthread_sem_signal(can_prod); // send signal that it can produce
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];
  uthread_init (4);

lock = uthread_sem_create(1);
can_prod = uthread_sem_create(MAX_ITEMS);
can_cons = uthread_sem_create(0);


for(int i = 0;i<NUM_PRODUCERS;i++){
  t[i]=uthread_create(producer,NULL);
}

for(int i = NUM_PRODUCERS;i<NUM_PRODUCERS+NUM_CONSUMERS;i++){
  t[i]=uthread_create(consumer,NULL);
}

  
for(int i = 0;i<NUM_PRODUCERS+NUM_CONSUMERS;i++){
  uthread_join(t[i],0);
}
 
  // TODO: Create Threads and Join

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
