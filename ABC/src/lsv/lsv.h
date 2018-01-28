#ifndef LSV_H 
#define LSV_H

#include <iostream>
#include <vector>
#include "minisat/sat.h"
#include "base/abc/abc.h"

using namespace std;
extern vector<Var> vecVar[2] ;
extern vector<Abc_Ntk_t*> vecNtk;
extern Var* actVar;
extern SatSolver *minisatPtr;
extern int flag;
extern vector<vector< int > > vecPo;
extern vector<vector<int > >vecNode;
extern  vector< int > vecTemp;
extern  vector<int > vecTem2;
extern  vector<int > vecPi;
extern int needPo;
extern Abc_Obj_t * pFanout;
extern Abc_Obj_t * pFanin;
extern int ri;
extern int pi_node;
extern vector<vector<int > >vecAll;
extern vector< int > vecTemp3;
extern void selectCandidate(Abc_Ntk_t* pNtk);
extern void selectCandidateforpi(Abc_Ntk_t* pNtk);
extern void selectPorec(Abc_Obj_t * pObj);
extern void selectPirec(Abc_Obj_t * pObj);
extern void selectGate(Abc_Ntk_t * pNtk);
extern void selectGaterec(Abc_Obj_t *pObj );
extern void selectGaterecforall(Abc_Obj_t *pObj );
extern void initSolver(SatSolver* solver, Abc_Ntk_t * pNtk) ;
extern void addCircuitCNF(SatSolver* solver, Abc_Ntk_t * pNtk, int n, int id);
extern void addCommonVar(int n1, int i1, int n2, int i2);
extern void buildActivateVar(Abc_Ntk_t * pNtk,int n);
extern void buildMiter(Abc_Ntk_t * pNtk, int n);
extern void check(Abc_Ntk_t * pNtk,int n);

#endif
