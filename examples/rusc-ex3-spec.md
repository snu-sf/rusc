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
  (* NOREAD? -> prove non-interference lemma? memory permission that induces NB? *)

  fun run2() : (ml: Mem * local_state) -> (ml': Mem * local_state) -> Prop :=
    ... ditto ...
}
```
