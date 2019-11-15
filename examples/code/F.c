#include <stdio.h>
#include "Map.h"
#include "Lock.h"
#include "F.h"
#include "G.h"

static void* F_lock;
static void* F_memoized;

bool F_init() {
  F_lock = Lock_new();
  if (!F_lock) {
    return false;
  }
  F_memoized = Map_new();
  if (!F_memoized) {
    return false;
  }
  return true;
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
