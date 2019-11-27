(API_Spec)
```Coq
Module API {
  isRunning = false
  //logical view of permission table
  permission_table: int64 -> Set (hv_or_vm_id: int64)

  //correspondence between logical view and physical state
  corresponds: Mem[100, 200) -> permission_table -> Prop := ...

  priv fun current_vm_is_owner(from: int64, to: int64) : bool {
    forall i in [from, to), 
      (permission_table i).contains(eq current_vm)
  }
  
  priv fun current_vm_is_exclusive_owner(from: int64, to: int64) : bool {
    current_vm_is_owner(from, to) /\
    forall i in [from, to), 
      (permission_table i).forall(eq current_vm)
  }
  
  fun share_memory(from: int64, to: int64, vm_id: int8) : int64 {
    [** l.lock() **] ; assume!(!isRunning) ; isRunning = true
    assume!(corresponds(Mem, permission_table))
    if(!current_vm_is_owner(from, to)) { 
      isRunning = false ; [** l.unlock() **]
      return -1 
    }
    let new_page = Mpool.alloc_page()
    if(new_page == NULL) { 
      isRunning = false ; [** l.unlock() **]
      return -1
    }
    forall i in [from, t), 
      (permission_table i).put(vm_id)
      Mem[100, 200) <-| new_physical s.t. corresponds(new_physical, permission_table) 
    //Note: choosing Mem' is non-deterministic. 
    //logically: [0,100) -> BLAH 
    //physically: [0,100) -> BLAH || ([0, 40) -> BLAH, [40, 100) -> BLAH) || ... || ...
    isRunning = false ; [** l.unlock() **]
    return 0
  }
  
  fun give_memory(from: int64, to: int64, vm_id: int8) : int64 {
    [** l.lock() **] ; assume!(!isRunning) ; isRunning = true
    assume!(corresponds(Mem, permission_table))
    if(!current_vm_is_exclusive_owner(from, to)) {
      isRunning = false ; [** l.unlock() **]
      return -1
    }
    if(choose { true, false }) {
      forall i in [from, to),
        (permission_table i).clear().put(vm_id)
      Mem[100, 200) <-| new_physical s.t. corresponds(new_physical, permission_table) 
      isRunning = false ; [** l.unlock() **]
      return 0
    }
    else { 
      isRunning = false ; [** l.unlock() **]
      return -1
    }
  }

  fun fault_handler := ...

  ...
}
```



(API_Spec')

//Note: all methods are atomic now (no [** YIELD **]) && local lock is disappeared

//TODO: Mpool.alloc_page(), free_page()도 지워도 됨. 다음 단계(합칠 때)?

//Note: write_entry! UB가 사라졌는데 이게 설명되는 이유: corresponds에서 자기 permission 써놓음

//TODO: latest_seen_memory 저장할 필요 없나?

//NOTE: corresponds가 그냥 막 정한 relation은 아니고, 잘 정한 것. physical view로 보는 다른 모듈들이 (여기선 HW) 존중하는 relation. (p1 ~ l, p2 ~ l 이면 p1이든 p2든 상관 안함)

```Coq
Module API {
  //logical view of permission table
  //TODO: does it have to me module-local?
  permission_table: int64 -> Set (hv_or_vm_id: int64)

  //correspondence between logical view and physical state
  corresponds: Mem[100, 200) -> permission_table -> Prop := ...

  priv fun current_vm_is_owner(from: int64, to: int64) : bool {
    forall i in [from, to), 
      (permission_table i).contains(eq current_vm)
  }
  
  priv fun current_vm_is_exclusive_owner(from: int64, to: int64) : bool {
    current_vm_is_owner(from, to) /\
    forall i in [from, to), 
      (permission_table i).forall(eq current_vm)
  }
  
  fun share_memory(from: int64, to: int64, vm_id: int8) : int64 {
    assume!(corresponds(Mem, permission_table))
    if(!current_vm_is_owner(from, to)) return -1
    let new_page = Mpool.alloc_page()
    if(new_page == NULL) return -1
    forall i in [from, t), 
      (permission_table i).put(vm_id)
      Mem[100, 200) <-| new_physical s.t. corresponds(new_physical, permission_table) 
    //Note: choosing Mem' is non-deterministic. 
    //logically: [0,100) -> BLAH 
    //physically: [0,100) -> BLAH || ([0, 40) -> BLAH, [40, 100) -> BLAH) || ... || ...
    return 0
  }
  
  fun give_memory(from: int64, to: int64, vm_id: int8) : int64 {
    assume!(corresponds(Mem, permission_table))
    if(!current_vm_is_exclusive_owner(from, to)) return -1
    if(choose { true, false }) {
      forall i in [from, to),
        (permission_table i).clear().put(vm_id)
      Mem[100, 200) <-| new_physical s.t. corresponds(new_physical, permission_table) 
      return 0
    }
    else return -1
  }

  ...
}
```




(Top_Spec)
```Coq
Module Top {
  local_state

  fun share_memory := ...

  fun give_memory := ...
 
  ...

  fun run1() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    VM1_Impl.run ml ml'
  
  fun run2() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    ... ditto ...
}
```


(Top_Spec')
```Coq
Module Top {
  local_state

  fun share_memory := ...

  fun give_memory := ...
 
  ...

  fun run1() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    VM1_Impl.run ml ml' /\ (NOSTORE: guarantee!(for i in [0,200), ml.fst[i] == ml'.fst[i]))
  (* TODO: NOREAD? -> prove non-interference lemma? make memory model's permission that induces NB? *)

  fun run2() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    ... ditto ...
}
```
