(HVC_Spec)

//Note: all methods are atomic now (no [** YIELD **]) && local lock is disappeared

//TODO: Mpool.alloc_page(), free_page()도 지워도 됨. 다음 단계(합칠 때)?

//Note: write_entry! UB가 사라졌는데 이게 설명되는 이유: corresponds에서 자기 permission 써놓음

//TODO: latest_seen_memory 저장할 필요 없나?

```Coq
Module HVC {
  //logical view of permission table
  //TODO: does it have to me module-local?
  permission_table: int64 -> Set (hv_or_vm_id: int64, acc_lv: AccessLevel)

  //correspondence between logical view and physical state
  corresponds: Mem -> permission_table -> Prop := ...

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
    assume!(corresponds(Mem, permission_table))
    if(!current_vm_is_owner(from, to)) return -1
    let new_page = Mpool.alloc_page()
    if(new_page == NULL) return -1
    forall i in [from, t), 
      (permission_table i).put(vm_id, ACCESSIBLE)
    Mem <-| Mem' s.t. corresponds(Mem', permission_table) 
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
        (permission_table i).clear().put(vm_id, OWN)
      Mem <-| Mem' s.t. corresponds(Mem', permission_table) 
      return 0
    }
    else return -1
  }

  fun revoke_memory(from: int64, to: int64, vm_id: int8) : int64 {
    assume!(corresponds(Mem, permission_table))
    if(!current_vm_is_owner(from, to)) return -1
    if(choose { true, false }) {
      //TODO: 지금 구현이랑 안맞음. 이게 되려면 구현에서 (from, to) 한개 뿐임을 확인해야 함
      forall i in [from, to),
        (permission_table i).filter(|elem| elem.fst <> vm_id)
      Mem <-| Mem' s.t. corresponds(Mem', permission_table) 
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
  local_state1, local_state2

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
  local_state1, local_state2
  
  fun share_memory(owners) {
    Mem[current_vm] = owners;
  }
 
  ...

  fun run1() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    VM1_Impl.run ml ml' /\ (NOSTORE: guarantee!(for i in [0,200), ml.fst[i] == ml'.fst[i]))
  (* TODO: NOREAD? -> prove non-interference lemma? make memory model's permission that induces NB? *)

  fun run2() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    ... ditto ...
}
```
