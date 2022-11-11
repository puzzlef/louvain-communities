Comparing static vs dynamic approaches of the [Louvain algorithm] for
[community detection].

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
significantly affecting the modularity of obtained communities. In this
experiment, we compare the performance of *three different types* of **dynamic
Louvain** with respect to the *static* version.

**Naive dynamic**:
- We start with previous community membership of each vertex (instead of each vertex its own community).

**Delta screening**:
- All edge batches are undirected, and sorted by source vertex-id.
- For edge additions across communities with source vertex `i` and highest modularity changing edge vertex `j*`,
  `i`'s neighbors and `j*`'s community is marked as affected.
- For edge deletions within the same community `i` and `j`,
  `i`'s neighbors and `j`'s community is marked as affected.

**Frontier-based**:
- All source and destination vertices are marked as affected for insertions and deletions.
- For edge additions across communities with source vertex `i` and destination vertex `j`,
  `i` is marked as affected.
- For edge deletions within the same community `i` and `j`,
  `i` is marked as affected.
- Vertices whose communities change in local-moving phase have their neighbors marked as affected.

First, we compute the community membership of each vertex using the static
Louvain algorithm. We then generate *batches* of *insertions* *(+)* and
*deletions (-)* of edges of sizes 500, 1000, 5000, ... 100000. For each batch
size, we generate *five* different batches for the purpose of *averaging*. Each
batch of edges (insertion / deletion) is generated randomly such that the
selection of each vertex (as endpoint) is *equally probable*. We choose the
Louvain *parameters* as `resolution = 1.0`, `tolerance = 1e-2` (for local-moving
phase) with *tolerance* decreasing after every pass by a factor of
`toleranceDeclineFactor = 10`, and a `passTolerance = 0.0` (when passes stop).
In addition we limit the maximum number of iterations in a single local-moving
phase with `maxIterations = 500`, and limit the maximum number of passes with
`maxPasses = 500`. We run the Louvain algorithm until convergence (or until the
maximum limits are exceeded), and measure the **time taken** for the
*computation* (performed 5 times for averaging), the **modularity score**, the
**total number of iterations** (in the *local-moving* *phase*), and the number
of **passes**. This is repeated for *seventeen* different graphs.

From the results, we make make the following observations. The frontier-based
dynamic approach converges the fastest, which obtaining communities with only
slightly lower modularity than other approaches. We also observe that
delta-screening based dynamic Louvain algorithm has the same performance as that
of the naive dynamic approach. Therefore, **frontier-based dynamic Louvain**
would be the **best choice**.

All outputs are saved in a [gist] and a small part of the output is listed here.
Some [charts] are also included below, generated from [sheets]. The input data
used for this experiment is available from the [SuiteSparse Matrix Collection].
This experiment was done with guidance from [Prof. Kishore Kothapalli] and
[Prof. Dip Sankar Banerjee].


