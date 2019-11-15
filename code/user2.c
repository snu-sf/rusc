#include <stdio.h>
#include "map.h"
#include "lock.h"
#include "user1.h"
#include "user2.h"

static void* G_memoized;
static void* G_lock;

void G_init() {
  G_lock = Lock_new();
  G_memoized = Map_new();
}

int G_sum(int x) {
  /* printf("[g] x = %d\n", x); */
  int t_i, t_v;
  if(x == 0) return 0;
  Lock_lock(G_lock);
  t_i = Map_find(G_memoized, 0);
  /* printf("[g] x, t_i: %d, %d\n", x, t_i); */
  if(x == t_i) {
    t_v = Map_find(G_memoized, 1);
    /* printf("[g] t_i, t_v: %d, %d\n", t_i, t_v); */
    Lock_unlock(G_lock);
  }
  else {
    Lock_unlock(G_lock);
    t_v = F_sum(x-1) + x;
    Lock_lock(G_lock);
    /* printf("[g] x, t_v: %d, %d\n", x, t_v); */
    Map_insert(G_memoized, 0, x);
    Map_insert(G_memoized, 1, t_v);
    Lock_unlock(G_lock);
  }
  return t_v;
}
