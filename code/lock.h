#ifndef LOCK_HEADER_FILE_H
#define LOCK_HEADER_FILE_H

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

struct spinlock {
	atomic_flag v;
};

#define SPINLOCK_INIT                 \
	{                             \
		.v = ATOMIC_FLAG_INIT \
	}

/* static inline void sl_init(struct spinlock *l) */
/* { */
/* 	*l = (struct spinlock)SPINLOCK_INIT; */
/* } */

/* static inline void sl_lock(struct spinlock *l) */
/* { */
/* 	while (atomic_flag_test_and_set_explicit(&l->v, memory_order_acquire)) { */
/* 		/\* do nothing *\/ */
/* 	} */
/* } */

/* static inline void sl_unlock(struct spinlock *l) */
/* { */
/* 	atomic_flag_clear_explicit(&l->v, memory_order_release); */
/* } */

void* sl_new() {
  struct spinlock *l = malloc(sizeof(struct spinlock));
  *l = (struct spinlock)SPINLOCK_INIT;
  return l;
}

void sl_lock(struct spinlock *l) {
  while (atomic_flag_test_and_set_explicit(&l->v, memory_order_acquire));
}

void sl_unlock(struct spinlock *l) {
  atomic_flag_clear_explicit(&l->v, memory_order_release);
}

#endif
