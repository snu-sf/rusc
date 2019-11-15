#ifndef LOCK_HEADER_FILE_H
#define LOCK_HEADER_FILE_H

#include <stdatomic.h>

struct spinlock {
	atomic_flag v;
};

#define SPINLOCK_INIT                 \
	{                             \
		.v = ATOMIC_FLAG_INIT \
	}

extern void* Lock_new();
extern void Lock_lock(struct spinlock *l);
extern void Lock_unlock(struct spinlock *l);

#endif
