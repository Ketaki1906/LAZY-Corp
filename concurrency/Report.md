# Sorting Algorithm Performance Analysis

## Implementation Analysis 

- **Count Sort**
  - The implementation uses a **chunk-based distribution** where the input array is divided into fixed-size chunks among threads.
  - Each thread maintains its own local counting array.
  - A single array for final counts is maintained which is merged after all threads complete


  - **Advantages**
    - Prevents thread contention during counting phase as each thread works independently on its chunk and thus minimizes thread synchronization overhead
    - Sequential access pattern within chunks and memory locality helps in better cache utilization
    - Equal-sized chunks (except possibly last chunk) leads to low scheduling overhead

  - **Disadvantages**
    - Multiple local counting arrays (one per thread) hence the meory usage is 
      Memory usage: O(k * t) where:
        - k = max_file_value_used_to_sort + 1
        - t = number of threads

    - Last thread might get larger/smaller chunk but as there is no dynamic load balancing it may lead to uneven workload.

    - Final merging of local counts is sequential so could become bottleneck for large value ranges.

  - **Potential Improvements**
    - Dynamic Load Balancing : Instead of pre-assigning fixed chunks to threads, each thread takes a new chunk when it finishes its current chunk. This will help to prevent situations where some threads finish early while others are overloaded
    - Parallel Merge Phase: Instead of one thread combining all the local counting arrays, multiple threads work together to combine results. This will help to speed up the combining phase significantly and utilize all available threads during the merging phase.

  - **Conclusion**
    The chosen approach prioritizes simplicity and reduced contention over memory efficiency. For small to medium datasets, this trade-off is reasonable. However, for very large datasets or systems with memory constraints, considering the suggested improvements would be beneficial.

- **Merge Sort**
  - The implementation uses a **hierarchical parallel approach** where:
  - Initial array is divided into chunks for parallel sorting
  - Each thread independently sorts its chunk using recursive merge sort
  - Final merging phase combines sorted chunks in a tree-like pattern


  - **Advantages**
    - Efficient Division of Work is present due to natural divide-and-conquer approach
    - Only requires temporary arrays during merge and no global shared data structures needed.
      Memory usage: O(n) where n is chunk size
    - Works well with multiple threads and good for large file counts

  - **Disadvantages**
    - Frequent malloc/free operations in merge function could impact performance with large datasets and potential memory fragmentation
    - No dynamic load balancing is present as fixed threads are assigned.

  - **Potential Improvements**
    - Pre-allocate merge buffers to reduce allocation frequency
    - Adaptive chunk sizing
  
  - **Conclusion**
    For large datasets, implementing the merge sort can help in better memory efficiency
    
## Execution Time Analysis
- **Count Sort**
  - System time increases significantly with dataset size (0.001s → 3.246s)
  - Shows degrading performance with larger file counts
- **Merge Sort**
  - Maintains consistent system times (0.000s → 0.002s)
  - Demonstrates stable performance across all dataset sizes

## Memory Usage Analysis
### Count Sort
- Small file counts: ~5 files
- Medium file counts: ~20 files
- Large file counts: ~41 files 
- Observation: Exponential memory growth with dataset size from small to medium, which becomes constant from mdeium to large file counts. 

### Merge Sort
- Consistent memory usage: ~1.7-2.0MB
- Observation: Gradually increases as file count increases. Hence, shows memory efficiency.

## Key Findings
1. **System Time Efficiency**
   - Count Sort: Efficient for small datasets but scales poorly
   - Merge Sort: Maintains consistent performance across all sizes

2. **Memory Management**
   - Most significant differentiator between the algorithms
   - Count Sort shows concerning memory growth
   - Merge Sort demonstrates superior memory efficiency

## Recommended Optimizations to increase memory efficiency

### Count Sort
- Increasing number of threads would lead to additional memory overhead for thread management, as each thread would need its own counting array. Hence, an optimised number of threads can lead to best performance.
- Suitable hash function in case of strings, can further help to optimise time and mery overhead when the file count is large.

### Merge Sort
- Exhibits optimal memory usage in the execution as each thread requires its own temporary array for merging.

## Conclusion
Merge Sort demonstrates superior overall performance, particularly for memory-constrained environments or when processing large datasets. Its consistent memory usage makes it a more reliable choice for sorting.