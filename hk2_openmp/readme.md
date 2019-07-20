<h2 style="text-align:center">Parallel computing and parallel algorithm 

Homework 2  

</h2>

## Problem 1
> OpenMP programming:  
Using OpenMP to implement the Monte Carlo algorithm.

#### Description
Here I use Monte Carlo algorithm to estimate `$\pi$`.  
20000000 points are randomly generated(number of points can be changed) in an area of 1 x 1({(x,y)|0<=x<=1, 0<=y<=1}).  
I use `#pragma omp parallel for num_threads(thread_count)` block, to caculate `$ distance_{i}^2 = x_i * x_i + y_i * y_i $` from every point i to (0,0), if it is less than 1, the count of points in circle add 1.   
Finally, we can use reduction operation to get the sum of count of points in circle from threads and estimate that: 
```math
\widehat{\pi} \approx 4* \frac{count~ of~ points~ in~ circle}{number~ of~ all~ points}
```
Details are in `hw2_1.cpp`.

#### Usage
```shell 
g++ -g -Wall -fopenmp -o hw2_1 hw2_1.cpp
./hw2_1 <thread_number>
```
#### Sample output
```shell
$time ./hw2_1 4
Use 20000000 points to compute PI
the estimate value of pi is 3.141005

real	0m7.570s
user	0m6.702s
sys	0m20.395s
```

-----------

## Problem 2
> OpenMP programming:  
Using OpenMP to implement a quick sorting algorithm with large data volume which contains 1000000 number.

#### Description

* `void swap(int &a, int &b)` function:  
change the value of a and b

* `quicksort` function:  
quick sort, the differnce bwteen parallel and serial one is that parallel one uses and only uses `#omp parallel sections` and `#pragma omp section` in recurrent step(do not include other steps such as swap operations)

Details are in `hw2_2.cpp`.

#### Usage
```shell
g++ -g -Wall -fopenmp -o hw2_1 hw2_1.cpp
./hw2_1 <thread_number>
```

#### Sample output
```shell
# only 10 numbers
$./hw2_2 2
Raw numbers:
4289383 46930886 81692777 14636915 57747793 24238335 19885386 49760492 96516649 89641421
Sorted numbers:
4289383 14636915 19885386 24238335 46930886 49760492 57747793 81692777 89641421 96516649

# 1000000 numbers
$ time ./hw2_2 4 > hw2_2.4.output

real	0m1.174s
user	0m0.993s
sys	0m0.266s
```

---------------------


## Problem 3
> OpenMP programming:  
Initialize a graph has 1, 024, 000 nodes and the edge count of different nodes ranges from 1 to 10. Implement the PageRank algorithm and run for 100 iterations.

#### Description
Simplified Pagerank:   
Suppose there are N nodes, let `$R$` be the vector of rank value which is equal that
```math
R = (r_1,r_2,...,r_{N-1},r_N)
```
where `$r_i$` is the rank value of node `$i$` (i=1,...,N) , and satisfy that
```math
\sum_{i=1}^N r_i = 1 
```
Set the transition matrix `$A$`,for example if there is 4 nodes,edges are(0,1),(0,3),(1,2),(2,0),(2,1),(2,2),(2,3),(3,1), the transition matrix `$A$` is 
```math
A_{4*4} = \begin{bmatrix}
 0& 0.5 & 0 & 0.5\\ 
 0& 0 & 1 & 0\\ 
 0.25& 0.25 & 0.25 & 0.25\\ 
0 & 1 & 0 & 0
\end{bmatrix}
```
At the iterator `$k$` (k=1,...,MAX_ITERATOR)
```math
R_1 = (\frac{1}{N},\frac{1}{N},...,\frac{1}{N},\frac{1}{N})

R_{k(raw)} = R_{{k-1}(normlized)}A
```
However, the sum of `$r_{k(raw)}$` may not be equal to 1, so it is necessary to normlize it, in order to satisfy the sum is equal to 1.
```math
R_{k(normlized)} = \frac{1}{\sum_{i=1}^N r_{i(raw)}} R_{k(raw)}
```
The max and min rank value is also restricted from `$1/node~number^2$` to `$1-1/node~number^2$` to avoid that the rank value is 1 or 0. 

