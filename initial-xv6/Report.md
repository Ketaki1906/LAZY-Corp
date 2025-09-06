# Lazy Fork

## Page Fault Frequency 

- For the processes that read only, the frequency of page faults for COW fork would be 0. 
- The page fault frequency for the given testcases is mentioned in the table.

|Testcase|Page Fault Frequency|
|--------|--------------------|
|simple| 1| 
|simple| 1|
|three| 18843 | 
|three| 18843 | 
|three| 18843 | 
|file|12|


## Brief Analysis: Benefits and Potential Optimization

### Benefits 
1. **Memory Conservation** - By sharing pages and duplicating them only when required (on modification), COW fork reduces the memory usage, especially for read only processes.
2. **Efficiency Gain** - Increases efficiency by deferring the cost of page duplication until necessary. 

### Potential Optimization 
- For processes that are likely to modify most shared memory, a heuristic that anticipates writes could optimize performance by pre-copying certain pages.