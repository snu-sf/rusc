(Lock_Impl) See `code/Lock.[hc]`

(Map_Impl) See `code/Map.[hc]`

(F_Impl) See `code/F.[hc]`

(G_Impl) See `code/G.[hc]`

-----------------------

Lock_Impl <= Lock_SP1

Map_Impl <= Map_SP1

F_Impl <= F_SP1

G_Impl <= G_SP1

Lock_SP1 o Map_SP1 <= Lock_SP1 o Map_SP2

Map_SP2 o F_SP1 <= Map_SP2 o F_SP2

Map_SP2 o G_SP1 <= Map_SP2 o G_SP2

F_SP2 o G_SP2 <= F_SP3 o G_SP3

=> by RUSC theory

Lock_Impl o Map_Impl o F_Impl o G_Impl <= Lock_SP1 o Map_SP2 o F_SP3 o G_SP3

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
  map: int64 -> option (int64 * bool * (int64 -> option int64)) = (fun _ => None)

  fun new() : int64 {
    if (choose { true, false }) return 0
    lock = [** Lock.new() **]
    if (lock == 0) return 0
    id = choose { p | map[p] = None /\ p != 0 }
    map[id] := Some (lock, false, (fun _ => None))
    return id
  }

  fun insert(id: int64, key: int64, val: int64) {
    match map[id] with    
    | None => UB
    | Some (lock, _, _) =>
        [** Lock.lock(lock) **]
        match map[id] with
        | None | Some(lock, true, _) => UB
        | Some(lock, false, m) =>
            map[id] := (lock, true, m)
            [** YIELD **]
            m[key] := Some val
            map[id] := (lock, false, m)
            [** Lock.unlock(lock) **]
            return true
        end
    end
  }

  fun find(id: int64, key: int64) : Option int64 {
    match map[id] with
    | None => UB
    | Some (lock, _, _) =>
        [** Lock.lock(lock) **]
        match map[id] with
        | None | Some(lock, true, _) => UB
        | Some(lock, false, m) =>
            map[id] := (lock, true, m)
            [** YIELD **]
            map[id] := (lock, false, m)
            [** Lock.unlock(lock) **]
            return m[key]
    end
  }
}
```

(F_SP1)
```Coq
Module F {
  initialized : bool = false
  memoized: int64 = 0

  fun init() {
    if (initialized == true) UB
    initialized := true
  
    memoized := [** Map.new() **]
    if (memoized == 0) { initialized := false; return false }
    
    return true
  }

  fun sum(i: int64) : int64 {
    if (!initialized \/ memoized == 0) UB
    if (!(0 <= i <= 1000)) UB
  
    if (i == 0) return 0
    
    res = [** Map.find(memoized, i) **]
    match res with
    | Some sum => return sum
    | None =>
        sum = [** G.sum(i-1) **] + i
        [** Map.insert(memoized, i, sum) **]
        return sum
    end
  }
}
```

(G_SP1)
```Coq
Module G {
  initialized : bool = false
  memoized: int64 = 0

  fun init() {
    if (initialized == true) UB
    initialized := true
  
    memoized := [** Map.new() **]
    if (memoized == 0) { initialized := false; return false }
    
    return true
  }

  fun sum(i: int64) : int64 {
    if (!initialized \/ memoized == 0) UB
    if (!(0 <= i <= 1000)) UB
  
    if (i == 0) return 0
    
    res = [** Map.find(memoized, i) **]
    match res with
    | Some sum => return sum
    | None =>
        sum = [** F.sum(i-1) **] + i
        [** Map.insert(memoized, i, sum) **]
        return sum
    end
  }
}
```

(Map_SP2)
```Coq
Module Map {
  map: int64 -> option (int64 -> option int64) = (fun _ => None)

  fun new() : int64 {
    if (choose { true, false }) return 0
    id = choose { p | map[p] = None /\ p != 0 }
    map[id] := Some (fun _ => None)
    return id
  }

  fun insert(id: int64, key: int64, val: int64) {
    match map[id] with
    | None => UB
    | Some m =>
        m[key] := Some val
        map[id] := Some m
        return true
    end 
  }

  fun find(id: int64, key: int64) : Option int64 {
    match map[id] with
    | None => UB
    | Some m =>
        return m[key]
    end
  }
}

(F_SP2)
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
        sum = [** G.sum(i-1) **]
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
```

(G_SP3)
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
    return (summation(i))
  }
}
```
