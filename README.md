Comparing approaches for *community detection* using **Louvain algorithm**.

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

<br>


### Adjusting Tolerance

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

In this experiment ([adjust-tolerance]), we change the initial value of
`tolerance` (for local-moving phase) from `1e-00` to `1e-6` in steps of `10`.
For each initial value of `tolerance`, we use a tolerance-norm of `L1`, `L2`, or
`L∞`. We compare the results, both in terms of quality (modularity) of
communities obtained, and performance. We choose the remaining Louvain
*parameters* as `resolution = 1.0`, `toleranceDeclineFactor = 10` (the rate at
which we reduce tolerance after every pass), and `passTolerance = 0.0`. In
addition we limit the maximum number of iterations in a single local-moving
phase with `maxIterations = 500`, and limit the maximum number of passes with
`maxPasses = 500`. We run the Louvain algorithm until convergence (or until the
maximum limits are exceeded), and measure the **time taken** for the
*computation* (performed 5 times for averaging), the **modularity score**, the
**total number of iterations** (in the *local-moving* *phase*), and the number
of **passes**. This is repeated for *seventeen* different graphs.

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

[adjust-tolerance]: https://github.com/puzzlef/louvain-communities/tree/adjust-tolerance

<br>


### Adjusting Tolerance iteratively

In this experiment ([adjust-tolerance-iteratively]), we adjust `tolerance` in
two different ways. First, we change the initial value of `tolerance` from
`1e-00` to `1e-12` in steps of `10`. For each initial value of `tolerance`, we
adjust the rate at which we decline tolerance between each pass
(`toleranceDeclineFactor`) from `10` to `10000`. We compare the results, both in
terms of quality (modularity) of communities obtained, and performance. We
choose the remaining Louvain *parameters* as `resolution = 1.0` and
`passTolerance = 0.0`. In addition we limit the maximum number of iterations in
a single local-moving phase with `maxIterations = 500`, and limit the maximum
number of passes with `maxPasses = 500`. We run the Louvain algorithm until
convergence (or until the maximum limits are exceeded), and measure the **time**
**taken** for the *computation* (performed 5 times for averaging), the
**modularity score**, the **total number of iterations** (in the *local-moving*
*phase*), and the number of **passes**. This is repeated for *seventeen*
different graphs.

From the results, we observe that an initial **tolerance** of `1e-2` yields
communities with the best possible modularity while requiring the least
computation time. In addition, increasing the `toleranceDeclineFactor`
increases the computation time (as expected), but does not seem to impact
resulting modularity. Therefore choosing a **toleranceDeclineFactor** of `10`
would be a good idea.

[adjust-tolerance-iteratively]: https://github.com/puzzlef/louvain-communities/tree/adjust-tolerance-iteratively

<br>


### Adjusting Accumulator hashtable capacity

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

In this experiment ([adjust-accumulator-capacity]), we adjust the capacity of
accumulator hashtable from `2` to `4093` in multiples of 2. This capacity is
always set to the highest prime number below a power of 2.  We choose the
Louvain *parameters* as `resolution = 1.0`, `tolerance = 0` and `passTolerance = 0.5`.
In addition we limit the maximum number of iterations in a single
local-moving phase with `maxIterations = 500`, and limit the maximum number of
passes with `maxPasses = 500`. We run the Louvain algorithm until convergence
(or until the maximum limits are exceeded), and measure the **time taken**
for the *computation* (performed 5 times for averaging), the **modularity**
**score**, the **total number of iterations** (in the *local-moving phase*), and
the number of **passes**. This is repeated for *seventeen* different graphs.

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

[adjust-accumulator-capacity]: https://github.com/puzzlef/louvain-communities/tree/adjust-accumulator-capacity

<br>


### Adjusting Accumulator hashtable capacity without collisions

Originally, my idea was to look if we can simply **do away** with **large**
**hash tables** and **collision resolution** altogether, by simply *considering*
*identical hash keys* as *identical labels*. However, either due to a mistake of
my own or due to the limit to the total number of communities enforced by the
lack of collision resolution, the quality of communitied (based on modularity)
were really bad. Therefore, with this experiment, we put in place **proper**
**collision resolution** *(algorithmically)*, but still *restrict the capacity of the*
*accumulator hashtable*. As you will read later, this gives us good results.

In this experiment ([adjust-accumulator-capacity-with-collisions]), we adjust
the capacity of accumulator hashtable from `2` to `4093` in multiples of 2. This
capacity is always set to the highest prime number below a power of 2.  We
choose the Louvain *parameters* as `resolution = 1.0`, `tolerance = 0` and
`passTolerance = 0.5`.  In addition we limit the maximum number of iterations in
a single local-moving phase with `maxIterations = 500`, and limit the maximum
number of passes with `maxPasses = 500`. We run the Louvain algorithm until
convergence (or until the maximum limits are exceeded), and measure the **time**
**taken** for the *computation* (performed 5 times for averaging), the
**modularity score**, the **total number of iterations** (in the *local-moving*
*phase*), and the number of **passes**. This is repeated for *seventeen*
different graphs.

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

[adjust-accumulator-capacity-with-collisions]: https://github.com/puzzlef/louvain-communities/tree/adjust-accumulator-capacity-with-collisions

<br>


