(Lock_Impl) ...

(Map_Impl) ...

(F_Impl) ...

(G_Impl) ...

-----------------------

Lock_Impl <= Lock_SP1

Map_Impl <= Map_SP1

F_Impl <= F_SP1

G_Impl <= G_SP1

Lock_SP1 o Map_SP1 o F_SP1 <= Lock_SP1 o Map_SP1 o F_SP2

Lock_SP1 o Map_SP1 o G_SP1 <= Lock_SP1 o Map_SP1 o G_SP2

F_SP2 o G_SP2 <= F_SP3 o G_SP3

=> by RUSC theory

Lock_Impl o Map_Impl o F_Impl o G_Impl <= Lock_SP1 o Map_SP1 o F_SP3 o G_SP3

-----------------------

(Lock_SP1)
```Coq
Module Lock {
  locked : int64 -> Option bool = (fun _ => None)

  fun new() : int64 {
    if (choose { true, false }) return 0
    id = choose { p | locked[p] = None /\ p != 0 }
    locked[id] := Some false
    return id
  }

  fun lock(id: int64) {
    while (1) {
      match locked[id] with
      | None => UB
      | Some true => [** YIELD **]
      | Some false => break
      end
    }
    
    locked[id] := true
  }

  fun unlock(id: int64) {
    match locked[id] with
    | None => UB
    | Some false => UB
    | Some true =>
    end

    locked[id] := false
  }
}
```

(Map_SP1)
```Coq
Module Map {
  map: int64 -> option (bool * (int64 -> option int64)) = (fun _ => None)

  fun new() : int64 {
    if (choose { true, false }) return 0
    id = choose { p | map[p] = None /\ p != 0 }
    map[id] := Some (false, (fun _ => None))
    return id
  }

  fun insert(id: int64, key: int64, val: int64) {
    match map[id] with
    | None | Some(true, _) => UB
    | Some(false, m) =>
        map[id] := (true, m)
        [** YIELD **]
        m[key] := Some val
        map[id] := (false, m)
        return true
    end
  }

  fun find(id: int64, key: int64) : Option int64 {
    match map[id] with
    | None | Some(true, _) => UB
    | Some(false, m) =>
        map[id] := (true, m)
        [** YIELD **]
        map[id] := (false, m)
        return m[key]
    end
  }
}
```

(F_SP1)
```Coq
Module F {
  initialized : bool = false
  lock: int64 = 0
  memoized: int64 = 0

  fun init() : bool {
    if (initialized == true) UB
    initialized := true
    
    lock := [** Lock.new() **]
    if (lock == 0) { initialized := false; return false }
    
    memoized := [** Map.new() **]
    if (memoized == 0) { initialized := false; return false }

    return true
  }

  fun sum(i: int64) : int64 {
    if (!initialized \/ lock == 0 \/ memoized == 0) UB
    if (!(0 <= i <= 1000)) UB

    if (i == 0) return 0

    [** Lock.lock(lock) **]
    res = [** Map.find(memoized, i) **]
    [** Lock.unlock(lock) **]
    match res with
    | Some sum => return sum
    | None =>
        sum = [** G.sum(i-1) **] + i
	[** Lock.lock(lock) **]
        [** Map.insert(memoized, i, sum) **]
        [** Lock.unlock(lock) **]
        return sum
    end
  }
}
```

(G_SP1)
```Coq
Module G {
  initialized : bool = false
  lock: int64 = 0
  memoized: int64 = 0

  fun init() : bool {
    if (initialized == true) UB
    initialized := true
    
    lock := [** Lock.new() **]
    if (lock == 0) { initialized := false; return false }
    
    memoized := [** Map.new() **]
    if (memoized == 0) { initialized := false; return false }

    return true
  }

  fun sum(i: int64) : int64 {
    if (!initialized \/ lock == 0 \/ memoized == 0) UB
    if (!(0 <= i <= 1000)) UB

    if (i == 0) return 0

    [** Lock.lock(lock) **]
    res0 = [** Map.find(memoized, 0) **]
    if (res0 == Some i) {
      res1 = [** Map.find(memoized, 1) **]
      [** Lock.unlock(lock) **]
      match res1 with
      | None => UB
      | Some sum => return sum
      end
    } else {
      [** Lock.unlock(lock) **]    
    }

    sum = [** F.sum(i-1) **] + i

    [** Lock.lock(lock) **]
    [** Map.insert(memoized, 0, i) **]
    [** Map.insert(memoized, 1, sum) **]
    [** Lock.unlock(lock) **]    
    return sum
  }
}
```

(F_SP2)
```Coq
Module F {
  initialized : bool = false

  fun init() : bool {
    if (initialized == true) UB
    initialized := true
    if (choose { true, false }) {
       initialized := false
       return false
    } else {
       return true
    }
  }

  fun sum(i: int64) : int64 {
    if (!initialized) UB
    if (!(0 <= i <= 1000)) UB
    if (i == 0) return 0
    if (choose { true, false }) {
        sum = [** G.sum(i-1) **]
        if (sum != summation(i-1)) UB
    }
    return (summation(i))
  }
}
```

(G_SP2)
```Coq
Module G {
  initialized : bool = false

  fun init() : bool {
    if (initialized == true) UB
    initialized := true
    if (choose { true, false }) {
       initialized := false
       return false
    } else {
       return true
    }
  }

  fun sum(i: int64) : int64 {
    if (!initialized) UB
    if (!(0 <= i <= 1000)) UB
    if (i == 0) return 0
    if (choose { true, false }) {
        sum = [** F.sum(i-1) **]
        if (sum != summation(i-1)) UB
    }
    return (summation(i))
  }
}
```

(F_SP3)
```Coq
Module F {
  initialized : bool = false

  fun init() : bool {
    if (initialized == true) UB
    initialized := true
    if (choose { true, false }) {
       initialized := false
       return false
    } else {
       return true
    }
  }

  fun sum(i: int64) : int64 {
    if (!initialized) UB
    if (!(0 <= i <= 1000)) UB
    return (summation(i))
  }
}

(G_SP3)
Module G {
  initialized : bool = false

  fun init() : bool {
    if (initialized == true) UB
    initialized := true
    if (choose { true, false }) {
       initialized := false
       return false
    } else {
       return true
    }
  }

  fun sum(i: int64) : int64 {
    if (!initialized) UB
    if (!(0 <= i <= 1000)) UB
    return (summation(i))
  }
}
```
