/* Nathan Marcotte
 * CSC 360 Spring 2019
 * V00876934
 * pc_spinlock_uthread.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

spinlock_t lock;
spinlock_t prod_lock;
spinlock_t cons_lock;
int items = 0;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    while(1){

      while(items>=MAX_ITEMS){
        spinlock_lock(&prod_lock);
        producer_wait_count++;
        spinlock_unlock(&prod_lock);
      }

      spinlock_lock(&lock);

      if(items >= MAX_ITEMS){
        spinlock_unlock(&lock);
      } else {
          break;
      }


    }

    items++;
    histogram[items]++;
    assert(items<=MAX_ITEMS);
    spinlock_unlock(&lock);

  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
       while(1){

      while(items<=0){
        spinlock_lock(&cons_lock);
        consumer_wait_count++;
        spinlock_unlock(&cons_lock);
      }

      spinlock_lock(&lock);

      if(items <= 0){
        spinlock_unlock(&lock);
      } else {
          break;
      }


    }

    items--;
    histogram[items]++;
    assert(items>=0);
    spinlock_unlock(&lock);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];
  uthread_init (4);
  spinlock_create(&lock);
  spinlock_create(&prod_lock);
  spinlock_create(&cons_lock);
  // TODO: Create Threads and Join

for(int i = 0;i<NUM_PRODUCERS;i++){
  t[i]=uthread_create(producer,NULL);
}

for(int i = NUM_PRODUCERS;i<NUM_PRODUCERS+NUM_CONSUMERS;i++){
  t[i]=uthread_create(consumer,NULL);
}

  
for(int i = 0;i<NUM_PRODUCERS+NUM_CONSUMERS;i++){
  uthread_join(t[i],0);
}
 
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
