
Notes
=======================

	1. This code was used for the empirical study of the SSTD'2017 paper 
	"Inherent-Cost Aware Collective Spatial Keyword Queries".
	2. This code is developed by Harry Kai-Ho Chan (khchanak@cse.ust.hk).
	3. This code is written in C/C++.
	4. This code runs under Unix/Linux.
	5. In case that you encounter any problems when using this code,
	please figure out the problem by yourself 
	(The code in fact is easy to read and you can modify it for your own purpose).

Usage
=======================

Step 1: Compile the source code
make

Step 2: Run the code
./CoSKQ

Step 3: Input Files (Format of those files can be found in Appendix A)
By default, we have file "config.txt" for configuration

Step 4: Collect the querying results and running statistics 
[you can ignore this step if you don't want to collect the information of
querying results and running statistics]

the querying results are stored in "result.txt"
which format is explained in Appendix D.

the running statistics are stored in "stat.txt"
which format is explained in Appendix E.


Appendix A. The format of config.txt
============================

<Cost indicator>
<Algorithm indicator> 
<# of dimensions>
<# of objects>
<Location file>
<# of keywords>
<Keyword file>
<IR-tree option>
<IR-tree file>
<# of query keywords>
<query set size>
<Percentile lower bound>
<Percentile upper bound>


Explanation of the content in config.txt
-----------------------

<Cost indicator>
	= 3: the MaxDotSize cost function (without inherent cost)
	= 13: the MaxDotSize cost function (with inherent cost)

<Algorithm indicator> 
	= 10: MaxDotSize-E
	= 11: MaxDotSize-A
	= 2: Long-A
	= 3: Cao-E
	= 4: Cao-A1
	= 52: Cao-A2
	= 9: Cao-A3

<# of dimensions>
	: the number of dimensions of spatial space.

<# of objects>
	: the number of spatial objects.

<Location file>
	: the file containing the locations of the spatial objects,
which format is explained in Appendix II.

<# of keywords>
	: the total number of all possible keywords contained by the objects.

<Keyword file>
	: the file containing the keywords of the spatial objects,
which format is explained in Appendix III.

<IR-tree option>
	= 0: the IR-tree has been built (which will be built and stored in <IR-tree file>)
	= 1: the IR-tree has been built (which is stored in <IR-tree file file>)

<IR-tree file>
	: the file for storing a new (or an existing) IR-tree.

<# of query keywords>
	: the number of keywords in the query (i.e., |q.\psi|).

<query set size>
	: the number of queries that will be performed for the same setting, 
	the average statistics based on which will be used.

<Percentile lower bound>
	: the percentile lower bound that is used for generating queries.

<Percentile upper bound>
	: the percentile upper bound that is used for generating queries.


(See file config.txt in the folder for example)

Appendix B. The format of <Location file>
============================

------------------------
<object ID1>, <object inherent cost>, <1st coordinate>, <2nd coordinate>, ..., <m^th coordinate>
<object ID2>, <object inherent cost>, <1st coordinate>, <2nd coordinate>, ..., <m^th coordinate>
<object ID3>, <object inherent cost>, <1st coordinate>, <2nd coordinate>, ..., <m^th coordinate>
...
<object IDn>, <object inherent cost>, <1st coordinate>, <2nd coordinate>, ..., <m^th coordinate>
------------------------

Note that
	n = # of objects
	m = # of dimensions
	In case you do not consider the object inherent cost, please input 1 in <object inherent cost>

(See file running-loc in the folder for example)

Appendix C. The format of <Keyword file>
=============================

------------------------
<object ID1>, <1st keyword>, <2nd keyword>, ...
<object ID2>, <1st keyword>, <2nd keyword>, ...
<object ID3>, <1st keyword>, <2nd keyword>, ...
...
<object IDn>, <1st keyword>, <2nd keyword>, ...
------------------------

(See file running-doc in the folder for example)

Appendix D. The format of <result.txt>
=============================

------------------------
Query #1:
Keywords: <Keywords of the query>
======================
<# of objects in the solution>

<object ID1>: <1st relevant of this object> <2nd relevant of this object> ...
<object ID2>: <1st relevant of this object> <2nd relevant of this object> ...
...
<object IDk>: <1st relevant of this object> <2nd relevant of this object> ...


Query #2:
same format as for Query #1.

...

Query #t:
same format as for Query #1
------------------------

Note that 
	k = the number of objects in the solution
	t = <query set size>

(See file result.txt in the folder for example)

Appendix E. The format of <stat.txt>
=============================
<Average cost function value>

<the time of building the IR-tree>
<the average time of performing a query>

<Memory usage>
<IR-tree memory usage>
<n_1>

<ratio_min>
<ratio_max>
<ratio_average>
<ratio_deviation>


(See file stat.txt in the folder for example)