### Adjusting Hash function for Accumulator hashtable

In this experiment ([adjust-accumulator-hash-function]), we try five different
hash functions, and adjust the capacity of accumulator hashtable from `509` to
`4093` in multiples of 2. This capacity is always set to the highest prime
number below a power of 2.  We choose the Louvain *parameters* as `resolution = 1.0`,
`tolerance = 0` and `passTolerance = 0.5`. In addition we limit the
maximum number of iterations in a single local-moving phase with `maxIterations = 500`,
and limit the maximum number of passes with `maxPasses = 500`. We run
the Louvain algorithm until convergence (or until the maximum limits are
exceeded), and measure the **time** **taken** for the *computation* (performed 5
times for averaging), the **modularity score**, the **total number of**
**iterations** (in the *local-moving* *phase*), and the number of **passes**. This
is repeated for *seventeen* different graphs.

We observe that `magic` **hash function** with an **accumulator hashtable**
**capacity** of `4093` appears to perform the **best** among the limited capacity
hashtable approaches. However it is still worse than the default approach of
Louvain algorithm. Therefore it seems using a limited capacity accumulator
hashtable for the Louvain algorithm is not useful, i.e., **we must stick to**
**full capacity accumulator hashtable**.

[adjust-accumulator-hash-function]: https://github.com/puzzlef/louvain-communities/tree/adjust-accumulator-hash-function

<br>


### Simple first move optimization

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

In this experiment ([optimization-simple-first-move]), we compare the standard
and simplified first move approaches for the Louvain algorithm, both in terms of
quality (modularity) of communities obtained, and performance. We choose the
Louvain *parameters* as `resolution = 1.0`, `tolerance = 1e-2` (for local-moving
phase) with *tolerance* decreasing after every pass by a factor of
`toleranceDeclineFactor = 10`, and a `passTolerance = 0.0` (when passes stop).
In addition we limit the maximum number of iterations in a single local-moving
phase with `maxIterations = 500`, and limit the maximum number of passes with
`maxPasses = 500`. We run the Louvain algorithm until convergence (or until the
maximum limits are exceeded), and measure the **time taken** for the
*computation* (performed 5 times for averaging), the **modularity score**, the
**total number of iterations** (in the *local-moving phase*), and the number of
**passes**. We also track the *time*, *iterations*, and *modularity* obtained
from the first local-moving phase on the CPU and the GPU. This is repeated for
*seventeen* different graphs.

From the results, we observe that **using a simple first move** requires in
general somewhat **more time (and iterations) to converge** than the standard
approach, but may also return communities of **slightly higher modularity**. We
therefore conclude that using a **simple first move is not useful on the CPU**,
but it may be useful on the GPU for the reasons mentioned above.

[optimization-simple-first-move]: https://github.com/puzzlef/louvain-communities/tree/optimization-simple-first-move

<br>


### Comparision with Unordered/Synchronous approach

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

In this experiment ([compare-unordered]), we compare the ordered and unordered
vertex processing approaches for the Louvain algorithm, both in terms of quality
(modularity) of communities obtained, and performance. We choose the Louvain
*parameters* as `resolution = 1.0`, `tolerance = 0.0` (for local-moving phase),
`passTolerance = 0.0` (when passes stop). In addition we limit the maximum
number of iterations in a single local-moving phase with `maxIterations = 500`,
and limit the maximum number of passes with `maxPasses = 500`. We run the
Louvain algorithm until convergence (or until the maximum limits are exceeded),
and measure the **time taken** for the *computation* (performed 5 times for
averaging), the **modularity score**, the **total number of iterations** (in the
*local-moving phase*), and the number of **passes**. This is repeated for
*seventeen* different graphs.

From the results, we observe that **both** the *ordered* and the *unordered*
vertex processing approaches of the Louvain algorithm are able to provide
communities **of equivalent quality** in terms of **modularity**, with the
*ordered approach* providing *slightly higher quality communities* for certain
graphs. However, the **unordered approach** is **quite a bit slower** than the
*ordered approach* in terms of the **total time taken**, as well as the **total**
**number of iterations** of the *local-moving phase* (which is the *most*
*expensive part* of the algorithm). We therefore conclude that **partially**
**ordered approaches** for vertex processing are **likely to provide good**
**performance improvements** over fully unordered approaches **for parallel**
**implementations** of the Louvain algorithm. *Vertex ordering* via *graph*
*coloring* has been explored by Halappanavar et al.

[compare-unordered]: https://github.com/puzzlef/louvain-communities/tree/compare-unordered

<br>


### Other experiments

- [adjust-iteration-processing](https://github.com/puzzlef/louvain-communities/tree/adjust-iteration-processing)

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
[![ORG](https://img.shields.io/badge/org-puzzlef-green?logo=Org)](https://puzzlef.github.io)
[![DOI](https://zenodo.org/badge/516219283.svg)](https://zenodo.org/badge/latestdoi/516219283)


[Prof. Dip Sankar Banerjee]: https://sites.google.com/site/dipsankarban/
[Prof. Kishore Kothapalli]: https://faculty.iiit.ac.in/~kkishore/
[SuiteSparse Matrix Collection]: https://sparse.tamu.edu
[Louvain]: https://en.wikipedia.org/wiki/Louvain_method
