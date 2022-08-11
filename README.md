Effect of adjusting accumulator hash function of the Louvain algorithm for
community detection.

[Louvain] is an algorithm for **detecting communities in graphs**. *Community*
*detection* helps us understand the *natural divisions in a network* in an
**unsupervised manner**. It is used in **e-commerce** for *customer*
*segmentation* and *advertising*, in **communication networks** for *multicast*
*routing* and setting up of *mobile networks*, and in **healthcare** for
*epidemic causality*, setting up *health programmes*, and *fraud detection* is
hospitals. *Community detection* is an **NP-hard** problem, but heuristics exist
to solve it (such as this). **Louvain algorithm** is an **agglomerative-hierarchical**
community detection method that **greedily optimizes** for **modularity**
(**iteratively**).

**Modularity** is a score that measures *relative density of edges inside* vs
*outside* communities. Its value lies between `−0.5` (*non-modular clustering*)
and `1.0` (*fully modular clustering*). Optimizing for modularity *theoretically*
results in the best possible grouping of nodes in a graph.

Given an *undirected weighted graph*, all vertices are first considered to be
*their own communities*. In the **first phase**, each vertex greedily decides to
move to the community of one of its neighbors which gives greatest increase in
modularity. If moving to no neighbor's community leads to an increase in
modularity, the vertex chooses to stay with its own community. This is done
sequentially for all the vertices. If the total change in modularity is more
than a certain threshold, this phase is repeated. Once this **local-moving**
**phase** is complete, all vertices have formed their first hierarchy of
communities. The **next phase** is called the **aggregation phase**, where all
the *vertices belonging to a community* are *collapsed* into a single
**super-vertex**, such that edges between communities are represented as edges
between respective super-vertices (edge weights are combined), and edges within
each community are represented as self-loops in respective super-vertices
(again, edge weights are combined). Together, the local-moving and the
aggregation phases constitute a **pass**. This super-vertex graph is then used
as input for the next pass. This process continues until the increase in
modularity is below a certain threshold. As a result from each pass, we have a
*hierarchy of community memberships* for each vertex as a **dendrogram**. We
generally consider the *top-level hierarchy* as the *final result* of community
detection process.

*Louvain* algorithm is a hierarchical algorithm, and thus has two different
tolerance parameters: `tolerance` and `passTolerance`. **tolerance** defines the
minimum amount of increase in modularity expected, until the local-moving phase
of the algorithm is considered to have converged. We compare the increase in
modularity in each iteration of the local-moving phase to see if it is below
`tolerance`. **passTolerance** defines the minimum amount of increase in
modularity expected, until the entire algorithm is considered to have converged.
We compare the increase in modularity across all iterations of the local-moving
phase in the current pass to see if it is below `passTolerance`. `passTolerance`
is normally set to `0` (we want to maximize our modularity gain), but the same
thing does not apply for `tolerance`. Adjusting values of `tolerance` between
each pass have been observed to impact the runtime of the algorithm, without
significantly affecting the modularity of obtained communities.

An **accumulator hashtable** is a data structure that is used to obtain the
*total weight of each community* connected to each vertex in the graph. This
is then used to find delta modularity of moving to each of its neighboring
communities. Its **capacity** is always a **prime number**, and the *hash key*
is obtained by simply finding the *modulo of the community id* with the
capacity of the accumulator hashtable. In order to accomodate all communities
in the worst case, the accumulator hastable is **normally initialized** with
a capacity of *the degree of each vertex* in the graph. High degree vertices
will thus require large accumulator hashtables. The problem with using a **large**
**accumulator hastable** is that it is **not feasible on a GPU**, where we
have a large number of *threads*, but a small amount of *working memory*
(shared memory). For *high-degree vertices*, this would have to be done in
the **global memory** instead (which is slow). In addition, we would have to
use `atomicCAS()` operations in order to **avoid collisions** which can further
drop performance.

