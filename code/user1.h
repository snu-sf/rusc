#include "map.h"
#include "lock.h"

extern int G_sum(int x);

void* F_memoized;
void* F_lock;

void F_init() {
  F_lock = Lock_new();
  F_memoized = Map_new();
}

int F_sum(int x) {
  /* printf("[f] x = %d\n", x); */
  int t;
  if(x == 0) return 0;
  Lock_lock(F_lock);
  t = Map_find(F_memoized, x);
  Lock_unlock(F_lock);
  if(t == -1) {
    t = G_sum(x-1) + x;
    Lock_lock(F_lock);
    Map_insert(F_memoized, x, t);
    Lock_unlock(F_lock);
  }
  return t;
}
