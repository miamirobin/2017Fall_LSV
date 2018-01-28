please add 
    src/lsv src/sat/bsat2
in the makefile
------------------------------------
simple dofile:

read final_testcase/c17.blif
MajDecompose
quit

----------------------------------------
the output will be

0
NO SOLUTION

if PO is not decomposable
------------------------------------------
0
13 12 11

1
12 11 14

if PO can be decompose by three part with 12 11 14 supports
---------------------------------------------