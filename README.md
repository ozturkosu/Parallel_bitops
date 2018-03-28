# Parallel_bitops
This is going to parallelize the bitmap opetations.
list of the things that can be parallelized:
1. the bimap generation itself can be well parallelized, each thread takes care of a single bin. 
2. bit operations
- compress
- uncompress
- bitwise_and
- bitwise_or
- bit count
3. compressed bit operations?
4. parallel aggregate query

