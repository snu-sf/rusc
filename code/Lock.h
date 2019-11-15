#ifndef LOCK_HEADER_FILE_H
#define LOCK_HEADER_FILE_H

#include <stdatomic.h>

typedef struct spinlock {
	atomic_flag v;
} spinlock_t;

#define SPINLOCK_INIT                 \
	{                             \
		.v = ATOMIC_FLAG_INIT \
	}

extern spinlock_t * Lock_new();
extern void Lock_lock(spinlock_t *l);
extern void Lock_unlock(spinlock_t *l);

#endif