Therefore, here my idea is to look if we can simply **do away** with **large**
**hash tables** and **collision resolution** altogether, by simply *considering*
*identical hash keys* as *identical labels*. This may lead to *bad communities*,
but that is what this experiment is for. If they do yield good communities it
can be a big win in performance. Note that such a scheme is only likely to cause
issues only in the *first few iterations*, when we have a large number of
communities. In addition, as there is potential for multiple communities to be
combined, we will consider the *new community* to be the *hash key*.

In this experiment we try five different hash functions, and adjust the capacity
of accumulator hashtable from `509` to `4093` in multiples of 2. This capacity
is always set to the highest prime number below a power of 2.  We choose the
Louvain *parameters* as `resolution = 1.0`, `tolerance = 0` and `passTolerance = 0.5`.
In addition we limit the maximum number of iterations in a single
local-moving phase with `maxIterations = 500`, and limit the maximum number of
passes with `maxPasses = 500`. We run the Louvain algorithm until convergence
(or until the maximum limits are exceeded), and measure the **time** **taken**
for the *computation* (performed 5 times for averaging), the **modularity
score**, the **total number of iterations** (in the *local-moving* *phase*), and
the number of **passes**. This is repeated for *seventeen* different graphs.

We observe that `magic` **hash function** with an **accumulator hashtable
capacity** of `4093` appears to perform the **best** among the limited capacity
hashtable approaches. However it is still worse than the default approach of
Louvain algorithm. Therefore it seems using a limited capacity accumulator
hashtable for the Louvain algorithm is not useful, i.e., **we must stick to
full** **capacity accumulator hashtable**.

All outputs are saved in a [gist] and a small part of the output is listed here.
Some [charts] are also included below, generated from [sheets]. The input data
used for this experiment is available from the [SuiteSparse Matrix Collection].
This experiment was done with guidance from [Prof. Kishore Kothapalli] and
[Prof. Dip Sankar Banerjee].

<br>

```bash
$ g++ -std=c++17 -O3 main.cxx
$ ./a.out ~/data/web-Stanford.mtx
$ ./a.out ~/data/web-BerkStan.mtx
$ ...

# Loading graph /home/subhajit/data/web-Stanford.mtx ...
# order: 281903 size: 2312497 [directed] {}
# order: 281903 size: 3985272 [directed] {} (symmetricize)
# [-0.000497 modularity] noop
# [13454.390 ms; 0287 iterations; 009 passes; 0.923316 modularity] louvainSeq
# [45147.336 ms; 2000 iterations; 004 passes; 0.767695 modularity] louvainSeqDivision {acc_capacity=509}
# [09683.511 ms; 0121 iterations; 004 passes; 0.784888 modularity] louvainSeqDivision {acc_capacity=1021}
# [05719.490 ms; 0076 iterations; 005 passes; 0.798086 modularity] louvainSeqDivision {acc_capacity=2039}
# [09164.935 ms; 0127 iterations; 006 passes; 0.809104 modularity] louvainSeqDivision {acc_capacity=4093}
# [07792.102 ms; 0153 iterations; 005 passes; 0.769352 modularity] louvainSeqMultiplication {acc_capacity=509}
# [05540.244 ms; 0109 iterations; 004 passes; 0.786598 modularity] louvainSeqMultiplication {acc_capacity=1021}
# [03712.969 ms; 0076 iterations; 005 passes; 0.795170 modularity] louvainSeqMultiplication {acc_capacity=2039}
# [13522.579 ms; 0269 iterations; 004 passes; 0.810318 modularity] louvainSeqMultiplication {acc_capacity=4093}
# [27546.578 ms; 2000 iterations; 004 passes; 0.769995 modularity] louvainSeqDjb2 {acc_capacity=509}
# [04609.747 ms; 0096 iterations; 004 passes; 0.785100 modularity] louvainSeqDjb2 {acc_capacity=1021}
# [05895.703 ms; 0124 iterations; 005 passes; 0.800249 modularity] louvainSeqDjb2 {acc_capacity=2039}
# [03982.653 ms; 0089 iterations; 006 passes; 0.814499 modularity] louvainSeqDjb2 {acc_capacity=4093}
# [08986.458 ms; 0184 iterations; 004 passes; 0.772474 modularity] louvainSeqSdbm {acc_capacity=509}
# [05996.684 ms; 0124 iterations; 004 passes; 0.781377 modularity] louvainSeqSdbm {acc_capacity=1021}
# [03235.132 ms; 0069 iterations; 005 passes; 0.801154 modularity] louvainSeqSdbm {acc_capacity=2039}
# [06004.832 ms; 0127 iterations; 005 passes; 0.804691 modularity] louvainSeqSdbm {acc_capacity=4093}
# [06458.338 ms; 0128 iterations; 004 passes; 0.767416 modularity] louvainSeqMagic {acc_capacity=509}
# [05665.010 ms; 0113 iterations; 004 passes; 0.783085 modularity] louvainSeqMagic {acc_capacity=1021}
# [03093.988 ms; 0065 iterations; 005 passes; 0.801779 modularity] louvainSeqMagic {acc_capacity=2039}
# [04554.626 ms; 0094 iterations; 005 passes; 0.814386 modularity] louvainSeqMagic {acc_capacity=4093}
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [16684.238 ms; 0404 iterations; 009 passes; 0.935729 modularity] louvainSeq
# [50433.898 ms; 0285 iterations; 004 passes; 0.834423 modularity] louvainSeqDivision {acc_capacity=509}
# [32044.062 ms; 0177 iterations; 005 passes; 0.842737 modularity] louvainSeqDivision {acc_capacity=1021}
# [46297.711 ms; 0256 iterations; 004 passes; 0.856244 modularity] louvainSeqDivision {acc_capacity=2039}
# ...
```

