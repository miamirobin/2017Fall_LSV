/**CFile****************************************************************

  FileName    [ecoGenPatch.cpp]

  SystemName  [2017 CAD contest problem A]

  Synopsis    [Resource-aware patch generation: main function]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [4, July, 2017]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "eco/RPGenMgr.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

extern "C" {
   void        Abc_NtkShow      ( Abc_Ntk_t * , int , int , int );
   Abc_Obj_t * Abc_AigMiter     ( Abc_Aig_t * , Vec_Ptr_t * , int );
   Abc_Obj_t * Abc_AigMiter2    ( Abc_Aig_t * , Vec_Ptr_t * );
   Aig_Man_t * Nz_NtkToDarMap   ( Abc_Ntk_t * , int , Vec_Int_t * );
}

static int Eco_NodeCompareIdsIncrease( Abc_Obj_t ** pp1 , Abc_Obj_t ** pp2 );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////
/**Function*************************************************************

  Synopsis    [Patch generation: main entrance]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
RPGenMgr::genPatch( bool fVerbose )
{
   Abc_Ntk_t * pNtkRes;
   Vec_Ptr_t * vPatchIn;
   
   printf( "[INFO] Build checking SAT solver ...\n" );
   _pSat = buildCheckSatSolver();
   printf( "[INFO] Start searching candidates ...\n" );
   vPatchIn = searchBase( _pSat );
   printf( "[INFO] Derive the patch ...\n" );
   pNtkRes = _pNtkP = interpolatePatch( vPatchIn , fVerbose );
   Vec_PtrFree( vPatchIn );
   return pNtkRes;
}

/**Function*************************************************************

  Synopsis    [Patch generation: network profiling]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
RPGenMgr::profileNetwork() const
{
   Vec_Ptr_t * vPos , * vAllPos;
   Abc_Obj_t * pObj , * pEntry;
   int i , j;

   /*Abc_NtkForEachNode( _pNtkF , pObj , i )
   {
      if ( Abc_NodeIsConst(pObj) )
         printf( "    > Node %s is constant\n" , Abc_ObjName(pObj) );
      if ( !Abc_ObjFanoutNum(pObj) ) {
         printf( "    > Node %s is dangling\n" , Abc_ObjName(pObj) );
         fflush(stdout);
         assert( Vec_IntEntry( _weight , Abc_ObjId(pObj) ) == -1 );
      }
      if ( !Abc_ObjFaninNum(pObj) && Abc_ObjFanoutNum(pObj) )
         printf( "    > [WARNING] Node %s has no fanin but fanouts\n" , Abc_ObjName(pObj) );
   }*/
   /*int total = 0;
   Abc_NtkForEachNode( _pNtkF , pObj , i )
   {
      if ( Vec_IntEntry( _weight , Abc_ObjId(pObj) ) == -1 ) continue;
      total += Vec_IntEntry( _weight , Abc_ObjId(pObj) );
   }
   printf( "[INFO] total weight = %d\n" , total );*/
   vAllPos = Vec_PtrAlloc( Abc_NtkPoNum(_pNtkF) );
   for ( i = Abc_NtkPiNum(_pNtkG) ; i < Abc_NtkPiNum(_pNtkF) ; ++i ) {
      pObj = Abc_NtkPi( _pNtkF , i );
      printf( "  > Check %s affected Pos:\n    > " , Abc_ObjName(pObj) );
      vPos = targetAffectedPos( pObj );
      Vec_PtrForEachEntry( Abc_Obj_t * , vPos , pEntry , j )
      {
         printf( "%s " , Abc_ObjName(pEntry) );
         Vec_PtrPushUnique( vAllPos , pEntry );
      }
      printf( "\n" );
      Vec_PtrFree( vPos );
   }
   printf( "  > Inequivalent Pos: %3d / %3d\n" , Vec_PtrSize(vAllPos) , Abc_NtkPoNum(_pNtkF) );
   Vec_PtrFree( vAllPos );
}

int
Eco_NodeCompareIdsIncrease( Abc_Obj_t ** pp1 , Abc_Obj_t ** pp2 )
{
    int Diff = Abc_ObjRegular(*pp1)->Id - Abc_ObjRegular(*pp2)->Id;
    if ( Diff < 0 )
        return -1;
    if ( Diff > 0 ) 
        return 1;
    return 0; 
}

Vec_Ptr_t*
RPGenMgr::targetAffectedPos( Abc_Obj_t * pObj ) const
{
   Vec_Ptr_t * vPos;
   vPos = Vec_PtrAlloc( Abc_NtkPoNum(_pNtkG) );
   Abc_NtkIncrementTravId( _pNtkF );
   targetAffectedPos_rec( pObj , vPos );
   Vec_PtrSort( vPos , (int (*)(void))Eco_NodeCompareIdsIncrease );
   return vPos;
}

