/**CFile****************************************************************

  FileName    [ecoInterPatch.cpp]

  SystemName  [2017 CAD contest problem A]

  Synopsis    [Generate patch by Craig interpolation]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [17, July, 2017]

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
   Abc_Ntk_t * Abc_NtkFromDar   ( Abc_Ntk_t * , Aig_Man_t * );
   Abc_Ntk_t * Abc_NtkDC2       ( Abc_Ntk_t * , int , int , int , int , int );
   void        Abc_NtkCecFraig  ( Abc_Ntk_t * , Abc_Ntk_t * , int , int );
}

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Generate patch function by interpolation]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
RPGenMgr::interpolatePatch( Vec_Ptr_t * vPatchIn , bool fVerbose ) const
{
   Abc_Obj_t * pObj;
   int i;
   
   if ( fVerbose ) printf( "[INFO] patch inputs:\n" );
   Vec_PtrForEachEntry( Abc_Obj_t * , vPatchIn , pObj , i )
   {
      if ( !Abc_ObjIsPi(pObj) && !Abc_ObjIsNode(pObj) ) {
         printf( "[ERROR] object %s is not Pi neither node --> cannot be patch input!\n" , Abc_ObjName(pObj) );
         exit(1);
      }
      if ( fVerbose ) {
         if ( Abc_ObjIsPi(pObj) )
            printf( "  > pi   %5s , id %d\n" , Abc_ObjName(pObj) , Abc_ObjId(pObj) );
         if ( Abc_ObjIsNode(pObj) )
            printf( "  > node %5s , id %d\n" , Abc_ObjName(pObj) , Abc_ObjId(pObj) );
      }
   }
   return interpolatePatch_int( vPatchIn );
}

Abc_Ntk_t*
RPGenMgr::interpolatePatch_int( Vec_Ptr_t * vPatchIn ) const
{
   Abc_Ntk_t * pNtkFAig , * pNtkGAig , * pNtkOn , * pNtkOff , * pNtkTemp , * pNtkRes;

   pNtkFAig  = Abc_NtkStrash( _pNtkF , 0 , 1 , 0 );
   pNtkGAig  = Abc_NtkStrash( _pNtkG , 0 , 1 , 0 );
   pNtkOn    = buildOnOffNtk( pNtkFAig , pNtkGAig , 1 );
   pNtkOff   = buildOnOffNtk( pNtkFAig , pNtkGAig , 0 );
   pNtkTemp  = derivePatch  ( pNtkFAig , pNtkOn , pNtkOff , vPatchIn );
   pNtkRes   = Abc_NtkDC2   ( pNtkTemp , 0 , 0 , 1 , 0 , 0 );

   /*if ( !verifyEcoResult( pNtkFAig , pNtkGAig , pNtkRes , vPatchIn ) ) {
      Abc_Print( -1 , "NOT EQUIVALENT! Rectification failed.\n" );
      exit(1);
   }*/

   Abc_NtkDelete( pNtkFAig );
   Abc_NtkDelete( pNtkGAig );
   Abc_NtkDelete( pNtkOn   );
   Abc_NtkDelete( pNtkOff  );
   Abc_NtkDelete( pNtkTemp );
   
   return pNtkRes;
}

