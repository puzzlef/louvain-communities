Effect of adjusting start tolerance and tolerance-norm of the
[Louvain algorithm] for [community detection].

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

In this experiment we change the initial value of `tolerance` (for local-moving
phase) from `1e-00` to `1e-6` in steps of `10`. For each initial value of
`tolerance`, we use a tolerance-norm of `L1`, `L2`, or `L∞`. We compare the
results, both in terms of quality (modularity) of communities obtained, and
performance. We choose the remaining Louvain *parameters* as `resolution = 1.0`,
`toleranceDeclineFactor = 10` (the rate at which we recude tolerance after every
pass), and `passTolerance = 0.0`. In addition we limit the maximum number of
iterations in a single local-moving phase with `maxIterations = 500`, and limit
the maximum number of passes with `maxPasses = 500`. We run the Louvain
algorithm until convergence (or until the maximum limits are exceeded), and
measure the **time taken** for the *computation* (performed 5 times for
averaging), the **modularity score**, the **total number of iterations** (in the
*local-moving* *phase*), and the number of **passes**. This is repeated for
*seventeen* different graphs.

From the results, we make the following observations. A tolerance-norm of `L1`
converges the fastest, followed by `L∞`, and then `L2` (except for initial
`tolerance` below `10^-2`). This could be due to delta-modularity for any vertex
being small, so that squiring it (as with `L2-norm`) reduces the net error
significantly. In general we observe that the modularity obtained with all
tolerance-norms is the same, but for some graphs, best modularity is achieved
with an initial `tolerance` of `10^-5` with `L∞-norm`, and an initial tolerance
of `> 10^-6` with `L2-norm`. As `L∞-norm` considers only the max error, we
believe it would be a suitable tolerance-norm of choice for dynamic Louvain
algorithm. Hence, we ask you to consider an **initial tolerance of 10^-5 with**
**L∞-norm** to be the **suitable choice** in the general case.

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
# [0e+00 batch_size; 0 batch_count; 00792.589 ms; 0025 iters.; 009 passes; 0.923382580 modularity] louvainSeqLast
# [0e+00 batch_size; 0 batch_count; 00573.409 ms; 0004 iters.; 001 passes; 0.766543329 modularity] louvainSeqFirst
# [5e+02 batch_size; 1 batch_count; 00224.703 ms; 0004 iters.; 004 passes; 0.914939582 modularity] louvainSeqDynamicLast
# [5e+02 batch_size; 1 batch_count; 00480.665 ms; 0025 iters.; 009 passes; 0.923243225 modularity] louvainSeqDynamicFirst
# [5e+02 batch_size; 2 batch_count; 00227.230 ms; 0004 iters.; 004 passes; 0.914955676 modularity] louvainSeqDynamicLast
# ...
# [-1e+05 batch_size; 4 batch_count; 00397.172 ms; 0011 iters.; 006 passes; 0.876155496 modularity] louvainSeqDynamicFirst
# [-1e+05 batch_size; 5 batch_count; 00206.570 ms; 0004 iters.; 004 passes; 0.869377553 modularity] louvainSeqDynamicLast
# [-1e+05 batch_size; 5 batch_count; 00406.253 ms; 0012 iters.; 006 passes; 0.876216054 modularity] louvainSeqDynamicFirst
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [0e+00 batch_size; 0 batch_count; 01248.049 ms; 0028 iters.; 009 passes; 0.935839474 modularity] louvainSeqLast
# [0e+00 batch_size; 0 batch_count; 00926.964 ms; 0005 iters.; 001 passes; 0.798873782 modularity] louvainSeqFirst
# [5e+02 batch_size; 1 batch_count; 00527.935 ms; 0003 iters.; 003 passes; 0.932618558 modularity] louvainSeqDynamicLast
# [5e+02 batch_size; 1 batch_count; 00884.497 ms; 0025 iters.; 009 passes; 0.935987055 modularity] louvainSeqDynamicFirst
# [5e+02 batch_size; 2 batch_count; 00529.566 ms; 0003 iters.; 003 passes; 0.932615817 modularity] louvainSeqDynamicLast
# ...
```

[![](https://i.imgur.com/MZVDsqq.png)][sheetp]
[![](https://i.imgur.com/IhJqxZG.png)][sheetp]
[![](https://i.imgur.com/bgDmlVZ.png)][sheetp]
[![](https://i.imgur.com/DKhTOB1.png)][sheetp]

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

[![](https://i.imgur.com/9HITKSz.jpg)](https://www.youtube.com/watch?v=wCUV6N4Qtew)<br>
[![ORG](https://img.shields.io/badge/org-puzzlef-green?logo=Org)](https://puzzlef.github.io)


[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
[gist]: https://gist.github.com/wolfram77/b2ac8e73457b2be74716707b8e56bbd6
[charts]: https://imgur.com/a/LNCHoD5
[sheets]: https://docs.google.com/spreadsheets/d/1HG88om3r0z77BZUQaDsYaQtlH0TYEO1GCFOvfCEGNb4/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vREObOkBKZhuBSLUq90YPwIAHEuAXWac3_MIhfu_X5zXH5AJ8oP552Dhwx7dITBHmUUK-W8OEkQTklb/pubhtml
