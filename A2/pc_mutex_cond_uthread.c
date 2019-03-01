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

//spinlock_t lock;
uthread_mutex_t mutex; 
uthread_cond_t max;
uthread_cond_t none;

int items = 0;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
   
    uthread_mutex_lock(mutex);
     
      while(items>=MAX_ITEMS){
        producer_wait_count++;
        uthread_cond_wait(none);
      }

      
    assert(items<=MAX_ITEMS);
      

    items++;
    histogram[items]++;
    
    uthread_cond_signal(max);
    uthread_mutex_unlock(mutex);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
       
      uthread_mutex_lock(mutex);
      
      while(items<=0){
        consumer_wait_count++;
        uthread_cond_wait(max);
      }
   
      assert(items>=0);
      items--;
      histogram[items]++;
   
     uthread_cond_signal(none);
     uthread_mutex_unlock(mutex);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_t t[4];
  uthread_init (4);
  mutex = uthread_mutex_create();
  max =  uthread_cond_create(mutex);
  none =  uthread_cond_create(mutex);
  //spinlock_create(&lock);
 
  
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
 
 uthread_mutex_destroy(mutex);
 uthread_cond_destroy(max);
 uthread_cond_destroy(none);
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
