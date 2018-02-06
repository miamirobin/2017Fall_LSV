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
#include "sat/bsat2/Solver.h"
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

int get_max_id(Abc_Ntk_t *pNtk ){
    Abc_Obj_t * pObj;
    int i,max=0;
    Abc_NtkForEachNode(pNtk,pObj,i){
        if ( Abc_ObjId(pObj) > max )
            max = Abc_ObjId(pObj);
    }
    return max;
}

int get_pi_num(Abc_Ntk_t *pNtk ){
    Abc_Obj_t *pObj;
    int i;
    Abc_NtkForEachPi( pNtk, pObj, i ) {
        ;
    }
    return i;
}
void addClause(Minisat::Solver& solver,int v1, int v2, int v3,int bb1,int bb2){
    Minisat::vec<Minisat::Lit> temp ;
    Minisat::Lit l1=Minisat::mkLit(v1,0),l2=Minisat::mkLit(v2,bb1),l3=Minisat::mkLit(v3,bb2);
    
    temp.clear();
    temp.push(~l1);  temp.push( l2);  solver.addClause(temp); temp.clear();
    
    temp.push(~l1);  temp.push( l3);  solver.addClause(temp); temp.clear();
    
    temp.push(l1);  temp.push( ~l2); temp.push( ~l3); solver.addClause(temp); temp.clear();
}

