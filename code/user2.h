#include "map.h"
#include "lock.h"

extern int f(int x);

static struct node* memoized_g;
static struct spinlock* sl;

void init_g() {
  memoized_g = new();
  sl = sl_new();
}

int g(int x) {
  /* printf("[g] x = %d\n", x); */
  int t_i, t_v;
  if(x == 0) return 0;
  sl_lock(sl);
  t_i = find(memoized_g, 0);
  /* printf("[g] x, t_i: %d, %d\n", x, t_i); */
  if(x == t_i) {
    t_v = find(memoized_g, 1);
    /* printf("[g] t_i, t_v: %d, %d\n", t_i, t_v); */
    sl_unlock(sl);
  }
  else {
    sl_unlock(sl);
    t_v = f(x-1) + x;
    sl_lock(sl);
    /* printf("[g] x, t_v: %d, %d\n", x, t_v); */
    insert(&memoized_g, 0, x);
    insert(&memoized_g, 1, t_v);
    sl_unlock(sl);
  }
  return t_v;
}
