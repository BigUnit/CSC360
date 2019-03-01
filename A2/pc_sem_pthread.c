/* Nathan Marcotte
 * CSC 360 Spring 2019
 * V00876934
 * pc_sem_pthread.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
int items = 0;

sem_t lock;
sem_t can_prod;
sem_t can_cons;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
    sem_wait(&can_prod);
    
        sem_wait(&lock);

            items++;
            histogram[items]++;
            assert(items<=MAX_ITEMS);

        sem_post(&lock);
    
    sem_post(&can_cons);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    // TODO
     sem_wait(&can_cons);
    
        sem_wait(&lock);

          items--;
          histogram[items]++;
          assert(items>=0);

        sem_post(&lock);
    
    sem_post(&can_prod);
  }
  return NULL;
}

int main (int argc, char** argv) {
  pthread_t t[4];


sem_init(&lock,0,1);
sem_init(&can_prod,0,MAX_ITEMS);
sem_init(&can_cons,0,0);


for(int i = 0;i<NUM_PRODUCERS;i++){
  pthread_create(&t[i],NULL,producer,NULL);
}

for(int i = NUM_PRODUCERS;i<NUM_PRODUCERS+NUM_CONSUMERS;i++){
  pthread_create(&t[i],NULL,consumer,NULL);
}

  
for(int i = 0;i<NUM_PRODUCERS+NUM_CONSUMERS;i++){
  pthread_join(t[i],0);
}
 
  // TODO: Create Threads and Join

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (pthread_t) * NUM_ITERATIONS);
}