void
RPGenMgr::targetAffectedPos_rec( Abc_Obj_t * pObj , Vec_Ptr_t * vPos ) const
{
   if ( Abc_NodeIsTravIdCurrent(pObj) ) return;
   if ( Abc_ObjIsPo(pObj) ) Vec_PtrPush( vPos , pObj );
   Abc_Obj_t * pFanout;
   int i;
   Abc_ObjForEachFanout( pObj , pFanout , i )
      targetAffectedPos_rec( pFanout , vPos );
   Abc_NodeSetTravIdCurrent(pObj);
   return;
} 

/**Function*************************************************************

  Synopsis    [Build the SAT solver for checking candidates]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

sat_solver*
RPGenMgr::buildCheckSatSolver()
{
   Abc_Ntk_t * pNtkFAig , * pNtkGAig , * pNtkMiter;
   sat_solver * pSat;

   // preprocess _pNtkF by adding Pis and Pos
   pNtkFAig  = addTargetPiWirePo();
   pNtkGAig  = Abc_NtkStrash( _pNtkG , 0 , 1 , 0 );
   pNtkMiter = buildCheckMiter( pNtkFAig , pNtkGAig );
   pSat      = deriveCheckSatSolver( pNtkFAig , pNtkGAig , pNtkMiter );
   
   Abc_NtkDelete( pNtkFAig  );
   Abc_NtkDelete( pNtkGAig  );
   Abc_NtkDelete( pNtkMiter );
   
   if ( !pSat ) {
      Abc_Print( -1 , "Deriving SAT solver has failed.\n" );
      exit(1);
   }
   return pSat;
}

Abc_Ntk_t*
RPGenMgr::addTargetPiWirePo()
{
   Abc_Ntk_t * pNtkAig;
   Abc_Obj_t * pObj , * pFanin , * pObjNew;
   map<char* , Abc_Obj_t*> targetMap;
   map<char* , Abc_Obj_t*>::iterator it;
   int nTarget = 0 , i , j;

   // convert targets to new Pis
   Abc_NtkForEachNode( _pNtkF , pObj , i )
   {
      Abc_ObjForEachFanin( pObj , pFanin , j )
      {
         if ( strncmp( Abc_ObjName(pFanin) , "t_num" , 2 ) == 0 ) {
            printf( "  > Replace targer point %s by Pi at node %s\n" , Abc_ObjName(pFanin) , Abc_ObjName(pObj) );
            it = targetMap.find( Abc_ObjName(pFanin) );
            if ( it == targetMap.end() ) {
               ++nTarget;
               pObjNew = Abc_NtkCreatePi( _pNtkF );
               Abc_ObjAssignName( pObjNew , Abc_ObjName(pFanin) , "" );
               targetMap.insert( pair<char*,Abc_Obj_t*>( Abc_ObjName(pFanin) , pObjNew ) );
            }
            else pObjNew = it->second;
            Abc_ObjPatchFanin( pObj , pFanin , pObjNew );
         }
      }
   }
   printf( "  > There are %d target point(s)\n" , nTarget );
   
   //printf( "[INFO] Profile old implementation ...\n" );
   //profileNetwork();
   
   // convert wires to new Pos
   _vNodeMapPo = Vec_IntStart( Abc_NtkObjNumMax( _pNtkF ) );
   Abc_NtkForEachPi( _pNtkF , pObj , i )
      if ( i < Abc_NtkPiNum( _pNtkG ) )
         Vec_IntWriteEntry( _vNodeMapPo , Abc_ObjId(pObj) , i );
   j = 0;
   Abc_NtkForEachNode( _pNtkF , pObj , i )
   {
      if ( Vec_IntEntry( _weight , Abc_ObjId(pObj) ) == -1 ) continue;
      pObjNew = Abc_NtkCreatePo( _pNtkF );
      Abc_ObjAssignName( pObjNew , Abc_ObjName(pObj) , "_po" );
      Abc_ObjAddFanin( pObjNew , pObj );
      Vec_IntWriteEntry( _vNodeMapPo , Abc_ObjId(pObj) , Abc_NtkPoNum(_pNtkG) + (j++) );
   }
   if ( !Abc_NtkCheck( _pNtkF ) ) {
      Abc_Print( -1 , "Adding Pi/Po to _pNtkF has failed.\n" );
      exit(1);
   }
   pNtkAig = Abc_NtkStrash( _pNtkF , 0 , 1 , 0 );
   return pNtkAig;
}

/**Function*************************************************************

  Synopsis    [Build the checking miter]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
RPGenMgr::buildCheckMiter( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig ) const
{
   char Buffer[16];
   Abc_Ntk_t * pNtkMiter;

   pNtkMiter = Abc_NtkAlloc( ABC_NTK_STRASH , ABC_FUNC_AIG , 1 );
   sprintf( Buffer , "check_miter" );
   pNtkMiter->pName = Extra_UtilStrsav( Buffer );
   buildMiterCreatePio ( pNtkFAig , pNtkGAig , pNtkMiter );
   buildMiterCreateGate( pNtkFAig , pNtkGAig , pNtkMiter );
   buildMiterFinalize  ( pNtkFAig , pNtkGAig , pNtkMiter );

   if ( !Abc_NtkCheck( pNtkMiter ) ) {
      Abc_Print( -1 , "Abc_NtkMiter: The network check has failed.\n" );
      Abc_NtkDelete( pNtkMiter );
      exit(1);
   }
   return pNtkMiter;
}

void
RPGenMgr::buildMiterCreatePio( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkMiter ) const
{
   Abc_Obj_t * pObj , * pObjNew;
   int i;

   // map constant node
   Abc_AigConst1(pNtkFAig)->pCopy = Abc_AigConst1(pNtkMiter);
   Abc_AigConst1(pNtkGAig)->pCopy = Abc_AigConst1(pNtkMiter);
   Abc_AigConst1(pNtkFAig)->pData = Abc_AigConst1(pNtkMiter);
   Abc_AigConst1(pNtkGAig)->pData = Abc_AigConst1(pNtkMiter);
   // create Pis: X1 (pCopy)
   Abc_NtkForEachPi( pNtkFAig , pObj , i )
   {
      pObjNew = Abc_NtkCreatePi( pNtkMiter );
      Abc_ObjAssignName( pObjNew , Abc_ObjName(pObj) , "_1" );
      pObj->pCopy = pObjNew;
      if ( i < Abc_NtkPiNum(pNtkGAig) ) {
         pObj = Abc_NtkPi( pNtkGAig , i );
         pObj->pCopy = pObjNew;
      }
   }
   // create Pis: X2 (pData)
   Abc_NtkForEachPi( pNtkFAig , pObj , i )
   {
      pObjNew = Abc_NtkCreatePi( pNtkMiter );
      Abc_ObjAssignName( pObjNew , Abc_ObjName(pObj) , "_2" );
      pObj->pData = pObjNew;
      if ( i < Abc_NtkPiNum(pNtkGAig) ) {
         pObj = Abc_NtkPi( pNtkGAig , i );
         pObj->pData = pObjNew;
      }
   }
   // create Po
   pObjNew = Abc_NtkCreatePo( pNtkMiter );
   Abc_ObjAssignName( pObjNew , "miter" , NULL );
}

void
RPGenMgr::buildMiterCreateGate( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkMiter ) const
{
   Abc_Obj_t * pNode;
   int i;

   Abc_AigForEachAnd( pNtkFAig , pNode , i )
      pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode) , Abc_ObjChild1Copy(pNode) );
   Abc_AigForEachAnd( pNtkGAig , pNode , i )
      pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Copy(pNode) , Abc_ObjChild1Copy(pNode) );
   Abc_AigForEachAnd( pNtkFAig , pNode , i )
      pNode->pData = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Data(pNode) , Abc_ObjChild1Data(pNode) );
   Abc_AigForEachAnd( pNtkGAig , pNode , i )
      pNode->pData = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc, Abc_ObjChild0Data(pNode) , Abc_ObjChild1Data(pNode) );
}

void
RPGenMgr::buildMiterFinalize( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkMiter ) const
{
   Vec_Ptr_t * vPairs;
   Abc_Obj_t * pNode , * pMiter1 , * pMiter2 , * pMiter;
   int i;

   vPairs = Vec_PtrAlloc( Abc_NtkPoNum(pNtkGAig) );
   Abc_NtkForEachPo( pNtkGAig , pNode , i )
   {
      Vec_PtrPush( vPairs , Abc_ObjChild0Copy(pNode) );
      pNode = Abc_NtkPo( pNtkFAig , i );
      Vec_PtrPush( vPairs , Abc_ObjChild0Copy(pNode) );
   }
   pMiter1 = Abc_AigMiter2( (Abc_Aig_t *)pNtkMiter->pManFunc , vPairs );
   Vec_PtrClear( vPairs );
   Abc_NtkForEachPo( pNtkGAig , pNode , i )
   {
      Vec_PtrPush( vPairs , Abc_ObjChild0Data(pNode) );
      pNode = Abc_NtkPo( pNtkFAig , i );
      Vec_PtrPush( vPairs , Abc_ObjChild0Data(pNode) );
   }
   pMiter2 = Abc_AigMiter2( (Abc_Aig_t *)pNtkMiter->pManFunc , vPairs );
   pMiter  = Abc_AigAnd( (Abc_Aig_t *)pNtkMiter->pManFunc , pMiter1 , pMiter2 );
   Abc_ObjAddFanin( Abc_NtkPo( pNtkMiter , 0 ) , pMiter );
   Vec_PtrFree( vPairs );
}

/**Function*************************************************************

  Synopsis    [Derive checking SAT solver from the miter]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
 
sat_solver*
RPGenMgr::deriveCheckSatSolver( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkMiter )
{
   Vec_Int_t  * vIdMap;
   Aig_Man_t  * pMan;
   Cnf_Dat_t  * pCnf;
   sat_solver * pSat;
   Abc_Obj_t  * pObj , * pPo , * pPi1 , * pPi2;
   int v1 , v2 , v3 , i;

   vIdMap = Vec_IntStart( Abc_NtkObjNumMax(pNtkMiter) );
   pMan   = Nz_NtkToDarMap( pNtkMiter , 0 , vIdMap );
   pCnf   = Cnf_DeriveSimple( pMan , Aig_ManCoNum(pMan) );
   pSat   = (sat_solver*)Cnf_DataWriteIntoSolver( pCnf , 1 , 0 );
   Cnf_DataWriteOrClause( pSat , pCnf ); // assert miter output
   
   // FIXME: cofactor target points (currently only single)
   pPi1 = Abc_NtkPi( pNtkMiter , Abc_NtkPiNum(pNtkFAig)-1 );
   pPi2 = Abc_NtkPi( pNtkMiter , Abc_NtkPiNum(pNtkMiter)-1 );
   v1   = pCnf->pVarNums[Vec_IntEntry( vIdMap , Abc_ObjId(pPi1) )];
   v2   = pCnf->pVarNums[Vec_IntEntry( vIdMap , Abc_ObjId(pPi2) )];
   sat_solver_add_const( pSat , v1 , 1 ); // t_0 = 0
   sat_solver_add_const( pSat , v2 , 0 ); // t_0 = 1
   // allocate assumption variables and add clauses
   _vAssume = Vec_IntStart( Abc_NtkObjNumMax(_pNtkF) );
   Vec_IntFill( _vAssume , Vec_IntSize(_vAssume) , -1 );
   for ( i = 0 ; i < Abc_NtkPiNum(pNtkGAig) ; ++i ) {
      pObj = Abc_NtkPi( _pNtkF , i );
      //printf( "  > _pNtkF pi name %s\n" , Abc_ObjName(pObj) );
      pPi1 = Abc_NtkPi( pNtkMiter , i );
      pPi2 = Abc_NtkPi( pNtkMiter , Abc_NtkPiNum(pNtkFAig)+i );
      v1   = sat_solver_addvar(pSat);
      v2   = pCnf->pVarNums[Vec_IntEntry( vIdMap , Abc_ObjId(pPi1) )];
      v3   = pCnf->pVarNums[Vec_IntEntry( vIdMap , Abc_ObjId(pPi2) )];
      Vec_IntWriteEntry( _vAssume , Abc_ObjId(pObj) , v1 );
      //printf( "    > v1 = %d , v2 = %d , v3 = %d\n" , v1 , v2 , v3 );
      sat_solver_add_buffer_enable( pSat , v2 , v3 , v1 , 0 );
   }
   Abc_NtkForEachNode( _pNtkF , pObj , i )
   {
      if ( Vec_IntEntry( _weight , Abc_ObjId(pObj) ) == -1 ) continue;
      //printf( "  > _pNtkF node name %s\n" , Abc_ObjName(pObj) );
      pPo = Abc_NtkPo( pNtkFAig , Vec_IntEntry( _vNodeMapPo , Abc_ObjId(pObj) ) );
      v1  = sat_solver_addvar(pSat);
      v2  = pCnf->pVarNums[Vec_IntEntry( vIdMap , Abc_ObjId(Abc_ObjRegular(Abc_ObjChild0Copy(pPo))) )];
      v3  = pCnf->pVarNums[Vec_IntEntry( vIdMap , Abc_ObjId(Abc_ObjRegular(Abc_ObjChild0Data(pPo))) )];
      Vec_IntWriteEntry( _vAssume , Abc_ObjId(pObj) , v1 );
      //printf( "    > v1 = %d , v2 = %d , v3 = %d\n" , v1 , v2 , v3 );
      sat_solver_add_buffer_enable( pSat , v2 , v3 , v1 , 0 );
   }
   // Map assumption variables back to pObj
   _vVarToNode = Vec_PtrStart( sat_solver_nvars(pSat)+1 );
   Vec_IntForEachEntry( _vAssume , v1 , i )
   {
      if ( v1 == -1 ) continue;
      Vec_PtrWriteEntry( _vVarToNode , v1 , Abc_NtkObj( _pNtkF , i ) );
   }
   Vec_IntFree ( vIdMap );
   Aig_ManStop ( pMan   );
   Cnf_DataFree( pCnf   );
   return pSat;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
