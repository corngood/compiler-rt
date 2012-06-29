//===-- sanitizer_atomic.h --------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef SANITIZER_ATOMIC_H
#define SANITIZER_ATOMIC_H

#include "sanitizer_internal_defs.h"

namespace __sanitizer {

enum memory_order {
  memory_order_relaxed = 1 << 0,
  memory_order_consume = 1 << 1,
  memory_order_acquire = 1 << 2,
  memory_order_release = 1 << 3,
  memory_order_acq_rel = 1 << 4,
  memory_order_seq_cst = 1 << 5,
};

struct atomic_uint8_t {
  typedef u8 Type;
  volatile Type val_dont_use;
};

struct atomic_uint16_t {
  typedef u16 Type;
  volatile Type val_dont_use;
};

struct atomic_uint32_t {
  typedef u32 Type;
  volatile Type val_dont_use;
};

struct atomic_uint64_t {
  typedef u64 Type;
  volatile Type val_dont_use;
};

struct atomic_uintptr_t {
  typedef uptr Type;
  volatile Type val_dont_use;
};

INLINE void atomic_signal_fence(memory_order) {
  __asm__ __volatile__("" ::: "memory");
}

INLINE void atomic_thread_fence(memory_order) {
  __sync_synchronize();
}

INLINE void proc_yield(int cnt) {
  __asm__ __volatile__("" ::: "memory");
#if defined(__i386__) || defined(__x86_64__)
  for (int i = 0; i < cnt; i++)
    __asm__ __volatile__("pause");
#endif
  __asm__ __volatile__("" ::: "memory");
}

template<typename T>
INLINE typename T::Type atomic_load(
    const volatile T *a, memory_order mo) {
  DCHECK(mo & (memory_order_relaxed | memory_order_consume
      | memory_order_acquire | memory_order_seq_cst));
  DCHECK(!((uptr)a % sizeof(*a)));
  typename T::Type v;
  if (mo == memory_order_relaxed) {
    v = a->val_dont_use;
  } else {
    atomic_signal_fence(memory_order_seq_cst);
    v = a->val_dont_use;
    atomic_signal_fence(memory_order_seq_cst);
  }
  return v;
}

template<typename T>
INLINE void atomic_store(volatile T *a, typename T::Type v, memory_order mo) {
  DCHECK(mo & (memory_order_relaxed | memory_order_release
      | memory_order_seq_cst));
  DCHECK(!((uptr)a % sizeof(*a)));
  if (mo == memory_order_relaxed) {
    a->val_dont_use = v;
  } else {
    atomic_signal_fence(memory_order_seq_cst);
    a->val_dont_use = v;
    atomic_signal_fence(memory_order_seq_cst);
  }
  if (mo == memory_order_seq_cst)
    atomic_thread_fence(memory_order_seq_cst);
}

template<typename T>
INLINE typename T::Type atomic_fetch_add(volatile T *a,
    typename T::Type v, memory_order mo) {
  (void)mo;
  DCHECK(!((uptr)a % sizeof(*a)));
  return __sync_fetch_and_add(&a->val_dont_use, v);
}

template<typename T>
INLINE typename T::Type atomic_fetch_sub(volatile T *a,
    typename T::Type v, memory_order mo) {
  (void)mo;
  DCHECK(!((uptr)a % sizeof(*a)));
  return __sync_fetch_and_add(&a->val_dont_use, -v);
}

template<typename T>
INLINE typename T::Type atomic_exchange(volatile T *a,
    typename T::Type v, memory_order mo) {
  DCHECK(!((uptr)a % sizeof(*a)));
  if (mo & (memory_order_release | memory_order_acq_rel | memory_order_seq_cst))
    __sync_synchronize();
  v = __sync_lock_test_and_set(&a->val_dont_use, v);
  if (mo == memory_order_seq_cst)
    __sync_synchronize();
  return v;
}

template<typename T>
INLINE bool atomic_compare_exchange_strong(volatile T *a,
                                           typename T::Type *cmp,
                                           typename T::Type xchg,
                                           memory_order mo) {
  typedef typename T::Type Type;
  Type cmpv = *cmp;
  Type prev = __sync_val_compare_and_swap(&a->val_dont_use, cmpv, xchg);
  if (prev == cmpv)
    return true;
  *cmp = prev;
  return false;
}

template<typename T>
INLINE bool atomic_compare_exchange_weak(volatile T *a,
                                           typename T::Type *cmp,
                                           typename T::Type xchg,
                                           memory_order mo) {
  return atomic_compare_exchange_strong(a, cmp, xchg, mo);
}

}  // namespace __sanitizer

#endif  // SANITIZER_ATOMIC_H