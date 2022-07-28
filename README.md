Effect of adjusting accumulator hashtable capacity of the Louvain algorithm for
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

We however obtain results that are significantly different from our expectations.
We observe that choosing a **lower accumulator hastable capacity** causes the
computation to **converge in much longer time**, **require a larger number of**
**iterations**, a **smaller number of passes**, and **worse modularity**. Choosing
an accumulator hashtable capacity of `4093` also does not seem to provide any
advantage over a full-size array based accumulator (except on some graphs, i.e.,
`soc-LiveJournal1`, `coPapersCiteseer`, `coPapersDBLP`). This is hopefully
interesting as we observe that this approach of using a limit capacity accumulator
hashtable seems to work well for the LabelRank algorithm [(1)].

However with higher accumulator labelset capacities, the time taken
may increase beyond the time required for a full size accumulator labelset. This
is because of the additional modulus (`%`) operator required with a limited
capacity accumulator labelset (my expectaction is that this would not
significantly affect performance in a GPU). Again, a similar effect is observed
with **modularity** (in the average case), which **increases with increasing**
**accumulator labelset capacity**. It appears that using an **accumulator labelset**
**capacity** of `61 / 127` would **yield a good enough modularity**. In some
cases, using a smaller accumulator labelset capacity yeilds an even better
modularity than full-size labelsets (but these are exception cases i think).
Note that choices might differ if a different *labelset capacity* is used. It
would be interesting to implement this kind of collision-ignoring hash table on
a GPU, and observe its impact on LabelRank as well as the Louvain algorithm for
community detection.

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
# [10130.134 ms; 0287 iterations; 009 passes; 0.923316 modularity] louvainSeq
# [51052.770 ms; 1000 iterations; 002 passes; 0.030269 modularity] louvainSeq {acc_capacity=2}
# [48293.227 ms; 1000 iterations; 002 passes; 0.151540 modularity] louvainSeq {acc_capacity=3}
# [37588.156 ms; 1000 iterations; 002 passes; 0.427272 modularity] louvainSeq {acc_capacity=7}
# [35413.695 ms; 1000 iterations; 002 passes; 0.570421 modularity] louvainSeq {acc_capacity=13}
# [34431.406 ms; 1000 iterations; 002 passes; 0.653526 modularity] louvainSeq {acc_capacity=31}
# [35660.219 ms; 1500 iterations; 003 passes; 0.720605 modularity] louvainSeq {acc_capacity=61}
# [35347.000 ms; 1500 iterations; 003 passes; 0.728701 modularity] louvainSeq {acc_capacity=127}
# [36584.254 ms; 2000 iterations; 004 passes; 0.750380 modularity] louvainSeq {acc_capacity=251}
# [36727.715 ms; 2000 iterations; 004 passes; 0.767695 modularity] louvainSeq {acc_capacity=509}
# [07881.160 ms; 0121 iterations; 004 passes; 0.784888 modularity] louvainSeq {acc_capacity=1021}
# [04656.739 ms; 0076 iterations; 005 passes; 0.798086 modularity] louvainSeq {acc_capacity=2039}
# [07468.710 ms; 0127 iterations; 006 passes; 0.809104 modularity] louvainSeq {acc_capacity=4093}
#
# Loading graph /home/subhajit/data/web-BerkStan.mtx ...
# order: 685230 size: 7600595 [directed] {}
# order: 685230 size: 13298940 [directed] {} (symmetricize)
# [-0.000316 modularity] noop
# [13553.333 ms; 0404 iterations; 009 passes; 0.935729 modularity] louvainSeq
# [120115.758 ms; 1000 iterations; 002 passes; 0.040547 modularity] louvainSeq {acc_capacity=2}
# [108734.406 ms; 1000 iterations; 002 passes; 0.159041 modularity] louvainSeq {acc_capacity=3}
# [89190.539 ms; 1000 iterations; 002 passes; 0.471730 modularity] louvainSeq {acc_capacity=7}
# ...
```

[![](https://i.imgur.com/hgM8ncd.png)][sheetp]
[![](https://i.imgur.com/OA0qHZT.png)][sheetp]
[![](https://i.imgur.com/V4gTrp9.png)][sheetp]
[![](https://i.imgur.com/OOUwbQc.png)][sheetp]

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
[gist]: https://gist.github.com/wolfram77/0263f2f19d9d5b814632dbf435803edc
[charts]: https://imgur.com/a/zwSu8Ey
[sheets]: https://docs.google.com/spreadsheets/d/1swkgG9evcMQerXFFDh76i6DGtI5I5rkHfxsFBqvhC1U/edit?usp=sharing
[sheetp]: https://docs.google.com/spreadsheets/d/e/2PACX-1vTCZvvRLy_h1u1ueTY8pcWoUT-8NohG2jntvge2U0iRsJ9God1cu7Wzk89BNRWc0cw--MVUJHof6yiZ/pubhtml
