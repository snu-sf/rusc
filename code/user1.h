#include "map.h"
#include "lock.h"

extern int g(int x);

static struct node** memoized_f;
static struct spinlock* sl;

void init_f() {
  memoized_f = new();
  sl = sl_new();
}

int f(int x) {
  /* printf("[f] x = %d\n", x); */
  int t;
  if(x == 0) return 0;
  sl_lock(sl);
  t = find(memoized_f, x);
  sl_unlock(sl);
  if(t == -1) {
    t = g(x-1) + x;
    sl_lock(sl);
    insert(memoized_f, x, t);
    sl_unlock(sl);
  }
  return t;
}
