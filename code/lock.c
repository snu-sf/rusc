#include <stdio.h>
#include <stdlib.h>
#include "lock.h"

spinlock_t* Lock_new() {
  spinlock_t *l = malloc(sizeof(spinlock_t));
  if (l) {
    *l = (spinlock_t)SPINLOCK_INIT;
  }
  return l;
}

void Lock_lock(spinlock_t *l) {
  while (atomic_flag_test_and_set_explicit(&l->v, memory_order_acquire));
}

void Lock_unlock(spinlock_t *l) {
  atomic_flag_clear_explicit(&l->v, memory_order_release);
}
