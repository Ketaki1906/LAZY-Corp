# Concurrency


## LAZY Read-Write

### Execution Instructions

```

gcc LazyReadWrite.c
./a.out

```

### Assumptions

1. When two requests come at the same time then LAZY handles it together.

## LAZY Sort

### Execution Instructions

```

gcc LazySort.c
./a.out

```

### Assumptions

1. Due to system constraints, the max string length for the file name may vary. The results provided are for filenames in range of 0-20 characters.
2. The filenames are sorted according to their hash-values obtained.


