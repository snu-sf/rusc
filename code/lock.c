#include <stdio.h>
#include <stdlib.h>
#include "lock.h"

void* Lock_new() {
  struct spinlock *l = malloc(sizeof(struct spinlock));
  if (l) {
    *l = (struct spinlock)SPINLOCK_INIT;
  }
  return l;
}

void Lock_lock(struct spinlock *l) {
  while (atomic_flag_test_and_set_explicit(&l->v, memory_order_acquire));
}

void Lock_unlock(struct spinlock *l) {
  atomic_flag_clear_explicit(&l->v, memory_order_release);
}
