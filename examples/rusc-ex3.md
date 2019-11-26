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
[100, 200) -> permission (page) table. mpool manages it
  [   ,    ) -> (from, to, hv_or_vm_id, AccessLevel)
  [100, 110) -> (0,   200,           0, OWN)
  [110, 120) -> (200, 300,           1, OWN)
  [120, 130) -> (300, 400,           2, OWN)
[200, 300) -> VM1
[300, 400) -> VM2

Macroes
assume!(cond)    := if(!cond) UndefinedBehavior
guarantee!(cond) := if(!cond) NoBehavior
read_entry!(from: i64, to: i64): Option (from: i64, to: i64, hv_or_vm_id: i8, acc_lv: AccesLevel) := 
  /* it uses HW.hw_load, which checks permission */
write_entry!(from: i64, to: i64, (from: i64, to: i64, hv_or_vm_id: i8, acc_lv: AccesLevel)): bool :=
  /* it uses HW.hw_store, which checks permission */
invalidate_entry!(from: i64, to: i64) :=
  /* it uses HW.hw_store, which checks permission */

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
      match read_entry!(100 + 10*i, 100 + 10*i + 10) {
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
    invalidate_entry!(page, page+10)
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
```Coq
Module HVC {
  fun share_memory(from: int64, to: int64, vm_id: int8) : int64 {
    bool current_vm_is_owner = false
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i, 100 + 10*i + 10) {
        Some(from', to', current_vm, OWN) => {
          if((from, to) ⊆ (from', to')) {
            current_vm_is_owner = true
            break
          }
        }
        _ => _
      }
    }
    if(!current_vm_is_owner) return -1
    let new_page = Mpool.alloc_page()
    if(new_page == NULL) return -1
    if(!write_entry!(new_page, new_page + 10, (from, to, vm_id, ACC))) return -1
    return 0
  }

  fun revoke_memory(from: int64, to: int64, vm_id: int8) : int64 {
    bool current_vm_is_owner = false
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i, 100 + 10*i + 10) {
        Some(from', to', current_vm, OWN) => {
          if((from, to) ⊆ (from', to')) {
            current_vm_is_owner = true
            break
          }
        }
        _ => _
      }
    }
    if(!current_vm_is_owner) return -1
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i, 100 + 10*i + 10) {
        Some(from, to, vm_id, ACC) => {
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
