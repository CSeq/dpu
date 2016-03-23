/* Adapted from: https://svn.sosy-lab.org/software/sv-benchmarks/trunk/c/pthread-atomic/dekker_true-unreach-call.c */

#include <pthread.h>
#include <assert.h>

volatile int flag1 = 0; 
volatile int flag2 = 0; // boolean flags
volatile int turn = 0; // integer variable to hold the ID of the thread whose turn it is
volatile int x; // boolean variable to test mutual exclusion

void *thr1() {
  flag1 = 1;
  while (flag2 >= 1) {
    if (turn != 0) {
      flag1 = 0;
      busy_1: if (turn != 0) { goto busy_1; }
      flag1 = 1;
    }
  }
  x = 0;
  if(x>0){
       assert(0); //  assert(x<=0);
  }
  turn = 1;
  flag1 = 0;
  //  return 0;
}

void *thr2() {
  flag2 = 1;
  while (flag1 >= 1) {
    if (turn != 1) {
      flag2 = 0;
      busy_2: if (turn != 1) { goto busy_2; } //while (turn != 0) {};
      flag2 = 1;
    }
  }

  x = 1;
  if(x<1){
      assert(0); //assert(x>=1);
  }
  
  turn = 0;
  flag2 = 0;
}

int main() {

  pthread_t t1;
  pthread_t t2;

  pthread_create(&t1, 0, thr1, 0);
  pthread_create(&t2, 0, thr2, 0);

  pthread_join(t1, 0);
  pthread_join(t2, 0);

}
