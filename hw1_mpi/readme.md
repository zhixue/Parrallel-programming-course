<h2 style="text-align:center">Parallel computing and parallel algorithm 

Homework 1  
</h2>
## Problem 1
> MPI programming:  
Use MPI_SEND and MPI_RECV to implement the MPI_ALLGATHER function, and compare the performance of your implementation and the original MPI implementation.  

##### Usage
```shell
mpicc -O -o hw1_1 hw1_1.c
mpirun -np 4 hw1_1 10000000
```
##### Description
`my_allgather` function: 
* step1 - send own data from every processor to other processors
* step2 - receive data from other processors and add into `recv_data` 
* step3 - add the own data from `send_data` to `recv_data`

Using computing the average of 100,000,000 random numbers to compare the performance(1/2/4 processors)

processor number | myallgather time(s) | MPI_Allgather time(s)
---|---|---
1 | 0.000009 | 0.000016
2 | 0.000026 | 0.000047
4 | 0.000038 | 0.000057

----------------

## Problem 2
> MPI programming:  
Initialize a random 1024 *1024 matrix.  
2.1 Implement the matrix multiplication function.  
2.2 Using a 4 * 4 kernel to do the pooling operation for the matrix.  
2.3 Using a 4 * 4 kernel to do the convolution operation for the matrix.  

##### Usage
change the `#define N 5` to `#define N 1024` and 
```shell
mpicc -O -o hw1_2 hw1_2.c
mpirun -np 4 hw1_2 > 1_2out.txt
```

##### Description
`random_matrix` function:  
* create a N x N random matrix (elements are limit 0~10 to present, which can be edited in `hw1_2.c`)

`print_matrix` function:  
* print matrix   

##### 2.1 multiplication
`multiplication_matrix_matrix` function:  
* divide rows of left matrix with n blocks(e.g. 1024 rows, 4 processors - processor 1: 1~256rows; processor 2: 257~512rows; processor 3: 513~768rows; processor 4: 769~1024rows).
* If rows can not divide by processor number, the left rows will be computed by root processor 0, and add the results to `outmatrix`

##### 2.2 pooling
`max_pooling` function:  
* bcast the matrix and divide the rows tasks with n blocks(e.g. 1024 rows, 4 processors - processor 1: 1~256rows; processor 2: 257~512rows; processor 3: 513~768rows; processor 4: 769~1024rows), compute the max number of sub_matrix with given kernel, stripe
* If rows can not divide by processor number, the left rows will be computed by root processor 0, and add the results to `out_matrix`

`get_sub_matrix_max` function:
* get the max number in kernel size(4x4) of matrix 
* `start_i`,`start_j` is the left_up element postion 


##### 2.3 convolution
`convolution` function:
* bcast the matrix and divide the rows tasks with n blocks(e.g. 1024 rows, 4 processors - processor 1: 1~256rows; processor 2: 257~512rows; processor 3: 513~768rows; processor 4: 769~1024rows), compute sub_matrix * weight at same positon and sum all with given kernel, stripe  

`get_sub_matrix_mult_weight_sum` function:  
* compute sub_matrix * weight with kernel size(4x4) at same positon and sum all
* `start_i`,`start_j` is the left_up element postion 

Here is a test example of `N = 5`(avoid too many outputs), and run with 2 processors(change the N to 1024 in `hw1_2.c` and it can run)
```shell
Matrix 1 is:
7 9 3 8 0
2 4 8 3 9
0 5 2 2 7
3 7 9 0 2
3 9 9 7 0
Matrix 2 is:
3 9 8 6 5
7 6 2 7 0
3 9 9 9 1
7 2 3 6 5
5 8 1 4 7
Matrix 1 * Matrix 2 is:
149 160 125 180 78
124 192 114 166 96
90 108 41 93 61
95 166 121 156 38
148 176 144 204 59
Matrix 1 is:
1 3 8 4 8
0 4 6 0 3
2 6 9 4 1
3 7 8 8 3
8 1 5 3 5
Matrix 1 after pooling is:
9 9
9 9
Weight is:
0 1 0 -1
1 0 0 -1
-1 0 1 0
1 -1 -1 0
Matrix 1 after convolution is:
-6 26
-5 -8
```

------------------

## Problem 3
> MPI programming:  
We will provide one folder which contains 100 small files and one folder which contains 1 big file.  
Implement the wordcount algorithm respectively for the two situations and print the results to the screen. 

##### Usage
Please make sure that `./Big_file/*` and `./Small_file/*` could be searched in the same dir. 
```shell
# small file
mpicxx -O -o hw1_3_1 hw1_3_1.cpp
mpirun -np 4 hw1_3_1 > 1_3_1out.txt 
# big file
mpicxx -O -o hw1_3_2 hw1_3_2.cpp
mpirun -np 4 hw1_3_2 > 1_3_2out.txt
```
##### Description

##### 3.1 small file
The first processor manage and receive the summary data, the rest processors read files and send the result of word count of every file to the first one. Map size,elements of map (key&value) as well as their length are sent to the first processor.    
Special characters in the words are removed in this homework.   
Note that the small file number may not be divided by processors number, set `blockn` as the file number read by each processor which is `int(file number/processor number) + 1` in order to read all of the files, and `min()` to promise that the file id will not exceed(the last processor may read less files than others).   
map object are turned in to char* for communication. 

Some detailes of functions are as following(in file `hw_1_3_1.cpp`):   
`split` function:
* split string to vector<string> with splitchar ' '

`isword` function:
* whether a char is a character range from a to z or A to Z

`strip` function:
* remove the special character from a string

`downletter` function:
* make the characters of a word range from a to z

`add_string_to_map` function:
* this string is like the pattern of `key value`, update the key and value of wordcount map

`wordcount_file` function:
* open a file and read words of each line and update the wordcount map

`min` function:
* return a min number of two number 

test output:(head -5)
```shell
a: 5299
aa: 12
aacontrols: 1
aarnet: 12
ab: 16
...
```

##### 3.2 big file
The first processor manage and receive the summary data, the rest processors read the big file **from the given start line number to end line number** and send the result of word count of every part to the first one. Map size,elements of map (key&value) as well as their length are sent to the first processor.  
Special characters in the words are removed in this homework.   
Note that the **all file line count** may not be divided by processors number, set `blockn` as the **file line count** read by each processor which is `int(file line count/processor number) + 1` in order to read all of the lines in the file, and `min()` to promise that the file line number will not exceed(the last processor may read less lines than others). The **all file line count** can be got from `get_fileline`.  
map object are turned in to char* for communication.

Some detailes of functions are as following(in file `hw_1_3_2.cpp`):  
`split` function:
* same with 3.1

`isword` function:
* same with 3.1

`strip` function:
* same with 3.1

`downletter` function:
* same with 3.1

`add_string_to_map` function:
* same with 3.1

`get_fileline` function:
* return the sum line count of a file

`wordcount_file` function:
* open a file and read words of **given start line to given end line** and update the wordcount map

`min` function:
* same with 3.1 

test output:(head -5)
```shell
a: 688836
ac: 707336
accumsan: 1577042
ad: 48776
adipiscing: 56278
...
```
