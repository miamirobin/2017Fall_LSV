seatAss.o: seatAss.cpp sat1.h Solver.h Vec.h IntTypes.h XAlloc.h Heap.h \
 IntMap.h Alg.h Options.h ParseUtils.h SolverTypes.h Map.h Alloc.h
Options.o: Options.cpp Sort.h Vec.h IntTypes.h XAlloc.h Options.h \
 ParseUtils.h
sat.o: sat.cpp ../minisat/sat.h ../minisat/Solver.h ../minisat/Vec.h \
 ../minisat/IntTypes.h ../minisat/XAlloc.h ../minisat/Heap.h \
 ../minisat/IntMap.h ../minisat/Alg.h ../minisat/Options.h \
 ../minisat/ParseUtils.h ../minisat/SolverTypes.h ../minisat/Map.h \
 ../minisat/Alloc.h
System.o: System.cpp System.h IntTypes.h
Solver.o: Solver.cpp Alg.h Vec.h IntTypes.h XAlloc.h Sort.h System.h \
 Solver.h Heap.h IntMap.h Options.h ParseUtils.h SolverTypes.h Map.h \
 Alloc.h
