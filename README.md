Effect of adjusting capacity of collision handled accumulator hashtable of the
Louvain algorithm for community detection.

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

Originally, my idea was to look if we can simply **do away** with **large**
**hash tables** and **collision resolution** altogether, by simply *considering*
*identical hash keys* as *identical labels*. However, either due to a mistake of
my own or due to the limit to the total number of communities enforced by the
lack of collision resolution, the quality of communitied (based on modularity)
were really bad. Therefore, with this experiment, we put in place **proper**
**collision resolution** *(algorithmically)*, but still *restrict the capacity of the*
*accumulator hashtable*. As you will read later, this gives us good results.

In this experiment we adjust the capacity of accumulator hashtable from `2` to
`4093` in multiples of 2. This capacity is always set to the highest prime
number below a power of 2.  We choose the Louvain *parameters* as `resolution = 1.0`,
`tolerance = 0` and `passTolerance = 0.5`.  In addition we limit the maximum
number of iterations in a single local-moving phase with `maxIterations = 500`,
and limit the maximum number of passes with `maxPasses = 500`. We run the
Louvain algorithm until convergence (or until the maximum limits are exceeded),
and measure the **time** **taken** for the *computation* (performed 5 times for
averaging), the **modularity score**, the **total number of iterations** (in the
*local-moving* *phase*), and the number of **passes**. This is repeated for
*seventeen* different graphs.

From the results we observe that we can achieve good modularity with accumulator
capacity of `7+`. We are able to obtain such good quality communities within the
least amount of time with an accumulator capacity of `31+`, and with the least
number of iterations with an accumulator capacity of `127+`. We therefore conclude
that using a limited capacity accumulator hashtable of `251` with `~50%` occupancy
would be a suitable for community detection using the Louvain algorithm on limited
memory parallel devices such as the GPU. We may also go down to a hashtable capacity
of `61` and still achieve good communities in a small amount of time. If time is not
a concern (due to high-degree of parallelism achieved with small hashtables), an
accumulator capacity of `13` may also be used. If minimization of collisions is not
important (thanks to parallel memory scanning), then an accumulator capacity of `7`
may also be attempted.

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
# [13021.103 ms; 0287 iterations; 009 passes; 0.923316 modularity] louvainSeq
# [27113.324 ms; 1026 iterations; 009 passes; 0.905717 modularity] louvainSeq {acc_capacity=2}
# [26685.023 ms; 1070 iterations; 009 passes; 0.917865 modularity] louvainSeq {acc_capacity=3}
# [25197.309 ms; 0807 iterations; 009 passes; 0.926431 modularity] louvainSeq {acc_capacity=7}
# [12712.631 ms; 0672 iterations; 009 passes; 0.927234 modularity] louvainSeq {acc_capacity=13}
# [12141.696 ms; 0290 iterations; 009 passes; 0.926465 modularity] louvainSeq {acc_capacity=31}
# [13137.970 ms; 0290 iterations; 009 passes; 0.923675 modularity] louvainSeq {acc_capacity=61}
# [13357.952 ms; 0290 iterations; 009 passes; 0.923293 modularity] louvainSeq {acc_capacity=127}
# [13179.921 ms; 0287 iterations; 009 passes; 0.923329 modularity] louvainSeq {acc_capacity=251}
# [12717.114 ms; 0287 iterations; 009 passes; 0.923321 modularity] louvainSeq {acc_capacity=509}
# [12869.202 ms; 0287 iterations; 009 passes; 0.923316 modularity] louvainSeq {acc_capacity=1021}
# [12868.821 ms; 0287 iterations; 009 passes; 0.923316 modularity] louvainSeq {acc_capacity=2039}
# [13307.006 ms; 0287 iterations; 009 passes; 0.923316 modularity] louvainSeq {acc_capacity=4093}
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [17659.744 ms; 0404 iterations; 009 passes; 0.935729 modularity] louvainSeq
# [41365.871 ms; 1525 iterations; 009 passes; 0.876490 modularity] louvainSeq {acc_capacity=2}
# [40255.496 ms; 1112 iterations; 009 passes; 0.902975 modularity] louvainSeq {acc_capacity=3}
# [41355.492 ms; 1260 iterations; 009 passes; 0.934294 modularity] louvainSeq {acc_capacity=7}
# ...
```

[![](https://i.imgur.com/4nOYacx.png)][sheetp]
[![](https://i.imgur.com/YAinlLo.png)][sheetp]
[![](https://i.imgur.com/ZbQbAYU.png)][sheetp]
[![](https://i.imgur.com/qlBnyKc.png)][sheetp]

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

[![](https://i.imgur.com/Q9a7QEJ.jpg)](https://www.youtube.com/watch?v=7ELkfd0560M)<br>
[![ORG](https://img.shields.io/badge/org-puzzlef-green?logo=Org)](https://puzzlef.github.io)
[![DOI](https://zenodo.org/badge/523712209.svg)](https://zenodo.org/badge/latestdoi/523712209)


[(1)]: https://github.com/puzzlef/labelrank-adjust-accumulator-capacity
[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
[gist]: https://gist.github.com/wolfram77/5d7ce8a692f492c22978c0136bf1c433
[charts]: https://imgur.com/a/zNyK91h
[sheets]: https://docs.google.com/spreadsheets/d/1EXI6sgLTqN_l6ov6z7VlnMSp0e7aZJJ482JsGy6QZ9s/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vRmA3B-0GEtd25DFjg-QlMYmu6qBvRRsAHop29FBRu_zh_eWWnZr-i0t8lS1yb-T8kwcAWLIRyTSKhM/pubhtml
