/* Nathan Marcotte
 * CSC 360 Spring 2019
 * V00876934
 * smoke_pthread.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
//#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000

#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

pthread_cond_t bac_mat;
pthread_cond_t pap_mat;
pthread_cond_t bac_pap;
int sum = 0;

struct Agent {
  pthread_mutex_t mutex;
  pthread_cond_t  match;
  pthread_cond_t  paper;
  pthread_cond_t  tobacco;
  pthread_cond_t  smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  pthread_mutex_init(&agent->mutex,NULL);
  pthread_cond_init (&agent->paper,NULL);
  pthread_cond_init (&agent->match,NULL);
  pthread_cond_init (&agent->tobacco,NULL);
  pthread_cond_init (&agent->smoke,NULL);
  return agent;
}

//
// TODO
// You will probably need to add some procedures and struct etc.
//

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};
  
  pthread_mutex_lock (&a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        pthread_cond_signal (&a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        pthread_cond_signal (&a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        pthread_cond_signal (&a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      pthread_cond_wait (&a->smoke,&a->mutex);
    }
  pthread_mutex_unlock (&a->mutex);
  return NULL;
}

void* get_smoker(int value){
  switch(value){
    case TOBACCO + PAPER:
      VERBOSE_PRINT("Get Match");
      pthread_cond_signal(&bac_pap);
      sum = 0;
      break;
  
    case TOBACCO + MATCH:
      VERBOSE_PRINT("Get Paper");
      pthread_cond_signal(&bac_mat);
      sum = 0;
      break;
  
    case MATCH + PAPER:
      VERBOSE_PRINT("Get Tobacco");
      pthread_cond_signal(&pap_mat);
      sum = 0;
      break;

    default :
      break;
  }
}

void* handle_bac(void* agent){
  struct Agent* ag = agent;
  
  pthread_mutex_lock(&ag->mutex);

    while(1){
      pthread_cond_wait(&ag->tobacco,&ag->mutex);
      sum+=TOBACCO;
      get_smoker(sum);
    }

  pthread_mutex_unlock(&ag->mutex);

}
void* handle_pap(void* agent){
    struct Agent* ag = agent;
  
  pthread_mutex_lock(&ag->mutex);

    while(1){
      pthread_cond_wait(&ag->paper,&ag->mutex);
      sum+=PAPER;
      get_smoker(sum);
    }

  pthread_mutex_unlock(&ag->mutex);
}
void* handle_mat(void* agent){
    struct Agent* ag = agent;
  
  pthread_mutex_lock(&ag->mutex);

    while(1){
      pthread_cond_wait(&ag->match,&ag->mutex);
      sum+=MATCH;
      get_smoker(sum);
    }

  pthread_mutex_unlock(&ag->mutex);
}

void* smoke_bac(void* agent){
  struct Agent* ag = agent;
  pthread_mutex_lock(&ag->mutex);

    while(1){
      pthread_cond_wait(&pap_mat,&ag->mutex);
      VERBOSE_PRINT("Tobacco Smoking");
      pthread_cond_signal(&ag->smoke);
      smoke_count[TOBACCO]++;
    }

  pthread_mutex_unlock(&ag->mutex);
}
void* smoke_pap(void* agent){
  struct Agent* ag = agent;
  pthread_mutex_lock(&ag->mutex);

    while(1){
      pthread_cond_wait(&bac_mat,&ag->mutex);
      VERBOSE_PRINT("Paper Smoking");
      pthread_cond_signal(&ag->smoke);
      smoke_count[PAPER]++;
    }

  pthread_mutex_unlock(&ag->mutex);
}
void* smoke_mat(void* agent){
  struct Agent* ag = agent;
  pthread_mutex_lock(&ag->mutex);

    while(1){
      pthread_cond_wait(&bac_pap,&ag->mutex);
      VERBOSE_PRINT("Match Smoking");
      pthread_cond_signal(&ag->smoke);
      smoke_count[MATCH]++;
    }

  pthread_mutex_unlock(&ag->mutex);
}

int main (int argc, char** argv) {
  struct Agent*  a = createAgent();
  
  pthread_cond_init(&bac_mat,NULL);
  pthread_cond_init(&pap_mat,NULL);
  pthread_cond_init(&bac_pap,NULL);

  // TODO
  pthread_t ag,hb,hp,hm,sb,sp,sm;

  pthread_create(&hb,NULL,handle_bac,a);
  pthread_create(&hp,NULL,handle_pap,a);
  pthread_create(&hm,NULL,handle_mat,a);
  pthread_create(&sb,NULL,smoke_bac,a);
  pthread_create(&sp,NULL,smoke_pap,a);
  pthread_create(&sm,NULL,smoke_mat,a);
  
    pthread_create(&ag,NULL,agent,a);

  pthread_join (ag, 0);
  assert (signal_count [MATCH]   == smoke_count [MATCH]);
  assert (signal_count [PAPER]   == smoke_count [PAPER]);
  assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);
  printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
}