**In my code,two-dimensional transition matrix `$A$` is changed to one dimensional vector which only records the existed edges.**  


* `int rand_edge_num(int *indegree_per_node, int nodes_num, int max_edge_num_per_node)` function  
random generate edge srart **to** every node and return the sum of edge count 

* `void rand_graph(int *edge_start, int *edge_end, int *indegree_per_node, int *outdegree_per_node, int node_num)` function  
random generate edge from start to end (no repeat, no link to self) 

* `void init_transition(double *transiton, int *outdegree_per_node, int *start_edge, int edge_num)` function  
init transition vector according to the indegree of nodes

* `void pagerank(long double *nextrank_value, long double *currentrank_value, double *transition,int *start_edge,int *end_edge, int edge_num, int iterator, int thread_count)` function  
apply the pagerank to rank value vector, use `#pragma omp parallel for num_threads(thread_count)` to parallel update the next rank value.

Details are in `hw2_3.cpp`.

#### Usage
```shell
g++ -g -Wall -fopenmp -o hw2_1 hw2_1.cpp
./hw2_3 <thread_number>
```

#### Sample out
```shell
# 12 nodes for test
$./hw2_3 4
Total 12 nodes!
Total 64 edges!
Init Rank value: 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333 0.0833333333
Edges: (7,0) (2,1) (9,1) (4,1) (11,1) (6,1) (3,1) (8,1) (5,1) (10,2) (5,2) (0,2) (7,2) (3,2) (9,3) (4,3) (11,4) (6,4) (1,4) (8,4) (3,4) (10,4) (5,4) (2,4) (7,4) (2,5) (9,5) (4,5) (11,5) (6,5) (1,5) (8,6) (3,6) (10,6) (5,7) (1,7) (8,7) (2,7) (9,7) (4,7) (11,7) (6,7) (3,7) (10,7) (3,8) (10,8) (5,8) (1,8) (7,8) (2,8) (9,8) (4,9) (11,9) (6,9) (1,9) (8,10) (3,11) (10,11) (5,11) (1,11) (7,11) (2,11) (9,11) (4,11)
After 100 iteration Rank value: 0.0290414605 0.1215903000 0.0831666281 0.0354417550 0.1388070080 0.1000118802 0.0285267756 0.1452073025 0.1005585759 0.0738435219 0.0201117152 0.1236930772

# 1024000 nodes
$time ./hw2_3 4 > hw2_3.4.output

real	0m26.913s
user	0m58.463s
sys	0m0.228s
$tail -1 hw2_3.4.output | less
After 100 iteration Rank value: 0.0000028828 0.0000018397 0.0000001434 0.0000024363 0.0000011088 0.0000030961 0.0000005986 0.0000033471 0.0000030008 0.0000053463 0.0000044870 0.0000025759 0.0000001420 0.0000002614 0.0000022331 0.0000030933 0.0000029153 0.0000025832 0.0000031675 0.0000048227 0.0000005962 0.0000053161 0.0000000113 0.0000025167 0.0000017865 0.0000002544 0.0000063997 0.0000038358 0.0000011591 0.0000069608 0.0000032108 0.0000053603 0.0000087889 0.0000017192 0.0000051124 0.0000020912 0.0000038109 0.0000066248 0.0000000984 0.0000036503 0.0000038069 0.0000040753 0.0000042352 0.0000022153 0.0000038879 0.0000006608 0.0000074357 0.0000000482 0.0000047684 0.0000009497 0.0000027099 0.0000019646 0.0000068329 0.0000055396 0.0000000121 0.0000053472 0.0000041335 0.0000027233 0.0000038700 0.0000037223 0.0000027467 0.0000030066 0.0000018591 0.0000026054 0.0000035020 0.0000016682 0.0000024971 0.0000040564 0.0000061832 0.0000038293 0.0000027074 0.0000059666 0.0000045243 0.0000011319 0.0000014152 0.0000034995 0.0000027378 0.0000008267 ....
```