void
Lsv_NtkMajDecompose( Abc_Ntk_t * pNtk,int selected_po )
{  
	//int n_po = Abc_NtkPoNum( pNtk );
    Abc_Obj_t *po_t,*pObj;
    int i,j,id_max,n_pi,i_po;
    id_max = get_max_id( pNtk );
    n_pi   = get_pi_num( pNtk );

    Minisat::vec<Minisat::Lit> temp;
    Minisat::Lit l1,l2,l3,l4,lk1,lk2,lk3;
    Minisat::Solver solver;

	po_t = Abc_NtkPo(pNtk,selected_po);
    i_po = Abc_ObjId(po_t);

    int *varList0,*varList1,*varList2,*varList3,*bindVar1,*bindVar2,*bindVar3;
    varList0=(int*)malloc( (id_max+1) * sizeof(int) );
    varList1=(int*)malloc( (id_max+1) * sizeof(int) );
    varList2=(int*)malloc( (id_max+1) * sizeof(int) );
    varList3=(int*)malloc( (id_max+1) * sizeof(int) );
    bindVar1=(int*)malloc( (n_pi) * sizeof(int) );
    bindVar2=(int*)malloc( (n_pi) * sizeof(int) );
    bindVar3=(int*)malloc( (n_pi) * sizeof(int) );

    for(i=0;i<id_max+1;i++){
        varList0[i]=solver.newVar();
        varList1[i]=solver.newVar();
        varList2[i]=solver.newVar();
        varList3[i]=solver.newVar();
    }

    for(i=0;i<n_pi;++i)
    {
        bindVar1[i]=solver.newVar();
        bindVar2[i]=solver.newVar();
        bindVar3[i]=solver.newVar();
    }

    Abc_NtkForEachNode(pNtk,pObj,i){
        //printf("%d %d %d %d %d \n",Abc_ObjId(pObj),Abc_ObjFaninId0(pObj),Abc_ObjFaninId1(pObj),Abc_ObjFaninC0(pObj),Abc_ObjFaninC1(pObj));
        addClause( solver,varList0[Abc_ObjId(pObj)],varList0[Abc_ObjFaninId0(pObj)],varList0[Abc_ObjFaninId1(pObj)],
                    Abc_ObjFaninC0(pObj),Abc_ObjFaninC1(pObj));

        addClause( solver,varList1[Abc_ObjId(pObj)],varList1[Abc_ObjFaninId0(pObj)],varList1[Abc_ObjFaninId1(pObj)],
                    Abc_ObjFaninC0(pObj),Abc_ObjFaninC1(pObj));

        addClause( solver,varList2[Abc_ObjId(pObj)],varList2[Abc_ObjFaninId0(pObj)],varList2[Abc_ObjFaninId1(pObj)],
                    Abc_ObjFaninC0(pObj),Abc_ObjFaninC1(pObj));

        addClause( solver,varList3[Abc_ObjId(pObj)],varList3[Abc_ObjFaninId0(pObj)],varList3[Abc_ObjFaninId1(pObj)],
                    Abc_ObjFaninC0(pObj),Abc_ObjFaninC1(pObj));
    }

    //printf("%d %d %d %d %d \n",Abc_ObjId(po_t),Abc_ObjFaninId0(po_t),Abc_ObjFaninId1(po_t),Abc_ObjFaninC0(po_t),Abc_ObjFaninC1(po_t));
    addClause( solver,varList0[Abc_ObjId(po_t)],varList0[Abc_ObjFaninId0(po_t)],varList0[Abc_ObjFaninId1(po_t)],
                    Abc_ObjFaninC0(po_t),Abc_ObjFaninC1(po_t));

    addClause( solver,varList1[Abc_ObjId(po_t)],varList1[Abc_ObjFaninId0(po_t)],varList1[Abc_ObjFaninId1(po_t)],
                Abc_ObjFaninC0(po_t),Abc_ObjFaninC1(po_t));

    addClause( solver,varList2[Abc_ObjId(po_t)],varList2[Abc_ObjFaninId0(po_t)],varList2[Abc_ObjFaninId1(po_t)],
                Abc_ObjFaninC0(po_t),Abc_ObjFaninC1(po_t));

    addClause( solver,varList3[Abc_ObjId(po_t)],varList3[Abc_ObjFaninId0(po_t)],varList3[Abc_ObjFaninId1(po_t)],
                Abc_ObjFaninC0(po_t),Abc_ObjFaninC1(po_t));

	
    Minisat::Lit FO0 = Minisat::mkLit(varList0[i_po],0);
    Minisat::Lit FO1 = Minisat::mkLit(varList1[i_po],1);
    Minisat::Lit FO2 = Minisat::mkLit(varList2[i_po],1);
    Minisat::Lit FO3 = Minisat::mkLit(varList3[i_po],1);

	temp.clear(); temp.push(FO0); solver.addClause(temp);
	temp.clear(); temp.push(FO1); solver.addClause(temp);
	temp.clear(); temp.push(FO2); solver.addClause(temp);
	temp.clear(); temp.push(FO3); solver.addClause(temp);
    
    Abc_NtkForEachPi( pNtk, pObj, i ) {
        l1=Minisat::mkLit(varList0[Abc_ObjId(pObj)]);
        l2=Minisat::mkLit(varList1[Abc_ObjId(pObj)]);
        l3=Minisat::mkLit(varList2[Abc_ObjId(pObj)]);
        l4=Minisat::mkLit(varList3[Abc_ObjId(pObj)]);
        lk1=Minisat::mkLit(bindVar1[i]);
        lk2=Minisat::mkLit(bindVar2[i]);
        lk3=Minisat::mkLit(bindVar3[i]);
        temp.clear(); temp.push( l1);  temp.push(~l2); temp.push( lk1); solver.addClause(temp);
        temp.clear(); temp.push(~l1);  temp.push( l2); temp.push( lk1); solver.addClause(temp);

        temp.clear(); temp.push( l1);  temp.push(~l3); temp.push( lk2); solver.addClause(temp);
        temp.clear(); temp.push(~l1);  temp.push( l3); temp.push( lk2); solver.addClause(temp);

        temp.clear(); temp.push( l1);  temp.push(~l4); temp.push( lk3); solver.addClause(temp);
        temp.clear(); temp.push(~l1);  temp.push( l4); temp.push( lk3); solver.addClause(temp);
    }
    
    temp.clear(); temp.push(Minisat::mkLit(varList0[0])); solver.addClause(temp);
    temp.clear(); temp.push(Minisat::mkLit(varList1[0])); solver.addClause(temp);
    temp.clear(); temp.push(Minisat::mkLit(varList2[0])); solver.addClause(temp);
    temp.clear(); temp.push(Minisat::mkLit(varList3[0])); solver.addClause(temp);

    Minisat::vec<Minisat::Lit> asrt ;



    int found_seed=0;
    for(int ii=0;ii<n_pi-2;ii++){
        for(int jj=ii+1;jj<n_pi-1;jj++){
            for(int kk=jj+1;kk<n_pi;kk++){
                if(found_seed) break;
                asrt.clear();
                asrt.push(Minisat::mkLit(bindVar1[0],0)); asrt.push(Minisat::mkLit(bindVar2[0],1)); asrt.push(Minisat::mkLit(bindVar3[0],1));
                asrt.push(Minisat::mkLit(bindVar1[1],1)); asrt.push(Minisat::mkLit(bindVar2[1],0)); asrt.push(Minisat::mkLit(bindVar3[1],1));
                asrt.push(Minisat::mkLit(bindVar1[2],1)); asrt.push(Minisat::mkLit(bindVar2[2],1)); asrt.push(Minisat::mkLit(bindVar3[2],0));

                for(i=3;i<n_pi;i++){
                    asrt.push(Minisat::mkLit(bindVar1[i],i==ii?0:1));
                    asrt.push(Minisat::mkLit(bindVar2[i],i==jj?0:1));
                    asrt.push(Minisat::mkLit(bindVar3[i],i==kk?0:1));
                }
                found_seed = !solver.solve(asrt);
            }
        }
    }
    if(!found_seed){
        printf("NO SOLUTION\n");
        free(varList0);
        free(varList1);
        free(varList2);
        free(varList3);
        free(bindVar1);
        free(bindVar2);
        free(bindVar3);
        return;
    }
/*
    for(i=0;i<solver.conflict.size();i++){
        printf("%d ",var(solver.conflict[i]));
    }
    printf("\n");

    for(i=0;i<n_pi;i++){
        printf("%d ",bindVar1[i]);
        printf("%d ",bindVar2[i]);
        printf("%d\n",bindVar3[i]);
    }
    printf("\n");
*/
    int** list;
    int** bindVar = (int**)malloc( 3 * sizeof(int*) );
    bindVar[0]=bindVar1;
    bindVar[1]=bindVar2;
    bindVar[2]=bindVar3;

    list = (int**)malloc( n_pi * sizeof(int*) );
    for(i=0;i<n_pi;i++)
        list[i] = (int*)malloc( 3 * sizeof(int) );

    for(i=0;i<n_pi;i++){
        for(j=0;j<3;j++){
            list[i][j]=1;
            for(int k=0;k<solver.conflict.size();k++){
                if((int)var(solver.conflict[k])==bindVar[j][i])
                    list[i][j]=0;
            }
        }
    }
/*
    for(i=0;i<n_pi;i++){
        for(j=0;j<3;j++){
            printf( "%d ",list[i][j]);
        }
        printf("\n");
    }
*/
    int X[3]={0,0,0};
    int *part = (int*)malloc( n_pi * sizeof(int) );
    int solution=1;
    for(i=0;i<n_pi;i++){
        int min =-1;
        for(j=0;j<3;j++){
            if(list[i][j]){
                if(min==-1) min =j;
                else if(X[min]>X[j]) min = j;
            }
        }
        if(min==-1){printf("NO SOLUTION\n"); solution=0;break;}
        else
            X[min]++;
        part[i]=min;

    }
    if(solution){
        printf("%d %d %d \n",X[0],X[1],X[2]);
        asrt.clear();
        for(i=0;i<n_pi;i++){
            asrt.push(Minisat::mkLit(bindVar1[i],part[i]==0?0:1));
            asrt.push(Minisat::mkLit(bindVar2[i],part[i]==1?0:1));
            asrt.push(Minisat::mkLit(bindVar3[i],part[i]==2?0:1));
        }
        printf("solve again: %d\n",solver.solve(asrt));
        
    }


    free(varList0);
    free(varList1);
    free(varList2);
    free(varList3);
    free(bindVar1);
    free(bindVar2);
    free(bindVar3);
    free(part);
    for(i=0;i<n_pi;i++)
        free(list[i]);
    free(list);
    free(bindVar);
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

