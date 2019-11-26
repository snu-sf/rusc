HW + HV_Impl <= HW + HV_Spec
HW + HV_Spec + VM1_Impl + VM_Impl2 <= HW + Top_Spec
HW + Top_Spec <= HW + Top'_Spec
(HW module exists everywhere. HW module's functions are like inline functions)

Top_Spec: erase UB
Top'_Spec: introduce NOB



Global Data: Mem, current_vm, is_hv

current_vm is either 1 or 2. It is changed directly in interaction semantics (interleaving run1, run2)
is_hv is either true or false. It is changed directly in interaction semantics (hvcall, return)

/*
Mem[0] represents owner of page [000, 100).
Mem[1] represents owner of page [100, 200).
Mem[2] represents owner of page [200, 300).

Mem[0] = 0x001 (HV owns it)
Mem[1] = 0x010 (VM1 owns it)
Mem[2] = 0x100 (VM2 owns it)
*/

[100, 200) -> mpool
[200, 300) -> VM1
[300, 400) -> VM2

assume(cond)    := if(!cond) UndefinedBehavior
guarantee(cond) := if(!cond) NoBehavior



(HW)
```Coq
Module HW {
  priv fun check_perm(addr) {
    assert(current_vm == 1 || current_vm == 2);
    if (is_hv) 
      current_hv_or_vm = 0
    else
      current_hv_or_vm = current_vm
    page = addr/100;
    assert(0 <= page <= 2);
    return (Mem[page] & (1 << current_hv_or_vm));
  }

  fun hw_load(addr) {
    if(check_perm(addr))
      return Some(Mem[addr])
    else 
      return None
  }

  fun hw_store(addr, val) {
    if(check_perm(addr)) {
      Mem[addr] = val;
      return true
    }
    else 
      return false
  }
}
```


(HV_Impl)
```Coq
(owns memory [0, 100))
Module HV {
  fun share_memory(owners) {
    hw_store(current_vm, owners);
  }

  ...
}
```


(VM1_Impl)
```Coq
(initially owns memory [100, 200))
Module VM1 {
  LS: Type
  local_state1: LS
  fun run() : (Mem * local_state) -> (Mem * local_state) -> Prop := 
    (pure_operation|hw_load|hw_store|hvcall_share_memory)*
  //pure_operation changes local_state (which is register value physically)
}
```


(VM2_Impl)
```Coq
(initially owns memory [200, 300))
Module VM2 {
  ... ditto ...
}
```
