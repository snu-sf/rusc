HW o Lock_Impl <= HW o Lock_Spec //TODO: 여기서도 page table에 대한 assume! 넣어야 할 듯

HW o Lock_Spec o Mpool_Impl <= HW o Lock_Spec o Mpool_Spec

HW o Mpool_Spec o API_Impl <= HW o Mpool_Spec o API_Spec

...

HW o Mpool_Spec o API_Spec o ... <= HW o HV_Spec

HW o HV_Spec o VM1_Impl o VM2_Impl <= HW o Top_Spec

HW o Top_Spec <= HW o Top_Spec'

(HW module exists everywhere. HW module's functions are like inline functions)



Top_Spec: erase UB

Top_Spec': introduce NB

.

.

Global Data: Mem, current_vm, is_hv

current_vm is either 1 or 2. It is changed directly in interaction semantics (interleaving run1, run2)

is_hv is either true or false. It is changed directly in interaction semantics (hvcall, return)

```
After initialization
[000, 100) -> HV stack, mailbox, etc...
[100, 200) -> permission table (page table in Hafnium). mpool manages it
  [   ,    ) -> (is_valid, from,   to, hv_or_vm_id)
  [100, 110) -> (    true,    0,  200,           0)
  [110, 120) -> (    true,  200,  300,           1)
  [120, 130) -> (    true,  300,  400,           2)
[200, 300) -> VM1
[300, 400) -> VM2
```

Macroes
```
assume!(cond)    := if(!cond) UndefinedBehavior
guarantee!(cond) := if(!cond) NoBehavior
/* below operations use HW.hw_load, HW.hw_store so that permissions are checked */
/* if HW.hw_load or HW.hw_store fails, it gives UB */
/* NOTE: its implementation is not locked at all -- [** YIELD **] is everywhere */
read_entry!(page: i64): Option (from: i64, to: i64, hv_or_vm_id: i8) :=  ...
  /* NOTE: first checks validity bit */
write_entry!(page: i64, (from: i64, to: i64, hv_or_vm_id: i8)) := ...
  /* NOTE: it writes is_valid bit at last */
invalidate_entry!(page: i64) := ...
read_entry_hardware!(page: i64) := same as read_entry!, but access Mem directly instead of using HW.hw_load.
  /* NOTE: if we use read_entry!, it causes infinite loop */
```

(HW)
//These operations are atomic. (i.e. no [** YIELD **])
```Coq
Module HW {
  fun hw_load(addr) : option memval {
    if(check_permission(addr)) {
      return Some(Mem[addr])
    }
    else {
      return None
    }
  }

  fun hw_store(addr, memval) : bool {
    if(check_permission(addr)) {
      Mem[addr] = val;
      return true
    }
    else {
      return false
    }
  }

  priv fun check_permission(addr) : bool {
    if (is_hv) 
      current_hv_or_vm = 0
    else
      current_hv_or_vm = current_vm
    for(int i=0; i<10; i++) {
      match read_entry_hardware!(100 + 10*i) {
        Some(from, to, current_hv_or_vm, _) => {
          if(addr ∈ [from, to)) return true;
        }
        None => _
      }
    }
    return API.fault_handler(addr)
  }
}
```


(Mpool_Spec)
//Singleton object
//Its implementation uses lock, so it has no [** YIELD **]
```Coq
Module Mpool {
  pages: { set: Set int64 | range: forall elem in set, 100 <= elem < 200 && elem % 10 == 0 }

  fun alloc_page() : int64 {
    match pages.get() with 
    | Some(page) => return page
    | None => return NULL
    end
  }

  fun free_page(page: int64) {
    assume!(100 <= page < 200 && page % 10 == 0);
    pages.put(page)
  }
}
```


(API_Impl)
```Coq
//Q: Is API/memory access(HW) thread-safe?
// [** YIELD **]s are omitted, but it exists in every line.
// lock()/unlock()s are omitted, but all the methods are fully locked.

Module API {
  l := Lock.new()

  priv fun current_vm_is_owner(from: int64, to: int64) : bool {
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i) with {
        Some(from', to', current_vm) => {
          if([from, to) ⊆ [from', to')) return true
        }
        _ => _
      }
    }
    return false
  }
  
  priv fun current_vm_is_exclusive_owner(from: int64, to: int64) : bool {
    if(!current_vm_is_owner(from, to)) return false;
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i) with {
        Some(from', to', vm_id) => {
          if([from, to) ∩ [from', to') /\ vm_id != current_vm) return false
        }
        _ => _
      }
    }
    return true
  }
  
  fun share_memory(from: int64, to: int64, vm_id: int8) : int64 {
    if(!current_vm_is_owner(from, to)) return -1
    let new_page = Mpool.alloc_page()
    if(new_page == NULL) return -1
    write_entry!(new_page, (from, to, vm_id))
    return 0
  }
  
  fun give_memory(from: int64, to: int64, vm_id: int8) : int64 {
    if(!current_vm_is_exclusive_owner(from, to)) return -1
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i) with {
        Some(from, to, current_vm) => {
          invalidate_entry!(100 + 10*i)
          write_entry!(100 + 10*i, (from, to, vm_id))
          return 0
        }
        _ => _
      }
    }
    return -1
  }
  
  fun fault_handler(addr) : bool {
    has_permission = /* same logic as HW.check_permission(addr) */
    if(has_permission) {
      //spurious fault caused by update
      return true
    }
    else {
      //real page fault
      //TODO: do something
      return false
    }
  }

  ...
}
```


(VM1_Impl)
```Coq
Module VM1 {
  LS: Type
  local_state1: LS
  fun run() : (Mem * local_state) -> (Mem * local_state) -> Prop := 
    (pure_operation|HW.hw_load|HW.hw_store|API.share_memory|API.give_memory)*
  //pure_operation changes local_state (which is register value physically)
}
```


(VM2_Impl)
```Coq
Module VM2 {
  ... ditto ...
}
```
