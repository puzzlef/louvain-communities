Effect of iteratively adjusting tolerance of the Louvain algorithm for community
detection.

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

In this experiment we adjust `tolerance` in two different ways. First, we change
the initial value of `tolerance` from `1e-00` to `1e-12` in steps of `10`. For
each initial value of `tolerance`, we adjust the rate at which we decline
tolerance between each pass (`toleranceDeclineFactor`) from `10` to `10000`. We
compare the results, both in terms of quality (modularity) of communities
obtained, and performance. We choose the remaining Louvain *parameters* as
`resolution = 1.0` and `passTolerance = 0.0`. In addition we limit the maximum
number of iterations in a single local-moving phase with `maxIterations = 500`,
and limit the maximum number of passes with `maxPasses = 500`. We run the
Louvain algorithm until convergence (or until the maximum limits are exceeded),
and measure the **time** **taken** for the *computation* (performed 5 times for
averaging), the **modularity score**, the **total number of iterations** (in the
*local-moving* *phase*), and the number of **passes**. This is repeated for
*seventeen* different graphs.

From the results, we observe that an initial **tolerance** of `1e-2` yields
communities with the best possible modularity while requiring the least
computation time. In addition, increasing the `toleranceDeclineFactor`
increases the computation time (as expected), but does not seem to impact
resulting modularity. Therefore choosing a **toleranceDeclineFactor** of `10`
would be a good idea.

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
# [00747.516 ms; 0016 iters.; 009 passes; 0.922466815 modularity] louvainSeq {tolerance: 1.0e+00, tol_dec_factor: 1.0e+01}
# [00591.458 ms; 0021 iters.; 009 passes; 0.923401713 modularity] louvainSeq {tolerance: 1.0e-01, tol_dec_factor: 1.0e+01}
# [00616.070 ms; 0025 iters.; 009 passes; 0.923382580 modularity] louvainSeq {tolerance: 1.0e-02, tol_dec_factor: 1.0e+01}
# ...
# [09872.146 ms; 0287 iters.; 009 passes; 0.923316002 modularity] louvainSeq {tolerance: 1.0e-10, tol_dec_factor: 1.0e+04}
# [09877.723 ms; 0287 iters.; 009 passes; 0.923316002 modularity] louvainSeq {tolerance: 1.0e-11, tol_dec_factor: 1.0e+04}
# [09907.396 ms; 0287 iters.; 009 passes; 0.923316002 modularity] louvainSeq {tolerance: 1.0e-12, tol_dec_factor: 1.0e+04}
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [01299.983 ms; 0017 iters.; 009 passes; 0.934353232 modularity] louvainSeq {tolerance: 1.0e+00, tol_dec_factor: 1.0e+01}
# [01059.621 ms; 0020 iters.; 009 passes; 0.937566638 modularity] louvainSeq {tolerance: 1.0e-01, tol_dec_factor: 1.0e+01}
# [01169.751 ms; 0028 iters.; 009 passes; 0.935839474 modularity] louvainSeq {tolerance: 1.0e-02, tol_dec_factor: 1.0e+01}
# ...
```

[![](https://i.imgur.com/weoGrJ1.png)][sheetp]
[![](https://i.imgur.com/gdrqi0W.png)][sheetp]
[![](https://i.imgur.com/l0GiYbN.png)][sheetp]
[![](https://i.imgur.com/nh1Ifjs.png)][sheetp]

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

[![](https://i.imgur.com/Pylv6Gc.jpg)](http://www.youtube.com/watch?v=m8fPyvA0QRE)<br>
[![DOI](https://zenodo.org/badge/516260025.svg)](https://zenodo.org/badge/latestdoi/516260025)


[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
[gist]: https://gist.github.com/wolfram77/75adaf8ef100b304ed52602fd68454b2
[charts]: https://imgur.com/a/9V03o4o
[sheets]: https://docs.google.com/spreadsheets/d/1Atv-xPmh7DrizWLT6Zis2OcoaDexNGX6edOqjVQu6f0/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vRkVmIDWzawEfwjM_JzUnG4Dtomkm9ze7aKEqEuCPree_qzf70npIm27nNXmzuE5QgocZ-XZcYxYSNh/pubhtml
