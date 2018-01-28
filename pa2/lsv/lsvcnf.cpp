


#include <stdio.h>
#include <cassert>
#include <climits>
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <algorithm>
#include <time.h>

#include "lsv.h"
#include <map>
/**************************************
   
**************************************/


// initialize SAT solver
void initSolver(SatSolver* solver, Abc_Ntk_t * pNtk) {
  solver->init();
  int i;  
  Abc_Obj_t * pObj;
  Abc_NtkForEachObj( pNtk, pObj, i){ 
    	vecVar[0].push_back(solver->newVar());
        vecVar[1].push_back(solver->newVar());
  }
}

// add CNF of the circuit to SAT solver
void addCircuitCNF(SatSolver* solver, Abc_Ntk_t * pNtk, int n, int id) {
    Abc_Obj_t * pObj;
    int i,ii;

    /*add const gate cnf*/
    solver->addUnitCNF(vecVar[n][0], true);

    Abc_NtkForEachPo( pNtk, pObj, i){ 
    	 Abc_Obj_t * in0 = Abc_ObjFanin0(pObj );
         int id0 = Abc_ObjId(in0);
         ii=Abc_NtkPiNum(  pNtk )+1+i;
         solver->addEqCNF(vecVar[n][ii], vecVar[n][id0], bool(pObj->fCompl0));

         //printf("\nPO:%2d %2d %2d",ii, id0, (pObj->fCompl0));
    }
    Abc_NtkForEachNode( pNtk, pObj, i){ 
         if (n==0 and i==id){continue;}
         Abc_Obj_t * in0 = Abc_ObjFanin0(pObj );
         Abc_Obj_t * in1 = Abc_ObjFanin1(pObj );
         int id0 = Abc_ObjId(in0);
         int id1 = Abc_ObjId(in1);
    	solver->addAndCNF(vecVar[n][i],vecVar[n][id0], bool(pObj->fCompl0),vecVar[n][id1],bool(pObj->fCompl1));


         //printf("\nNode:%2d %2d %2d %2d %2d",i,id0, (pObj->fCompl0),id1,(pObj->fCompl1));
    }

}


void addCommonVar(int n1, int i1, int n2, int i2){     
   
      minisatPtr->addEqCNF(vecVar[n1][i1], vecVar[n2][i2], false);
}

void buildActivateVar(Abc_Ntk_t * pNtk,int n){
  int number=vecNode[n].size();
  actVar = new Var[number*2];
  Var v1,v2;
 int realid=n+ Abc_NtkPiNum(  pNtk )+ Abc_NtkPoNum(  pNtk )+1;
 //printf ("nn:%2d %2d",n,realid);
  for (int i=0;i<number; ++i){
    
		actVar[i*2] = minisatPtr->newVar();  
                actVar[i*2+1] = minisatPtr->newVar();
		v1 = vecVar[0][vecNode[n][i]];
		v2 = vecVar[0][realid];    
		minisatPtr->addClause(mkLit(actVar[i*2], true), mkLit(v1, false), mkLit(v2, true));
		minisatPtr->addClause(mkLit(actVar[i*2], true), mkLit(v1, true), mkLit(v2, false));   
      
               
		minisatPtr->addClause(mkLit(actVar[i*2+1], true), mkLit(v1, true), mkLit(v2, true));
		minisatPtr->addClause(mkLit(actVar[i*2+1], true), mkLit(v1, false), mkLit(v2, false));   
       
       
  }
   

}







