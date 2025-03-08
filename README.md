# btrees

40x faster AVX-512 vectorised static b-trees benchmarked against std::lower_bound. 

  - [btree](#btree)
  - [bplus](#bplus)
  - [batching_bplus](#bplus)

The below is intended to be read top down and explains how we derive each optimisation. Of course, use the links above to jump to what you find interesting :)

## Implementations

<img src="./results/plot.png" width="800" alt="lower_bound() comparison" />

We will cover a few implementations below. For each one we will present a profile from `perf` on a dataset of 64MB (see [src/profile.cpp](./src/profile.cpp) and [profile.sh](./profile.sh)). The cache based information is not so helpful at this data set size. It would be better to provide profiles within L1, L2, L3, RAM, etc., but the fans on my PC are broken and I need to point an industrial fan at my laptop for every benchmark...

It is worth noting this is all done on the AMD Zen4 microarchitecture which has:

- L1d - 32KB 8-way set associative
- L2d - 1MB 8-way set associative
- L3d - 32MB 16-way set associative (shared)
- L1d TLB - 72 entries fully associative (64MB of data on 2MB pages is only 32 entries)
- L2d TLB - 3072 entries 24-way set associative

> These profiles are likely not perfect as there are many factors to control. My harness certainly inflates cache references and misses; it would be better to use a library. I do at least pin the CPU to a single core, set our 'niceness' to -20 to avoid being preemepted, and ensure we're running on the `performance` CPU governor to prevent frequency scaling.

We'll start with `std::lower_bound()` to establish a baseline:

```
        366,314.01 msec task-clock                       #    0.980 CPUs utilized
           182,649      context-switches                 #  498.613 /sec
                 0      cpu-migrations                   #    0.000 /sec
                 6      page-faults                      #    0.016 /sec
 1,644,098,096,642      cycles                           #    4.488 GHz                         (33.36%)
   283,750,588,990      stalled-cycles-frontend          #   17.26% frontend cycles idle        (33.35%)
    80,345,190,172      instructions                     #    0.05  insn per cycle
                                                  #    3.53  stalled cycles per insn     (33.34%)
    18,963,996,859      branches                         #   51.770 M/sec                       (33.32%)
     3,927,239,950      branch-misses                    #   20.71% of all branches             (33.31%)
    97,890,725,014      cache-references                 #  267.232 M/sec                       (33.30%)
    57,723,201,472      cache-misses                     #   58.97% of all cache refs           (33.30%)
   240,881,422,337      L1-dcache-loads                  #  657.582 M/sec                       (33.31%)
    61,927,977,402      L1-dcache-load-misses            #   25.71% of all L1-dcache accesses   (33.33%)
    58,803,167,555      dTLB-loads                       #  160.527 M/sec                       (33.35%)
    44,045,075,637      dTLB-load-misses                 #   74.90% of all dTLB cache accesses  (33.35%)

     373.603498850 seconds time elapsed

     364.873843000 seconds user
       2.563420000 seconds sys
```

The key takeaway is that we jump around 64MB of memory according to properties of random data. So, we miss 58% of our cache references and mispredict 20% of our 18 billion branches, which reflect in the 17% front end cycle stall. This means the CPU is just sat cleaning up after itself and waiting on memory, leading to an abysmal IPC of 0.05.


### btree

This is the first of our trees. It extends the Eytzinger layout (`2k`, `2k + 1`) to B-trees which have `B + 1` children, where `B` is the number of keys in a block (16 in this case - one cache line). This outperforms `std::lower_bound` by up to 10x in terms of reciprocal throughput.

```cpp
// Traverse from the root block, taking the first element not less than our target.
int lower_bound(int target) const noexcept {
    int found = 0;

    u32 block = 0;
    while (block < _nblocks) {
        int i = first_ge(target, &_tree[block * constants::block_len]);
        if (i < constants::block_len) {
            found = _tree[block * constants::block_len + i];
        }

        block = child(block, i);
    }

    return found;
}


// Compare data >= target across the whole block simultaneously. The number of results
// for which this is false (0, counted by tzcnt) is the index of the first key which is
// not less than the target, i.e. the lower bound.
int first_ge(int target, int* block) const noexcept {
    __m512i target_vec = _mm512_set1_epi32(target);
    __m512i data = _mm512_load_si512(reinterpret_cast<__m512i*>(block));
    __mmask16 mask = _mm512_cmpge_epi32_mask(data, target_vec);
    return __tzcnt_u16(mask);
}
```

A high-level profile from `perf`:

```
         26,163.74 msec task-clock                       #    0.985 CPUs utilized
            13,511      context-switches                 #  516.402 /sec
                 0      cpu-migrations                   #    0.000 /sec
                 6      page-faults                      #    0.229 /sec
   124,124,983,795      cycles                           #    4.744 GHz                         (45.49%)
     2,583,889,976      stalled-cycles-frontend          #    2.08% frontend cycles idle        (45.46%)
    33,138,737,009      instructions                     #    0.27  insn per cycle
                                                  #    0.08  stalled cycles per insn     (45.41%)
     4,384,601,376      branches                         #  167.583 M/sec                       (45.44%)
       193,145,489      branch-misses                    #    4.41% of all branches             (45.42%)
     1,771,561,132      cache-references                 #   67.711 M/sec                       (45.42%)
       828,118,228      cache-misses                     #   46.75% of all cache refs           (45.42%)
     9,696,193,588      L1-dcache-loads                  #  370.597 M/sec                       (45.46%)
     1,072,994,324      L1-dcache-load-misses            #   11.07% of all L1-dcache accesses   (45.48%)
       227,858,754      dTLB-loads                       #    8.709 M/sec                       (45.50%)
        69,370,837      dTLB-load-misses                 #   30.44% of all dTLB cache accesses  (45.50%)

      26.568969520 seconds time elapsed

      27.022361000 seconds user
       1.070548000 seconds sys
```

This clearly runs a lot quicker (364s vs 27s). It is worth noting before we delve into any percentages, the absolute terms on a lot of our metrics have dropped significantly.

- `instructions` 80B -> 33B
- `branches` 18B -> 4B
- `cache-references` 98B -> 2B

We're literally doing less work by being less general; that's not to be ignored and significantly contributes to the performance gain. 

If we want to get more granular, we see our branch misses dropped from 20% to 4.41% and consequently stall only 2% of our frontend cycles. Our IPC greatly increased because the CPU is spending less time undoing predictions which it had little chance of predicting in the first place. Our cache misses dropped from 58% to 47%, which is nice, but it's clear this isn't really where the gains are coming from.

> Our IPC is still very low at 0.27. This is because we're issuing AVX-512 instructions, which do a lot of work (process entire cache lines) in one instruction.

How do we go faster? Let's look at what it is we're mispredicting:

```
       │    │found = _tree[block * constants::block_len + i];                                                                                                                 
  2.10 │330:│  movzx        eax,ax                                                                                                                                            
       │    │return child_block + offset;                                                                                                                                     
  6.69 │    │  lea          edx,[rdx+rcx*1+0x1]                                                                                                                               
       │    │found = _tree[block * constants::block_len + i];                                                                                                                 
  3.13 │    │  lea          edi,[rax+rcx*1]                                                                                                                                   
       │    │return child_block + offset;                                                                                                                                     
  4.79 │    │  add          edx,eax                                                                                                                                           
       │    │found = _tree[block * constants::block_len + i];                                                                                                                 
  3.83 │    │  mov          edi,DWORD PTR [r11+rdi*4]                                                                                                                         
       │    │while (block < _nblocks) {                                                                                                                                       
  4.19 │    │  cmp          esi,edx                                                                                                                                           
       │    │↑ ja           278                                                                                                                                               
 30.96 │    └──jmp          2ad     
```

We're struggling to predict the while condition more than anything else. If we knew the height of the tree in advance, we can make our code branchless. It'd just have a loop which runs `height` times, which would be unrolled. A B+ tree is perfect for this.


### bplus

A B+ tree has an added idea of internal and data nodes. It is essentially a complete tree where the leaves of the tree contain the actual data. So, we just need to iterate a fixed number (the tree height) of levels and then read that final block. This implementation outperforms the previous by up to 30%.

```
int lower_bound(int target) const noexcept {
    int k = 0;

    __m512i target_vec = _mm512_set1_epi32(target);
    for (int h = num_layers - 1; h > 0; h--) {
        int i = first_ge(target_vec, _tree + offset(h) + k);

        k = k * (constants::block_len + 1) + i * constants::block_len;
    }

    int i = first_ge(target_vec, _tree + k);
    return _tree[k + i];
}
```

We'll again consult with `perf`:

```
         18,745.52 msec task-clock                       #    1.000 CPUs utilized
               208      context-switches                 #   11.096 /sec
                 0      cpu-migrations                   #    0.000 /sec
                 6      page-faults                      #    0.320 /sec
    90,876,167,288      cycles                           #    4.848 GHz                         (45.45%)
       580,143,314      stalled-cycles-frontend          #    0.64% frontend cycles idle        (45.46%)
    19,509,704,688      instructions                     #    0.21  insn per cycle
                                                  #    0.03  stalled cycles per insn     (45.45%)
       386,855,336      branches                         #   20.637 M/sec                       (45.45%)
        14,044,341      branch-misses                    #    3.63% of all branches             (45.45%)
     1,455,676,549      cache-references                 #   77.655 M/sec                       (45.46%)
       712,493,508      cache-misses                     #   48.95% of all cache refs           (45.46%)
     3,739,751,169      L1-dcache-loads                  #  199.501 M/sec                       (45.45%)
     1,052,036,642      L1-dcache-load-misses            #   28.13% of all L1-dcache accesses   (45.46%)
       131,541,396      dTLB-loads                       #    7.017 M/sec                       (45.46%)
           261,612      dTLB-load-misses                 #    0.20% of all dTLB cache accesses  (45.45%)

      18.748113779 seconds time elapsed

      20.041994000 seconds user
       0.701964000 seconds sys
```

The core idea for this was to reduce branches. In absolute terms, we dropped from 4.4B to 387M, even though the relative percent missed is only slightly lower. That is a very significant reduction - we've dropped mispredictions from 193M to 14M. Consequently, the frontend is only idle for 0.64% of cycles.

I'm wary of the cache misses and TLB misses. I think this is more to do with the profiling harness than the implementation. I don't have an explanation as to why the dTLB misses dropped from 30% to 0.2%, but, 0.2% sounds far more reasonable as the entire 64MB of data fits in 32 of the 72 L1 TLB entries.

An interesting note is that our IPC dropped from 0.27 to 0.21. Counterintuitively, this is a good thing. The IPC was higher previously because we were issuing more instructions overall, and those instructions run on execution ports which are independent of those which are executing our AVX-512 instructions. That artificially drove up our IPC, because we were parallelising useless overhead with our 'actual' work. 

#### How do we go faster?

I'm not sure how much of those cache / TLB misses are the profiling harness, but, that seems to be the bottleneck.

The typical solution is to prefetch and tell the CPU we need that data soon, so, get it ready. There's a caveat however; prefetching is only effective if you have other work to do while you wait for your data to be fetched in the background. That's not true for us. We do the work to figure out where to go, then we need to go there immediately. There's nothing to overlap.

Well, we're already measuring throughput. What if we allowed the implementation to leverage that? What if we moved those many queries into our core lower bound loop? What if, instead of immediately 'going there', we prefetch, then overlap that work figuring out where we need to go for another query, prefetch, and so on? *Batching is the answer.*


### batching_bplus

See just above for the idea, but, this is where things get really fast. Here, we can run a lower bound query on 1GiB of data in 38ns. That's a 40x speed up over `std::lower_bound()`.

```
void lower_bound_batch(const int* queries, int* results) const noexcept {
    int positions[B]{};

    __m512i targets[B];
    for (size_t i = 0; i < B; i++) {
        targets[i] = _mm512_set1_epi32(queries[i]);
    }

    for (int h = num_layers - 1; h > 0; h--) {
        for (size_t i = 0; i < B; i++) {
            int k = positions[i];

            int key_i = first_ge(targets[i], _tree + offset(h) + k * constants::block_len);
            positions[i] = k * (constants::block_len + 1) + key_i;

            int* next_block = _tree + offset(h - 1) + positions[i] * constants::block_len;
            _mm_prefetch(reinterpret_cast<const char*>(next_block), _MM_HINT_T0);
        }
    }

    for (size_t i = 0; i < B; i++) {
        int* leaf_block = _tree + positions[i] * constants::block_len;
        results[i] = leaf_block[first_ge(targets[i], leaf_block)];
    }
}
```

All we do is hold an array of positions into the next layer we'll look at, initially zero for the root node. We then work our way through the tree, and for each batch, compute where we need to go on the next layer. We'll update our position, then load that cache line into L1 with `_MM_HINT_T0`. When we revisit this query in the batch we won't need to wait on main memory; it's already there.

Customary `perf` output with a batch size of 64:

```
          7,206.46 msec task-clock                       #    1.000 CPUs utilized
                24      context-switches                 #    3.330 /sec
                 0      cpu-migrations                   #    0.000 /sec
                 5      page-faults                      #    0.694 /sec
    35,233,287,284      cycles                           #    4.889 GHz                         (45.45%)
     1,534,689,292      stalled-cycles-frontend          #    4.36% frontend cycles idle        (45.46%)
    62,889,785,198      instructions                     #    1.78  insn per cycle
                                                  #    0.02  stalled cycles per insn     (45.47%)
    12,435,640,167      branches                         #    1.726 G/sec                       (45.47%)
       136,540,875      branch-misses                    #    1.10% of all branches             (45.47%)
     1,194,403,357      cache-references                 #  165.741 M/sec                       (45.46%)
       593,607,772      cache-misses                     #   49.70% of all cache refs           (45.45%)
    12,787,600,737      L1-dcache-loads                  #    1.774 G/sec                       (45.44%)
     1,085,507,013      L1-dcache-load-misses            #    8.49% of all L1-dcache accesses   (45.44%)
       134,275,834      dTLB-loads                       #   18.633 M/sec                       (45.44%)
            39,816      dTLB-load-misses                 #    0.03% of all dTLB cache accesses  (45.44%)

       7.207235275 seconds time elapsed

       9.050222000 seconds user
       0.155986000 seconds sys
```

9 seconds runtime is pretty great; that's twice as fast as our last one. We can see `L1-dcache-load-misses` dropped down to 8.49% from the previous 28.13%, which is a massive reduction. I'm not sure where the 50% cache misses (which are consistent across all these profiles) are coming from, but, I'd assume it's a test harness thing.

Our IPC also shot up significantly to 1.78 from 0.21, but note also that our total instructions increased from 20B to 63B. As previously explained, this increase is largely explained by the parallelising the extra scalar instructions from batching, which run in parallel with our AVX-512 instructions.
