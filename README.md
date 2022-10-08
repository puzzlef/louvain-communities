Effect of using a simple weight-based community choice for first iteration for
local-moving phase of [Louvain algorithm] for [community detection].

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

The **first iteration** of **local-moving phase** can be simplified to **not**
**require** the use of an **accumulator hashtable**. This is because in the *first*
*iteration*, each vertex is its *own community* (*no* accumulation is needed).
However, this **must** be performed in an **unordered fashion**, i.e., the
community choosing of one vertex would *not* affect the community choosing of
*another vertex* (as it normally does with local-moving phase, as we are using
*ordered* Louvain algorithm). We anticipate that this may help reducing the time
taken for convergence, while still taking almost the same time for convergence.
This *can be useful on the GPU* when using *limited capacity accumulator*
*hashtable* as it can *reduce the number of communities present* and thus help
limit the impact of the inaccuracies of a limited capacity hashtable. Note that
we apply this simiplification **only** for the **first iteration of the first**
**local-moving phase** of the algorithm.

In this experiment we compare the standard and simplified first move approaches
for the Louvain algorithm, both in terms of quality (modularity) of communities
obtained, and performance. We choose the Louvain *parameters* as
`resolution = 1.0`, `tolerance = 1e-2` (for local-moving phase) with *tolerance*
decreasing after every pass by a factor of `toleranceDeclineFactor = 10`, and a
`passTolerance = 0.0` (when passes stop). In addition we limit the maximum
number of iterations in a single local-moving phase with `maxIterations = 500`,
and limit the maximum number of passes with `maxPasses = 500`. We run the
Louvain algorithm until convergence (or until the maximum limits are exceeded),
and measure the **time taken** for the *computation* (performed 5 times for
averaging), the **modularity score**, the **total number of iterations** (in the
*local-moving phase*), and the number of **passes**. We also track the *time*,
*iterations*, and *modularity* obtained from the first local-moving phase on the
CPU and the GPU. This is repeated for *seventeen* different graphs.

From the results, we observe that **using a simple first move** requires in
general somewhat **more time (and iterations) to converge** than the standard
approach, but may also return communities of **slightly higher modularity**. We
therefore conclude that using a **simple first move is not useful on the CPU**,
but it may be useful on the GPU for the reasons mentioned above.

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
# [00420.826 ms; 0025 iters.; 008 passes; 0.923382580 modularity] louvainSeq
# [00479.680 ms; 0027 iters.; 008 passes; 0.927262425 modularity] louvainSeqFirst
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [00687.219 ms; 0028 iters.; 008 passes; 0.935839474 modularity] louvainSeq
# [00729.971 ms; 0031 iters.; 009 passes; 0.935943425 modularity] louvainSeqFirst
#
# ...
```

[![](https://i.imgur.com/XFX1S8h.png)][sheetp]
[![](https://i.imgur.com/8ouL5ar.png)][sheetp]
[![](https://i.imgur.com/5fGEYFF.png)][sheetp]
[![](https://i.imgur.com/n5Fsm7o.png)][sheetp]

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

[![](https://i.imgur.com/RbvQPwO.jpg)](https://www.youtube.com/watch?v=XE_z2X98a_Y)<br>
[![ORG](https://img.shields.io/badge/org-puzzlef-green?logo=Org)](https://puzzlef.github.io)
[![DOI](https://zenodo.org/badge/547428545.svg)](https://zenodo.org/badge/latestdoi/547428545)


[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
[gist]: https://gist.github.com/wolfram77/b33a74f40d32a9c3fed12cd348853cd0
[charts]: https://imgur.com/a/2Rzc3dP
[sheets]: https://docs.google.com/spreadsheets/d/1L3M8tjHPfmo69OinHnNMRuOZIu1aSDHHml2o97-eILo/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vT7lULjkO_3TTh8tQyxWznBIgip7P63VHAVoneai9bTwRnx0zYcAEu6CMqYJMv0VeVIxSSDKKBSkqQq/pubhtml