void buildMiter(Abc_Ntk_t * pNtk, int n){
  
   Abc_Obj_t * pObj;
    int id;

  int o=vecPo[n].size();                                                                        
  Var* _on_OR_i_Var = new Var[ o];                        
  Var  _on_OR_o_var = minisatPtr->newVar();
  vec<Lit> _litVec;
  _litVec.capacity(o+1);                                 
  _litVec.push(mkLit(_on_OR_o_var, true)); // (~Output)                              
                                                                   
  for (int j=0; j < o; ++j){   
     int id= vecPo[n][j];         
    //printf("po:%2d",id);
    _on_OR_i_Var[j] = minisatPtr->newVar();                             

    // add output difference gate
    minisatPtr->addXorCNF(_on_OR_i_Var[j],vecVar[0][id],false,
    vecVar[1][id],false);                                                    
    // add (~Input i + Output)
    minisatPtr->addClause(mkLit(_on_OR_i_Var[j], true), mkLit(_on_OR_o_var, false));           
            
    // (~Output += Input 1~n)
    _litVec.push(mkLit(_on_OR_i_Var[j], false));
  }  
  minisatPtr->addClause(_litVec); // _litVec = (~Output + Input 0 + ... + Input N)                   
                                                                                                 
  // bind primary input
   Abc_NtkForEachPi( pNtk, pObj, id){
      if (pi_node==5 and id==n){continue;}
      minisatPtr->addEqCNF(vecVar[0][id+1], vecVar[1][id+1], false);
  }                                                                                            
 
   minisatPtr->addClause(mkLit(_on_OR_o_var, false)); // output of OR_miter must be 1
    
  
  
	minisatPtr->simplify();
      if (!minisatPtr->solve() and pi_node==5){
               //printf("55");
               sort(vecAll[n].begin(),vecAll[n].end());
                     pObj= Abc_NtkObj( pNtk, n+1);
                     printf("\n%2s: ",Abc_ObjName(pObj));
                     for (int j=0;j<vecAll[n].size();j++){
                            pObj= Abc_NtkObj( pNtk,vecAll[n][j]  ); 
                            printf("%-2s ",Abc_ObjName(pObj));
                            printf("-%-2s ",Abc_ObjName(pObj));
                     }
                     
                    
      }
		
}

void check(Abc_Ntk_t * pNtk,int n){
  Abc_Obj_t * pObj;
  int j,count=0;
  vec<Lit> _assume;
  int number =vecNode[n].size();
  int realid=n+ Abc_NtkPiNum(  pNtk )+ Abc_NtkPoNum(  pNtk )+1;
 
	for (int i=0; i<number; ++i){
		_assume.push(mkLit(actVar[i*2], true)); // make all variable free at first
                _assume.push(mkLit(actVar[i*2+1], true)); 
         }
          
         if ( !minisatPtr->solve(_assume)){
                     //printf("66");
                     sort(vecAll[n].begin(),vecAll[n].end());
                     pObj= Abc_NtkObj( pNtk, realid );
                     printf("\n%2s: ",Abc_ObjName(pObj));
                     for (int j=0;j<vecAll[n].size();j++){
                            pObj= Abc_NtkObj( pNtk,vecAll[n][j]  ); 
                            printf("%-2s ",Abc_ObjName(pObj));
                            printf("-%-2s ",Abc_ObjName(pObj));
                     }
                     
                     return;
         }
      

        for (int i =0;i<number;++i){
		_assume[i*2] = mkLit(actVar[i*2], false); // make them be common variable
                if (vecNode[n][i]==0){continue;}
		if ( !minisatPtr->solve(_assume)){
                     if (count==0){
                          pObj= Abc_NtkObj( pNtk, realid );
                          printf("\n%2s: ",Abc_ObjName(pObj));
                          count++;
                     } 
                     pObj= Abc_NtkObj( pNtk, vecNode[n][i] ); 
                     
                     printf("%-2s ",Abc_ObjName(pObj));
                }
                _assume[i*2] = mkLit(actVar[i*2], true);

                _assume[i*2+1] = mkLit(actVar[i*2+1], false);
                if ( !minisatPtr->solve(_assume)){
                      if (count==0){
                          pObj= Abc_NtkObj( pNtk, realid );
                          printf("\n%2s: ",Abc_ObjName(pObj));
                          count++;
                     } 
                      pObj= Abc_NtkObj( pNtk, vecNode[n][i] ); 
                     
                     printf("-%-2s ",Abc_ObjName(pObj));
                }
                 _assume[i*2+1] = mkLit(actVar[i*2+1], true);
         }           
		
  
  delete actVar;
  vecVar[0].clear();
  vecVar[1].clear(); 

}

