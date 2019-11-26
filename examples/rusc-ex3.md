Lock_Impl <= Lock_Spec
Lock_Spec o Mpool_Impl <= Lock_Spec o Mpool_Spec
HW o Mpool_Spec o HVC_Impl <= HW o Mpool_Spec o HVC_Spec

HW + HV_Impl <= HW + HV_Spec
HW + HV_Spec + VM1_Impl + VM_Impl2 <= HW + Top_Spec
HW + Top_Spec <= HW + Top'_Spec
(HW module exists everywhere. HW module's functions are like inline functions)

Top_Spec: erase UB
Top'_Spec: introduce NOB



Global Data: Mem, current_vm, is_hv

current_vm is either 1 or 2. It is changed directly in interaction semantics (interleaving run1, run2)
is_hv is either true or false. It is changed directly in interaction semantics (hvcall, return)

//hypervisor id = 0, hvcall 때 current_vm = 0으로 세팅하면 --> hvcall 한 vm 이 자기 id 도 넘겨야 함.
//그러면 그 데이터를 올바르게 넘겼는지 알 수 없음 (vm1이 vm2인 척 하고 share_memory call)
//정리하면, current_vm (레지스터에 있던 인자로 넘기던) 데이터를 vm이 만질 수 있으면 안됨.

/*
Mem[0] represents owner of page [000, 100).
Mem[1] represents owner of page [100, 200).
Mem[2] represents owner of page [200, 300).

Mem[0] = 0x001 (HV owns it)
Mem[1] = 0x010 (VM1 owns it)
Mem[2] = 0x100 (VM2 owns it)
*/

After initialization
Accesslevel = OWN | ACCESSIBLE
[000, 100) -> HV stack, mailbox, etc...
[100, 200) -> permission table (page table in Hafnium). mpool manages it
  [   ,    ) -> (is_valid, from,   to, hv_or_vm_id, AccessLevel)
  [100, 110) -> (    true,    0,  200,           0,         OWN)
  [110, 120) -> (    true,  200,  300,           1,         OWN)
  [120, 130) -> (    true,  300,  400,           2,         OWN)
[200, 300) -> VM1
[300, 400) -> VM2

Macroes
assume!(cond)    := if(!cond) UndefinedBehavior
guarantee!(cond) := if(!cond) NoBehavior
/* below operations use HW.hw_load, HW.hw_store so that permission is checked */
read_entry!(page: i64): Option (from: i64, to: i64, hv_or_vm_id: i8, acc_lv: AccesLevel) :=  ...
  /* NOTE: first checks validity bit */
write_entry!(page: i64, (from: i64, to: i64, hv_or_vm_id: i8, acc_lv: AccesLevel)): bool := ...
  /* NOTE: it writes is_valid bit at last */
invalidate_entry!(page: i64) := ...

(HW)
```Coq
Module HW {
  fun hw_load(addr) : option memval {
    if(check_permission(addr))
      return Some(Mem[addr])
    else 
      return None
  }

  fun hw_store(addr, memval) : bool {
    if(check_permission(addr)) {
      Mem[addr] = val;
      return true
    }
    else 
      return false
  }

  priv fun check_permission(addr) {
    if (is_hv) 
      current_hv_or_vm = 0
    else
      current_hv_or_vm = current_vm
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i) {
        Some(from, to, hv_or_vm_id, _) => {
          if(hv_or_vm_id == current_hv_or_vm && from <= addr < to) return true;
        }
        None => _
      }
    }
    return false;
  }
}
```


(Mpool_SP)
//Singleton object
//Its implementation uses lock
```Coq
Module Mpool {
  pages: Set int64

  fun alloc_page() : int64 {
    match pages.get() with 
    | Some(page) => return page
    | None => return NULL
    end
  }

  fun free_page(page: int64) {
    pages.put(page)
  }
}
```


(Mpool_SP_rust)
//Diff: It maintains invariant
```Coq
Module Mpool {
  pages: { set: Set int64 | range: forall elem in set, 100 <= elem < 200 && elem % 10 == 0 }

  fun alloc_page() : int64 {
    match pages.get() with 
    | Some(i64) => return i64
    | None => return NULL
    end
  }
  
  fun free_page(page: int64) : bool {
    assume!(100 <= page < 200 && page % 10 == 0);
    pages.put(page);
  }
}
```


(HVC_Impl)
//Q: Is HVC/memory access(HW) thread-safe?
// - 한 VM이 여러 코어에서 동시에 give_memory 부르면 OWN 이 두개 이상이 될 수 있음. lock 잡아야 함.
// - revoke 해도 이미 issue 된 메모리 접근은 계속 진행할 수 있음. 
//   어떤 경우에도 permission table이 corrupt 된게 노출되지는 않음 (zeroing을 이상하게 하면 일어날 수도 있는데, 안함)
// - share 했을 때 메모리 접근이 check_permission (read_entry!) 통과했으면 write_entry! 에서 적었던 내용이 전부 전달됨.
//   어떤 경우에도 permission table이 corrupt 된게 노출되지는 않음 (write_entry!가 완전히 적히지 않은 상태)
```Coq
Module HVC {
  priv fun current_vm_is_owner(from: int64, to: int64) : bool {
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i) with {
        Some(from', to', current_vm, OWN) => {
          if((from, to) ⊆ (from', to')) return true;
        }
        _ => _
      }
    }
    return false;
  }
  
  fun share_memory(from: int64, to: int64, vm_id: int8) : int64 {
    if(!current_vm_is_owner(from, to)) return -1
    let new_page = Mpool.alloc_page()
    if(new_page == NULL) return -1
    if(!write_entry!(new_page, (from, to, vm_id, ACC))) return -1
    return 0
  }
  
  fun give_memory(from: int64, to: int64, vm_id: int8) : int64 {
    if(!current_vm_is_owner(from, to)) return -1
    // TODO: fill in
  }

  fun revoke_memory(from: int64, to: int64, vm_id: int8) : int64 {
    if(!current_vm_is_owner(from, to)) return -1
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i) with {
        Some(from, to, vm_id, ACC) => {
          invalidate_entry!(page)
          Mpool.free_page(100 + 10*i)
          return 0
        }
        _ => _
      }
    }
    return -1
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
    (pure_operation|HW.hw_load|HW.hw_store|HVC.share_memory|HVC.revoke_memory)*
  //pure_operation changes local_state (which is register value physically)
}
```


(VM2_Impl)
```Coq
Module VM2 {
  ... ditto ...
}
```
