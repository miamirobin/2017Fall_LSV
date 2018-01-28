/**CFile****************************************************************

  FileName    [lsvMajFind.cpp]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [lsv: Logic Synthesis and Verification PA.]

  Synopsis    [procedure for finding MAJ gates.]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [17, Sep., 2017.]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "base/main/mainInt.h"
#include <vector>
using namespace std;
ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////
 
/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Lsv_NtkMajFind( Abc_Ntk_t * pNtk )
{
   // TODO
  int count=0;
  Abc_Obj_t * pObj;
  int i;           
  printf("hidden majority (MAJ) gates:\n");
  Abc_NtkForEachObj( pNtk, pObj, i){
       if (Abc_ObjFaninNum( pObj)==0){continue;}
      Abc_Obj_t * p8;
      unsigned com;
      if (Abc_ObjFaninNum( pObj)==1 ){ 
            continue;
            p8= Abc_ObjFanin0(pObj );  
            com =pObj->fCompl0; 
       }
       else if (Abc_ObjFaninNum( pObj)==2 ){
            p8= (pObj );
            com= 0;   
       }    
        else { continue;}     
            unsigned   p8inv=p8->fCompl0+p8->fCompl1;
            if (Abc_ObjFaninNum( p8)!=2 or p8inv!=2){ continue;}
               Abc_Obj_t * p6= Abc_ObjFanin0(p8 );
               Abc_Obj_t * p7= Abc_ObjFanin1(p8 );  
                             
               if (Abc_ObjFaninNum( p6)!=2 or Abc_ObjFaninNum( p7)!=2 ){continue;}
               vector< Abc_Obj_t * > ObjVec;
               vector<  unsigned   > ComVec;
               ObjVec.push_back( Abc_ObjFanin0(p6 ));ComVec.push_back(p6->fCompl0);
               ObjVec.push_back( Abc_ObjFanin1(p6 ));ComVec.push_back(p6->fCompl1);            
               ObjVec.push_back( Abc_ObjFanin0(p7 ));ComVec.push_back(p7->fCompl0);
               ObjVec.push_back( Abc_ObjFanin1(p7 ));ComVec.push_back(p7->fCompl1);
               for (int v=0;v<ObjVec.size();v++){
                  if (Abc_ObjFaninNum(ObjVec[v])!=2 or ComVec[v]!=1 ){continue;}
                  if ( v<2){ 
                      Abc_Obj_t * pb= Abc_ObjFanin0(ObjVec[v]);
                      Abc_Obj_t * pc= Abc_ObjFanin1(ObjVec[v]);
                      unsigned   Cb=ObjVec[v]->fCompl0;
                      unsigned   Cc=ObjVec[v]->fCompl1;
                      if (pb!=ObjVec[2] or pc!=ObjVec[3]){continue;}
                      if (ObjVec[1-v]==pb or ObjVec[1-v]==pc){continue;}
                      if (Cb==ComVec[2] or Cc==ComVec[3]){continue;}
                    //ComVec[1-v]==0;Cb==1; Cc==1
                      printf("%2d = MAJ(%2s%2d,%2s%2d,%2s%2d)\n",Abc_ObjId(pObj),
                       (ComVec[1-v]==0)^(com==1)? "-":"", Abc_ObjId(ObjVec[1-v]),
                       (Cb==1)^(com==1)?"-":"", Abc_ObjId(ObjVec[2]),
                       (Cc==1)^(com==1)?"-":"", Abc_ObjId(ObjVec[3]));  
                       count++;        
                  }
                  else { 
                      Abc_Obj_t * pb= Abc_ObjFanin0(ObjVec[v]);
                      Abc_Obj_t * pc= Abc_ObjFanin1(ObjVec[v]);
                      unsigned   Cb=ObjVec[v]->fCompl0;
                      unsigned   Cc=ObjVec[v]->fCompl1;
                      if (pb!=ObjVec[0] or pc!=ObjVec[1]){continue;}
                      if (ObjVec[5-v]==pb or ObjVec[5-v]==pc){continue;}
                      if (Cb==ComVec[0] or Cc==ComVec[1]){continue;}
                    //ComVec[1-v]==0;Cb==1; Cc==1
                      printf("%2d = MAJ(%2s%2d,%2s%2d,%2s%2d)\n",Abc_ObjId(pObj),
                       (ComVec[5-v]==0)^(com==1)? "-":"", Abc_ObjId(ObjVec[5-v]),
                       (Cb==1)^(com==1)?"-":"", Abc_ObjId(ObjVec[0]),
                       (Cc==1)^(com==1)?"-":"", Abc_ObjId(ObjVec[1]));
                       count++;
                  }
               }
               

    
  }
/*
  Abc_NtkForEachObj( pNtk, pObj, i){
       if (Abc_ObjFaninNum( pObj)==0){continue;}
       printf("FanNum: %2d, Node:%2d  ,Fanin0:%2d %2d %2d, Fanin1:%2d %2d\n",Abc_ObjFaninNum( pObj),Abc_ObjId            
       (pObj),Abc_ObjId(Abc_ObjFanin0(pObj )),
        Abc_ObjFaninId0(pObj), pObj->fCompl0,Abc_ObjFaninId1(pObj), pObj->fCompl1 );
    
  }*/

  printf("\nTotal MAJ Gates: %2d\n",count);

}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

