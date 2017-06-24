## Cached Write Policies
  
### Write-through caches
- A write-through cache forces all writes to update both the cache and the main memory.
- This is simple to implement and keeps the cache and memory consistent.
- The bad thing is that forcing every write to go to main memory, we use up bandwidth between the cache and the memory.

### Write-back caches
- In a write-back cache, the memory is not updated until the cache block needs to be replaced.
- For example,  we might write some data to the cache at first, leaving it inconsistent with the main memory as shown before.
  - The cache block is marked "dirty" to indicate this inconsistency
