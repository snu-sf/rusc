(HVC_Spec)
//Note: all methods are atomic now (no [** YIELD **]) && local lock is disappeared
```Coq
Module HVC {
  //logical view of permission table
  //TODO: does it have to me module-local?
  permission_table: int64 -> Set (hv_or_vm_id: int64, acc_lv: AccessLevel)

  priv fun current_vm_is_owner(from: int64, to: int64) : bool {
    forall i in [from, to), 
      (permission_table i).contains(fun elem => elem.fst == current_vm /\ elem.snd == OWN)
  }
  
  priv fun current_vm_is_exclusive_owner(from: int64, to: int64) : bool {
    current_vm_is_owner(from, to) /\
    forall i in [from, to), 
      (permission_table i).forall(fun elem => elem.fst == current_vm)
  }
  
  fun share_memory(from: int64, to: int64, vm_id: int8) : int64 {
    if(!current_vm_is_owner(from, to)) return -1
    let new_page = Mpool.alloc_page()
    if(new_page == NULL) return -1
    if(!write_entry!(new_page, (from, to, vm_id, ACC))) return -1
    return 0
  }
  
  fun give_memory(from: int64, to: int64, vm_id: int8) : int64 {
    if(!current_vm_is_exclusive_owner(from, to)) return -1
    forall i in [from, to),
      (permission_table i).clear().put(current_vm, OWN)TTTTTTTTTTTTTTTTT
    for(int i=0; i<10; i++) {
      match read_entry!(100 + 10*i) with {
        Some(from, to, current_vm, OWN) => {
          if(write_entry!(100 + 10*i, (from, to, vm_id, OWN))) return true
          else return false
        }
        _ => _
      }
    }
    return false
  }

  fun revoke_memory(from: int64, to: int64, vm_id: int8) : int64 {
    /* lock is omitted, but it is fully locked */
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



(HV_Spec)
```Coq
(owns memory [0, 100))
Module HV {
  latest_memory: Mem
  
  fun share_memory(owners) {
    assume!(current_vm == 1 || current_vm == 2); (TODO: just use enum type)
    assume!(for i in [0,100), latest_memory[i] == Mem[i]);
    assume!(Mem[0] == 0x001); //actually implied from above
    Mem[current_vm] := owners;
    latest_memory := Mem;
  }

  ...
}
```


(Top_Spec)
```Coq
Module Top {
  local_state1, local_state2
  
  fun share_memory(owners) {
    Mem[current_vm] = owners;
  }
 
  ...

  fun run1() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    VM1_Impl.run ml ml'
  
  fun run2() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    ... ditto ...
}
```


(Top'_Spec)
```Coq
Module Top' {
  local_state1, local_state2
  
  fun share_memory(owners) {
    Mem[current_vm] = owners;
  }
 
  ...

  fun run1() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    VM1_Impl.run ml ml' /\ (NOSTORE: guarantee!(for i in [0,200), ml.fst[i] == ml'.fst[i]))
  (* NOREAD? -> prove non-interference lemma? make memory model's permission that induces NB? *)

  fun run2() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    ... ditto ...
}
```