[![](https://i.imgur.com/EXTesTP.png)][sheetp]
[![](https://i.imgur.com/CgfPIuj.png)][sheetp]
[![](https://i.imgur.com/9zBlMnK.png)][sheetp]
[![](https://i.imgur.com/tcnvEB7.png)][sheetp]

<br>
<br>


## References

- [Fast unfolding of communities in large networks; Vincent D. Blondel et al. (2008)](https://arxiv.org/abs/0803.0476)
- [Community Detection on the GPU; Md. Naim et al. (2017)](https://arxiv.org/abs/1305.2006)
- [Scalable Static and Dynamic Community Detection Using Grappolo; Mahantesh Halappanavar et al. (2017)](https://ieeexplore.ieee.org/document/8091047)
- [From Louvain to Leiden: guaranteeing well-connected communities; V.A. Traag et al. (2019)](https://www.nature.com/articles/s41598-019-41695-z)
- [CS224W: Machine Learning with Graphs | Louvain Algorithm; Jure Leskovec (2021)](https://www.youtube.com/watch?v=0zuiLBOIcsw)
- [The University of Florida Sparse Matrix Collection; Timothy A. Davis et al. (2011)](https://doi.org/10.1145/2049662.2049663)

<br>
<br>

[![](https://i.imgur.com/x7jvUkt.jpg)](https://www.youtube.com/watch?v=I-PIFYTbBe0)<br>


[(1)]: https://github.com/puzzlef/labelrank-adjust-accumulator-capacity
[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
[gist]: https://gist.github.com/wolfram77/7413ec304193901a238108378b7d06e9
[charts]: https://imgur.com/a/HxCnM2z
[sheets]: https://docs.google.com/spreadsheets/d/1NrCPwU25peLR8Yl29BAMfrNyL2zlIHvgEF4EoNvFyUg/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vTFPR38ZidzcBULWggiCGnRehCBi3dY9-cUulJI9YQ8GOx_UIxwGG0UWymZF5vfv2uHkyi6Y82nC8Ep/pubhtml
