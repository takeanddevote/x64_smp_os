#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

typedef int spin_lock_handle;

#define SPIN_LOCK(name) \
    spin_lock_handle name = 0

void spin_lock_init(spin_lock_handle *handle);
void spin_lock_destroy(spin_lock_handle *handle);
void spin_lock(spin_lock_handle *handle);
void spin_unlock(spin_lock_handle *handle);

#endif /* __SPINLOCK_H__ */
