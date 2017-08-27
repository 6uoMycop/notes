# Evaluation of Delta Compression Techniques for Efficient Live Migration of Large Virtual Machines

### Run Length Encoding Delta Compression
The VM memory pages that needs to be compressed are in binary form and it is thus easy to compute the delta page by applying XOR on the current and previous version of a page. The delta page is the same size as the original memory page and as our goal is to reduce the amount of transferred data during migration, the delta page must be compressed. Fortunately, in many cases when a page is dirtied, only a couple of words are changed. This means that the delta page can be efficiently compressed by binary Run Length Encoding (RLE) which is a well-known, fast, and efficient compression algorithm.

Run Length Encoding (RLE)  
Consider a text source: RTAAAASDEEEEE  
The RLE representation is: RT\*4ASD\*5E

#### Caching
In order to compute a delta page, the previous version of the page is needed. On the source side the memory contents are continuously overwritten and a copy of the previously sent version of the page must be kept. Hence, only a subset of the VM's memory pages should be stored which means that a caching scheme must be used.

As discussed earlier, it is often the case that a set of memory pages is being constantly dirtied, this set is called the VM's hot pages set.

### Delta Compression Algorithm Overview
In our evaluation, we use a XOR binary RLE (XBRLE) live migration algorithm in order to increase migration throughput and thus reduce downtime. When transferring a page, the source checks if a previous version of the page exists in the cache. If this is the case, a delta page between the new version and the cached version is created using XOR. The delta page is compressed using XBRLE and a delta compression flag (RAM\_SAVE\_FLAG_XBZRLE) is set in the page header. Finally, the cache is updated and the compressed page is transferred. On the destination side, if the delta compression flag is set for a page, the delta page is decompressed and the new version of the page is reconstructed from the delta page and the destination's previous version of the page using XOR.