[Louvain algorithm]: https://en.wikipedia.org/wiki/Louvain_method
[community detection]: https://en.wikipedia.org/wiki/Community_search

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
# [0e+00 batch_size; 00442.963 ms; 0025 iters.; 009 passes; 0.923382580 modularity] louvainSeqStatic
# [5e+02 batch_size; 00394.755 ms; 0024 iters.; 008 passes; 0.923357189 modularity] louvainSeqStatic
# [5e+02 batch_size; 00138.878 ms; 0004 iters.; 004 passes; 0.914949775 modularity] louvainSeqNaiveDynamic
# [5e+02 batch_size; 00136.585 ms; 0004 iters.; 004 passes; 0.914949775 modularity] louvainSeqDynamicDeltaScreening
# [5e+02 batch_size; 00106.190 ms; 0003 iters.; 003 passes; 0.913411021 modularity] louvainSeqDynamicFrontier
# [5e+02 batch_size; 00398.024 ms; 0031 iters.; 009 passes; 0.923311889 modularity] louvainSeqStatic
# [5e+02 batch_size; 00132.255 ms; 0003 iters.; 003 passes; 0.914943874 modularity] louvainSeqNaiveDynamic
# [5e+02 batch_size; 00126.150 ms; 0003 iters.; 003 passes; 0.914943874 modularity] louvainSeqDynamicDeltaScreening
# [5e+02 batch_size; 00105.081 ms; 0003 iters.; 003 passes; 0.913414836 modularity] louvainSeqDynamicFrontier
# ...
# [1e+05 batch_size; 00459.907 ms; 0017 iters.; 006 passes; 0.913970351 modularity] louvainSeqStatic
# [1e+05 batch_size; 00166.834 ms; 0005 iters.; 005 passes; 0.912481785 modularity] louvainSeqNaiveDynamic
# [1e+05 batch_size; 00170.689 ms; 0005 iters.; 005 passes; 0.912481785 modularity] louvainSeqDynamicDeltaScreening
# [1e+05 batch_size; 00174.321 ms; 0006 iters.; 006 passes; 0.912482142 modularity] louvainSeqDynamicFrontier
# [-5e+02 batch_size; 00401.930 ms; 0027 iters.; 009 passes; 0.923128545 modularity] louvainSeqStatic
# [-5e+02 batch_size; 00136.292 ms; 0004 iters.; 004 passes; 0.914732695 modularity] louvainSeqNaiveDynamic
# [-5e+02 batch_size; 00133.254 ms; 0004 iters.; 004 passes; 0.914732695 modularity] louvainSeqDynamicDeltaScreening
# [-5e+02 batch_size; 00096.799 ms; 0002 iters.; 002 passes; 0.913195193 modularity] louvainSeqDynamicFrontier
# ...
# [-1e+05 batch_size; 00389.311 ms; 0017 iters.; 006 passes; 0.877391517 modularity] louvainSeqStatic
# [-1e+05 batch_size; 00134.187 ms; 0004 iters.; 004 passes; 0.869822621 modularity] louvainSeqNaiveDynamic
# [-1e+05 batch_size; 00132.575 ms; 0004 iters.; 004 passes; 0.869822621 modularity] louvainSeqDynamicDeltaScreening
# [-1e+05 batch_size; 00129.124 ms; 0004 iters.; 004 passes; 0.869453311 modularity] louvainSeqDynamicFrontier
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [0e+00 batch_size; 00729.297 ms; 0028 iters.; 009 passes; 0.935839474 modularity] louvainSeqStatic
# [5e+02 batch_size; 00742.073 ms; 0029 iters.; 009 passes; 0.935999393 modularity] louvainSeqStatic
# [5e+02 batch_size; 00225.180 ms; 0003 iters.; 003 passes; 0.932615280 modularity] louvainSeqNaiveDynamic
# [5e+02 batch_size; 00224.688 ms; 0003 iters.; 003 passes; 0.932615280 modularity] louvainSeqDynamicDeltaScreening
# [5e+02 batch_size; 00189.586 ms; 0003 iters.; 003 passes; 0.932642102 modularity] louvainSeqDynamicFrontier
# ...
```

[![](https://i.imgur.com/M0gLGLc.png)][sheetp]
[![](https://i.imgur.com/r3hwsPc.png)][sheetp]
[![](https://i.imgur.com/2cg4fpX.png)][sheetp]
[![](https://i.imgur.com/pMZIbKL.png)][sheetp]

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

[![](https://i.imgur.com/UGB0g2L.jpg)](https://www.youtube.com/watch?v=pIF3wOet-zw)<br>
[![ORG](https://img.shields.io/badge/org-puzzlef-green?logo=Org)](https://puzzlef.github.io)
[![DOI](https://zenodo.org/badge/538336155.svg)](https://zenodo.org/badge/latestdoi/538336155)


[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
[gist]: https://gist.github.com/wolfram77/de2c1e1c8f6efb7f4053a122b688c7a7
[charts]: https://imgur.com/a/0IixyhS
[sheets]: https://docs.google.com/spreadsheets/d/1U8cdi0Y9i-Pl0SkUdSsuM87Khw0Jn6SCM7u6qlVp--Y/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vQjZGdwSy2Cd5mojCQgvvl0P5eaZBjJwhIBqcX-RN5MfeFVl9V2o9G4XCjR2_qaH_mpeBSm7eMIhSeQ/pubhtml
