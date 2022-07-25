Comparison of ordered vs unordered vertex processing in Louvain algorithm for
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

There exist two possible approaches of vertex processing with the Louvain
algorithm: *ordered* and *unordered*. With the **ordered approach** (original
paper's approach), the *local-moving phase* is **performed sequentially** upon
each vertex such that the moving of a *previous vertex* in the graph *affects*
the decision of the *current vertex* being processed. On the other hand, with
the **unordered approach** the moving of a *previous vertex* in the graph *does*
*not affect* the decision of movement for the *current vertex*. This *unordered*
*approach* (aka *relaxed approach*) is made possible by maintaining the
*previous* and the *current community membership status* of each vertex (along
with associated community information), and is the approach **followed by**
**parallel Louvain implementation** on the CPU as well as the GPU. We are
interested in looking at *performance/modularity penalty* (if any) associated
with the *unordered approach*.

In this experiment we compare the ordered and unordered vertex processing
approaches for the Louvain algorithm, both in terms of quality (modularity) of
communities obtained, and performance. We choose the Louvain *parameters* as
`resolution = 1.0`, `tolerance = 0.0` (for local-moving phase), `passTolerance = 0.0`
(when passes stop). In addition we limit the maximum number of iterations in
a single local-moving phase with `maxIterations = 500`, and limit the maximum
number of passes with `maxPasses = 500`. We run the Louvain algorithm until
convergence (or until the maximum limits are exceeded), and measure the **time**
**taken** for the *computation* (performed 5 times for averaging), the
**modularity score**, the **total number of iterations** (in the *local-moving*
*phase*), and the number of **passes**. This is repeated for *seventeen*
different graphs.

From the results, we observe that **both** the *ordered* and the *unordered*
vertex processing approaches of the Louvain algorithm are able to provide
communities **of equivalent quality** in terms of **modularity**, with the
*ordered approach* providing *slightly higher quality communities* for certain
graphs. However, there is a *significant performance gap* between the *ordered*
and the *unordered* approaches. The **unordered approach** is **orders of**
**magnitude slower** than the *ordered approach* in terms of the **total time**
**taken**, as well as the **total number of iterations** of the *local-moving*
*phase* (which is the *most expensive part* of the algorithm). We also observe
that the unordered algorithm requires **more number of passes** for convergence.
Choosing *non-zero tolerances* for the algorithm will likely *reduce* this
significant gap between the two approaches, but the **ordered approach is**
**clearly superior** to the unordered approach. We therefore conclude that
**partially ordered approaches** for vertex processing are **likely to provide**
**good performance improvements** over fully unordered approaches **for parallel**
**implementations** of the Louvain algorithm. *Vertex ordering* via *graph*
*coloring* has been explored by Halappanavar et al.

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
# [09929.730 ms; 0287 iters.; 009 passes; 0.923316 modularity] louvainSeqOrdered
# [29279.896 ms; 6003 iters.; 014 passes; 0.924421 modularity] louvainSeqUnordered
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [13976.750 ms; 0404 iters.; 009 passes; 0.935729 modularity] louvainSeqOrdered
# [49975.512 ms; 5503 iters.; 013 passes; 0.937115 modularity] louvainSeqUnordered
# ...
```

[![](https://i.imgur.com/17Y4dSG.png)][sheetp]
[![](https://i.imgur.com/8Gr6Jxg.png)][sheetp]
[![](https://i.imgur.com/8GJl7pp.png)][sheetp]
[![](https://i.imgur.com/IRlfApi.png)][sheetp]

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

[![](https://i.imgur.com/CSFI99v.jpg)](https://www.youtube.com/watch?v=soFR3Uf6Aoo)<br>
[![DOI](https://zenodo.org/badge/516219283.svg)](https://zenodo.org/badge/latestdoi/516219283)


[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
[gist]: https://gist.github.com/wolfram77/20646e8a76711fdac463eabc92a306ff
[charts]: https://imgur.com/a/DJVI4SV
[sheets]: https://docs.google.com/spreadsheets/d/1s9d2NGOrQT9_0uixIBKoTSSlERcr2PTVxJlhaWZ0st4/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vQ9L0XHRdEJpkWvWpWCxPr8M7CajTfSIXS_Q2-NAVpGYR-Rr83QDu3ZON88Y32yVGCF1iwo14RniOsm/pubhtml
