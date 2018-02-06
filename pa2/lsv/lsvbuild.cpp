



#include <iostream>
#include <cassert>
#include <vector>
#include "lsv.h"
#include <algorithm>

int flag=1;
vector<Var> vecVar[2] ;
vector<Abc_Ntk_t*> vecNtk;
Var* actVar;
SatSolver *minisatPtr ;


vector<vector< int > > vecPo;
vector<vector<int > >vecNode;
vector< int > vecTemp;
vector<int > vecTem2;
vector<int > vecPi;
int needPo;
Abc_Obj_t * pFanout;
Abc_Obj_t * pFanin;
int ri;
vector<vector<int > >vecAll;
vector< int > vecTem3;
int pi_node;

void selectCandidateforpi(Abc_Ntk_t * pNtk){
     int i;
     pi_node=5;
     Abc_Obj_t * pObj;
     vecPo.clear();
     vecNode.clear();
     vecAll.clear();
     Abc_NtkForEachPi( pNtk, pObj, i){  
          flag++;	
          vecTemp.clear();
          needPo=flag;
       
          
          selectPorec(pObj);           
          vecPo.push_back(vecTemp);
          vecPi.clear();
          vecTem3.clear();
                  
          pObj->Temp=needPo;
          flag++;
          selectGate(pNtk);
          vecAll.push_back(vecTem3);
    }

} 

//select the PO containing target node
void selectCandidate(Abc_Ntk_t* pNtk){
     int i;
     pi_node=10;
     Abc_Obj_t * pObj;
     vecPo.clear();
     vecNode.clear();
     vecAll.clear();
    
    

     Abc_NtkForEachNode( pNtk, pObj, i){  
          flag++;	
          vecTemp.clear();
          needPo=flag;
       
          
          selectPorec(pObj);           
          vecPo.push_back(vecTemp);
          vecPi.clear();
          vecTem2.clear();
          vecTem3.clear();
          flag++;
          selectPirec(pObj);
          pObj->Temp=needPo;
          flag++;
          selectGate(pNtk);
          sort(vecTem2.begin(),vecTem2.end());
          vecNode.push_back(vecTem2);
          vecAll.push_back(vecTem3);
    }
       
        
    	
    
}
void selectPorec(Abc_Obj_t * pObj){

   if (pObj->Temp==flag){return;}
   
   pObj->Temp=flag;

  
 
  for (int i=0;i<Abc_ObjFanoutNum(pObj);i++){
       Abc_Obj_t * pFanout=Abc_ObjFanout(pObj, i);
   
      
       if(pFanout->Temp==flag){continue;}
         
       
       if (Abc_ObjIsPo(pFanout) ){
             pFanout->Temp=flag;
             vecTemp.push_back(Abc_ObjId(pFanout));
             
       }
       else {
             
             selectPorec(pFanout);
             pFanout->Temp=flag;
             
       }
     
   }
}




void selectPirec(Abc_Obj_t * pObj){
  
  if ( pObj->Temp==flag){	return;}
  pObj->Temp=flag;
  for (int i=0;i<Abc_ObjFaninNum(pObj);i++){
       Abc_Obj_t * pFanin=Abc_ObjFanin(pObj, i);
       if (pFanin->Temp==flag){ continue;}
       if (Abc_ObjIsPi(pFanin) ){
             pFanin->Temp=flag;
             vecPi.push_back(Abc_ObjId(pFanin));
             
       }
       else {
             
             selectPirec(pFanin);
             pFanin->Temp=flag;
             
       }
 }
}

void selectGate(Abc_Ntk_t * pNtk){
      
    	for (int i=0;i<vecPi.size();i++){
        	
        	Abc_Obj_t * needPi =  Abc_NtkObj( pNtk,vecPi[i]);         	
        	selectGaterec(needPi);            
               
                
    	}
      

        flag++;
        for (int i=0;i<Abc_NtkPiNum(  pNtk );i++){
             Abc_Obj_t * Pi =  Abc_NtkObj( pNtk,i+1);         	
        	selectGaterecforall(Pi);    

        }
    
}



void selectGaterec(Abc_Obj_t *pObj ){
   if (pObj->Temp ==flag or pObj->Temp ==needPo){ return;}
   if (Abc_ObjIsPo(pObj)){return;}
   pObj->Temp=flag;       
   vecTem2.push_back(Abc_ObjId(pObj)); 
   for (int i=0;i<Abc_ObjFanoutNum(pObj);i++){
       Abc_Obj_t * pFanout=Abc_ObjFanout(pObj, i);  
       
       selectGaterec(pFanout);
    
   }
}

void selectGaterecforall(Abc_Obj_t *pObj ){
   if (pObj->Temp ==flag or pObj->Temp ==needPo){ return;}
   if (Abc_ObjIsPo(pObj)){return;}
   pObj->Temp=flag;       
   vecTem3.push_back(Abc_ObjId(pObj)); 
   for (int i=0;i<Abc_ObjFanoutNum(pObj);i++){
       Abc_Obj_t * pFanout=Abc_ObjFanout(pObj, i);  
       
       selectGaterecforall(pFanout);
    
   }
}