/**Function*************************************************************

  Synopsis    [Build on/off set networks]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
RPGenMgr::buildOnOffNtk( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , bool fOn ) const
{
   char Buffer[16];
   Abc_Ntk_t * pNtkNew;

   pNtkNew = Abc_NtkAlloc( ABC_NTK_STRASH , ABC_FUNC_AIG , 1 );
   sprintf( Buffer , fOn ? "onset" : "offset" );
   pNtkNew->pName = Extra_UtilStrsav( Buffer );
   buildOnOffCreatePio ( pNtkFAig , pNtkGAig , pNtkNew , fOn );
   buildOnOffCreateGate( pNtkFAig , pNtkGAig , pNtkNew , fOn );
   buildOnOffFinalize  ( pNtkFAig , pNtkGAig , pNtkNew , fOn );

   if ( !Abc_NtkCheck( pNtkNew ) ) {
      Abc_Print( -1 , "Build On/Off network: The network check has failed.\n" );
      exit(1);
   }
   return pNtkNew;
}

void
RPGenMgr::buildOnOffCreatePio( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkNew , bool fOn ) const
{
   Abc_Obj_t * pObj , * pObjNew;
   char suffix[16] , po[16];
   int i;

   sprintf( suffix , fOn ? "_on" : "_off" );
   sprintf( po     , fOn ? "onset" : "offset" );
   if ( fOn ) {
      Abc_AigConst1(pNtkFAig)->pCopy = Abc_AigConst1(pNtkNew);
      Abc_AigConst1(pNtkGAig)->pCopy = Abc_AigConst1(pNtkNew);
      Abc_NtkForEachPi( pNtkFAig , pObj , i )
      {
         pObjNew = Abc_NtkCreatePi( pNtkNew );
         Abc_ObjAssignName( pObjNew , Abc_ObjName(pObj) , suffix );
         pObj->pCopy = pObjNew;
         if ( i < Abc_NtkPiNum(pNtkGAig) ) {
            pObj = Abc_NtkPi( pNtkGAig , i );
            pObj->pCopy = pObjNew;
         }
      }
   }
   else {
      Abc_AigConst1(pNtkFAig)->pData = Abc_AigConst1(pNtkNew);
      Abc_AigConst1(pNtkGAig)->pData = Abc_AigConst1(pNtkNew);
      Abc_NtkForEachPi( pNtkFAig , pObj , i )
      {
         pObjNew = Abc_NtkCreatePi( pNtkNew );
         Abc_ObjAssignName( pObjNew , Abc_ObjName(pObj) , suffix );
         pObj->pData = pObjNew;
         if ( i < Abc_NtkPiNum(pNtkGAig) ) {
            pObj = Abc_NtkPi( pNtkGAig , i );
            pObj->pData = pObjNew;
         }
      }
   }
   pObjNew = Abc_NtkCreatePo( pNtkNew );
   Abc_ObjAssignName( pObjNew , po , NULL );
}

void
RPGenMgr::buildOnOffCreateGate( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkNew , bool fOn ) const
{
   Abc_Obj_t * pNode;
   int i;

   if ( fOn ) {
      Abc_AigForEachAnd( pNtkFAig , pNode , i )
         pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pNode) , Abc_ObjChild1Copy(pNode) );
      Abc_AigForEachAnd( pNtkGAig , pNode , i )
         pNode->pCopy = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Copy(pNode) , Abc_ObjChild1Copy(pNode) );
   }
   else {
      Abc_AigForEachAnd( pNtkFAig , pNode , i )
         pNode->pData = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Data(pNode) , Abc_ObjChild1Data(pNode) );
      Abc_AigForEachAnd( pNtkGAig , pNode , i )
         pNode->pData = Abc_AigAnd( (Abc_Aig_t *)pNtkNew->pManFunc, Abc_ObjChild0Data(pNode) , Abc_ObjChild1Data(pNode) );
   }
}

void
RPGenMgr::buildOnOffFinalize( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkNew , bool fOn ) const
{
   Vec_Ptr_t * vPairs;
   Abc_Obj_t * pNode , * pMiter;
   int i;

   vPairs = Vec_PtrAlloc( Abc_NtkPoNum(pNtkGAig) );
   if ( fOn ) {
      Abc_NtkForEachPo( pNtkGAig , pNode , i )
      {
         Vec_PtrPush( vPairs , Abc_ObjChild0Copy(pNode) );
         pNode = Abc_NtkPo( pNtkFAig , i );
         Vec_PtrPush( vPairs , Abc_ObjChild0Copy(pNode) );
      }
   }
   else {
      Abc_NtkForEachPo( pNtkGAig , pNode , i )
      {
         Vec_PtrPush( vPairs , Abc_ObjChild0Data(pNode) );
         pNode = Abc_NtkPo( pNtkFAig , i );
         Vec_PtrPush( vPairs , Abc_ObjChild0Data(pNode) );
      }
   }
   pMiter = Abc_AigMiter2( (Abc_Aig_t *)pNtkNew->pManFunc , vPairs );
   Abc_ObjAddFanin( Abc_NtkPo( pNtkNew , 0 ) , pMiter );
   Vec_PtrFree( vPairs );
}

/**Function*************************************************************

  Synopsis    [Derive SAT solver from on/off set, interpolation mode]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
RPGenMgr::derivePatch( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkOn , Abc_Ntk_t * pNtkOff , Vec_Ptr_t * vPatchIn ) const
{
   Vec_Int_t  * vIdMapOn , * vIdMapOff , * vVarsAB;
   Aig_Man_t  * pManOn , * pManOff , * pAigRes;
   Cnf_Dat_t  * pCnfOn , * pCnfOff;
   sat_solver * pSat;
   Inta_Man_t * pManInter; 
   void       * pSatCnf;
   Abc_Ntk_t  * pNtkTemp , * pNtkRes , * pNtkModel;
   Abc_Obj_t  * pObj , * pPo , * pPi1 , * pPi2;
   int status , index , v1 , v2 , i;

   vIdMapOn  = Vec_IntStart    ( Abc_NtkObjNumMax(pNtkOn) );
   pManOn    = Nz_NtkToDarMap  ( pNtkOn , 0 , vIdMapOn );
   pCnfOn    = Cnf_DeriveSimple( pManOn , 0 );
   vIdMapOff = Vec_IntStart    ( Abc_NtkObjNumMax(pNtkOff) );
   pManOff   = Nz_NtkToDarMap  ( pNtkOff , 0 , vIdMapOff );
   pCnfOff   = Cnf_DeriveSimple( pManOff , 0 );
   Cnf_DataLift( pCnfOff , pCnfOn->nVars );
   pSat = sat_solver_new();
   sat_solver_store_alloc( pSat );
   sat_solver_setnvars( pSat, pCnfOn->nVars + pCnfOff->nVars );
   
   // add clauses of A
   for ( i = 0 ; i < pCnfOn->nClauses ; i++ ) {
      if ( !sat_solver_addclause( pSat , pCnfOn->pClauses[i] , pCnfOn->pClauses[i+1] ) ) {
         Cnf_DataFree( pCnfOn );
         Cnf_DataFree( pCnfOff );
         sat_solver_delete( pSat );
         return NULL;
      }
   }
   // FIXME: cofactor target points (currently only single)
   pPi1 = Abc_NtkPi( pNtkOn  , Abc_NtkPiNum(pNtkFAig)-1 );
   v1   = pCnfOn ->pVarNums[Vec_IntEntry( vIdMapOn  , Abc_ObjId(pPi1) )];
   sat_solver_add_const( pSat , v1 , 1 ); // t_0 = 0
   sat_solver_store_mark_clauses_a( pSat );
   // add clauses of B
   for ( i = 0 ; i < pCnfOff->nClauses ; i++ ) {
      if ( !sat_solver_addclause( pSat , pCnfOff->pClauses[i] , pCnfOff->pClauses[i+1] ) ) {
         Cnf_DataFree( pCnfOn );
         Cnf_DataFree( pCnfOff );
         sat_solver_delete( pSat );
         return NULL;
      }
   }
   // FIXME: cofactor target points (currently only single)
   pPi2 = Abc_NtkPi( pNtkOff , Abc_NtkPiNum(pNtkFAig)-1 );
   v2   = pCnfOff->pVarNums[Vec_IntEntry( vIdMapOff , Abc_ObjId(pPi2) )];
   sat_solver_add_const( pSat , v2 , 0 ); // t_0 = 1

   // add patch input clauses and collect common variables
   vVarsAB = Vec_IntAlloc( Vec_PtrSize(vPatchIn) );
   Vec_PtrForEachEntry( Abc_Obj_t * , vPatchIn , pObj, i )
   {
      index = Vec_IntEntry( _vNodeMapPo , Abc_ObjId(pObj) );
      if ( Abc_ObjIsPi(pObj) ) {
         pPi1 = Abc_NtkPi( pNtkOn  , index );
         pPi2 = Abc_NtkPi( pNtkOff , index );
         v1   = pCnfOn ->pVarNums[Vec_IntEntry( vIdMapOn  , Abc_ObjId(pPi1) )];
         v2   = pCnfOff->pVarNums[Vec_IntEntry( vIdMapOff , Abc_ObjId(pPi2) )];
         sat_solver_add_buffer( pSat , v1 , v2 , 0 );
      }
      else { // pObj is node
         pPo = Abc_NtkPo( pNtkFAig , index );
         v1  = pCnfOn ->pVarNums[Vec_IntEntry( vIdMapOn  , Abc_ObjId(Abc_ObjRegular(Abc_ObjChild0Copy(pPo))) )];
         v2  = pCnfOff->pVarNums[Vec_IntEntry( vIdMapOff , Abc_ObjId(Abc_ObjRegular(Abc_ObjChild0Data(pPo))) )];
         sat_solver_add_buffer( pSat , v1 , v2 , 0 );
      }
      Vec_IntPush( vVarsAB , v1 );
   }
   sat_solver_store_mark_roots( pSat );
   printf( "[INFO] Prove the miter is UNSAT ...\n" );
   status = sat_solver_solve( pSat , NULL , NULL , (ABC_INT64_T)0 , (ABC_INT64_T)0 , (ABC_INT64_T)0 , (ABC_INT64_T)0 );
   if ( status != -1 ) {  // -1: l_False
      Abc_Print( -1 , "Patch inputs are invalid (the problem is not UNSAT).\n" );
      exit(1);
   }
   
   pSatCnf   = sat_solver_store_release( pSat );
   pManInter = Inta_ManAlloc();
   printf( "[INFO] Perform interpolation ...\n" );
   pAigRes   = (Aig_Man_t *)Inta_ManInterpolate( pManInter , (Sto_Man_t *)pSatCnf , 0 , vVarsAB , 0 );
   pNtkModel = initPatchModel( vPatchIn );
   pNtkTemp  = Abc_NtkFromDar( pNtkModel , pAigRes );
   pNtkRes   = fixPatchInputPhase( pNtkTemp , pNtkFAig , vPatchIn );
   
   Vec_IntFree ( vIdMapOn  );
   Vec_IntFree ( vIdMapOff );
   Vec_IntFree ( vVarsAB   );
   Aig_ManStop ( pManOn    );
   Aig_ManStop ( pManOff   );
   Aig_ManStop ( pAigRes   );
   Cnf_DataFree( pCnfOn    );
   Cnf_DataFree( pCnfOff   );
   Inta_ManFree( pManInter );
   Sto_ManFree ( (Sto_Man_t *)pSatCnf );
   Abc_NtkDelete( pNtkModel );
   Abc_NtkDelete( pNtkTemp  );
   sat_solver_delete( pSat );
   if ( !pNtkRes || !Abc_NtkCheck( pNtkRes ) ) {
      Abc_Print( -1 , "Interpolating patch has failed.\n" );
      exit(1);
   }
   return pNtkRes;
}

Abc_Ntk_t* 
RPGenMgr::initPatchModel( Vec_Ptr_t * vPatchIn ) const
{
   Abc_Ntk_t * pNtkModel;
   Abc_Obj_t * pObj;
   char Buffer[16];
   int i;

   pNtkModel = Abc_NtkAlloc( ABC_NTK_STRASH , ABC_FUNC_AIG , 1 );
   sprintf( Buffer , "patch" );
   pNtkModel->pName = Extra_UtilStrsav( Buffer );

   Vec_PtrForEachEntry( Abc_Obj_t * , vPatchIn , pObj , i )
      Abc_ObjAssignName( Abc_NtkCreatePi(pNtkModel) , Abc_ObjName(pObj) , "" );
   Abc_ObjAssignName( Abc_NtkCreatePo(pNtkModel) , "t_0" , NULL );
   return pNtkModel;
}

Abc_Ntk_t*
RPGenMgr::fixPatchInputPhase( Abc_Ntk_t * pNtkTemp , Abc_Ntk_t * pNtkFAig , Vec_Ptr_t * vPatchIn ) const
{
   Abc_Obj_t * pObj , * pPo , * pPi , * pFanout;
   int i , j;

   Vec_PtrForEachEntry( Abc_Obj_t * , vPatchIn , pObj , i )
   {
      if ( Abc_ObjIsPi(pObj) ) continue;
      printf( "  > Checking phase of node %s\n" , Abc_ObjName(pObj) );
      pPo = Abc_NtkPo( pNtkFAig , Vec_IntEntry( _vNodeMapPo , Abc_ObjId(pObj) ) );
      if ( Abc_ObjFaninC0(pPo) ) {
         pPi = Abc_NtkPi( pNtkTemp , i );
         printf( "    > Patch input %s needs to be negated\n" , Abc_ObjName(pPi) );
         Abc_ObjForEachFanout( pPi , pFanout , j )
            Abc_ObjXorFaninC( pFanout , Abc_ObjFanoutFaninNum( pFanout , pPi ) );
      }
   }
   return Abc_NtkRestrash( pNtkTemp , 1 );
}

/**Function*************************************************************

  Synopsis    [Derive SAT solver from on/off set, interpolation mode]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

bool
RPGenMgr::verifyEcoResult( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkRes , Vec_Ptr_t * vPatchIn ) const
{
   // Use output file and cec command to verify instead
   // WARNING: codes below are very MESSY
   return true;
#if 1
   Abc_Ntk_t * pNtkRecF;
   char Buffer[16];

   pNtkRecF = Abc_NtkAlloc( ABC_NTK_STRASH , ABC_FUNC_AIG , 1 );
   sprintf( Buffer , "rectified_F" );
   pNtkRecF->pName = Extra_UtilStrsav( Buffer );
   buildRecFCreatePio ( pNtkFAig , pNtkGAig , pNtkRecF );
   buildRecFCreateGate( pNtkFAig , pNtkRecF , pNtkRes , vPatchIn );
   buildRecFFinalize  ( pNtkFAig , pNtkRecF );

   if ( !Abc_NtkCheck( pNtkRecF ) ) {
      Abc_Print( -1 , "Build rectified network: The network check has failed.\n" );
      exit(1);
   }
   Abc_NtkCecFraig( pNtkRecF , pNtkGAig , 20 , 0 );
   Abc_NtkDelete( pNtkRecF );
   return true;
#else
   Vec_Ptr_t * vNode;
   Abc_Obj_t * pPi , * pObj , * pTarget;
   int i;

   Abc_NtkCleanCopy(pNtkFAig);
   Abc_NtkCleanData(pNtkFAig);
   Abc_AigConst1(pNtkRes)->pCopy = Abc_AigConst1(pNtkFAig);
   // link patch inputs
   Abc_NtkForEachPi( pNtkRes , pPi , i )
   {
      pObj = (Abc_Obj_t*)Vec_PtrEntry( vPatchIn , i );
      // FIXME: take care phase!
      pPi->pCopy = Abc_ObjFanin0( Abc_NtkPo( pNtkFAig , Vec_IntEntry( _vNodeMapPo , Abc_ObjId(pObj) ) ) );
   }
   // create patch gates
   vNode = Abc_NtkDfs( pNtkRes , 0 );
   Vec_PtrForEachEntry( Abc_Obj_t * , vNode , pObj , i )
      pObj->pCopy = Abc_AigAnd( (Abc_Aig_t*)pNtkFAig->pManFunc , Abc_ObjChild0Copy(pObj) , Abc_ObjChild1Copy(pObj) );
   //Abc_NtkCheck( pNtkFAig );
   
   //Abc_AigForEachAnd( pNtkRes , pObj , i )
     // pObj->pCopy = Abc_AigAnd( (Abc_Aig_t*)pNtkFAig->pManFunc , Abc_ObjChild0Copy(pObj) , Abc_ObjChild1Copy(pObj) );
   // replace target
   // FIXME: may have multiple fanouts
   printf( "  > Last Pi %s\n" , Abc_ObjName(Abc_NtkPi( pNtkFAig , Abc_NtkPiNum(pNtkFAig)-1 )) );
   pTarget = Abc_ObjFanout0( Abc_NtkPi( pNtkFAig , Abc_NtkPiNum(pNtkFAig)-1 ) );
   printf( "  > pTarget %s , pDriver %p\n" , Abc_ObjName(pTarget) , Abc_ObjRegular(Abc_ObjChild0Copy(Abc_NtkPo( pNtkRes , 0 ))) );
   //Abc_ObjPatchFanin( pTarget , Abc_NtkPi( pNtkFAig , Abc_NtkPiNum(pNtkFAig)-1 ) , Abc_ObjChild0Copy(Abc_NtkPo( pNtkRes , 0 )) );
   // delete additional Pi/Po
#if 0
   Vec_Ptr_t * vPos = Vec_PtrAlloc(100);
   Abc_NtkDeleteObj( Abc_NtkPi( pNtkFAig , Abc_NtkPiNum(pNtkFAig)-1 ) );
   for ( i = Abc_NtkPoNum(pNtkGAig) ; i < Abc_NtkPoNum(pNtkFAig) ; ++i )
      Vec_PtrPush( vPos , Abc_NtkPo( pNtkFAig , i ) );
   Vec_PtrForEachEntry( Abc_Obj_t * , vPos , pObj , i ) Abc_NtkDeleteObj( pObj );
   Vec_PtrFree( vPos );
#endif
   Abc_NtkCheck( pNtkFAig );
   Abc_Ntk_t * pNtkPatF = Abc_NtkRestrash( pNtkFAig , 1 );
   //Abc_NtkShow( pNtkFAig , 0 , 0 , 1 );
   Abc_NtkCecFraig( pNtkPatF , pNtkGAig , 20 , 0 );
   Abc_NtkDelete( pNtkPatF );
   return true;
#endif
}

void
RPGenMgr::buildRecFCreatePio( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkGAig , Abc_Ntk_t * pNtkRecF ) const
{
}

void
RPGenMgr::buildRecFCreateGate( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkRes , Abc_Ntk_t * pNtkRecF , Vec_Ptr_t * vPatchIn ) const
{
}

void
RPGenMgr::buildRecFFinalize( Abc_Ntk_t * pNtkFAig , Abc_Ntk_t * pNtkRecF ) const
{
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
