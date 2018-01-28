/**CppFile****************************************************************

  FileName    [ecoSearchBase.cpp]

  SystemName  [2017 CAD contest problem A]

  Synopsis    [Resource-aware patch generation: main function]

  Author      [Li-Chen Chen]
  
  Affiliation [NTU]

  Date        [21, July, 2017]

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

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

Vec_Ptr_t* RPGenMgr::searchBase( sat_solver * pSat ) {
   cout << "[INFO] Hank Part start---------------------------------------------------" << endl;
                           #ifndef NDEBUG
                           cout << "DEBUGGING" << endl;
                           #endif
   VecLit allMiterLits, PiMiterLits; // candidate variable in miter qbf solver.
   genAllMiterLits( allMiterLits, PiMiterLits );
   compactLits( allMiterLits, _miterVarToCandVar, _candVarToMiterVar ); // ignore the "not-candidate" variables. Rearrange the variables.
   VecLit allCandLits, PiCandLits;
   convertMiterLitsToCandLits( allMiterLits, allCandLits );
   convertMiterLitsToCandLits( PiMiterLits, PiCandLits );
   
   const int targetSize = Abc_NtkPiNum( _pNtkF ) - Abc_NtkPiNum( _pNtkG );
   if( targetSize != 1 ) { cerr << "We only deal with single target." << endl; return NULL; }
   VecLit baseCand; // use miter variables rather than cand variables.
   for( int i = Abc_NtkPiNum( _pNtkG ); i < Abc_NtkPiNum( _pNtkF ); ++i ) {
      searchBaseSingle( allCandLits, PiCandLits, Abc_NtkPi( _pNtkF, i ), baseCand );
      //cout << "We choose " << base << " to fix target 0" << endl;
   }
   Vec_Ptr_t * vBase = convertCandLitsToObjects( baseCand );
   //Vec_Ptr_t * vBase = Hank::mkVecLit( baseMiter );

   cout << "[INFO] Hank Part end-----------------------------------------------------" << endl;
   return vBase;
}

void RPGenMgr::searchBaseSingle( const VecLit& allLits, const VecLit& PiLits, Abc_Obj_t* pTarget , VecLit& baseLits ) {
   VecLit tfoLits, candLits;
   targetAffectedLits( pTarget, tfoLits );
   genTargetCandidateLits( allLits, tfoLits, candLits );
  /* 
   int qbfRet = solve( _pSat, PiLits );
   using Minisat::Solver;
   Solver candSolver();
   */
   PiLits.copyTo( baseLits );
}


void RPGenMgr::compactLits( const VecLit& oldLits, vec<Var>& old2New, vec<Var>& new2Old ) {
   assert( !oldLits.empty() );
   using namespace Hank;
   assert( isPositiveAndSorted( oldLits ) );
   old2New.clear();
   new2Old.clear();
   
   Var smallestVar, biggestVar;
   getSmallestAndBiggest( oldLits, smallestVar, biggestVar );
   int varSize = biggestVar - smallestVar;
   old2New.growTo( biggestVar + 1, -1 );
   new2Old.growTo( varSize + 2, -1 );
   for( int i = 0; i < oldLits.size(); ++i ) {
      const Var oldV = var( oldLits[i] );
      old2New[ oldV ] = i+1; 
      new2Old[ i+1 ] = oldV;
   }
   #ifndef NDEBUG
   /*
   for( int i = 0; i < oldLits.size(); ++i ) {
      const Var oldV = var( oldLits[i] );
      const Var newV = old2New[ oldV ];
      assert( new2Old[newV] == oldV );
      cout << oldV << " get a new variable " << newV << endl;
   }
   */
   cout << " newVar is from 1 to " << old2New[ var( oldLits.last() ) ] << endl;
   #endif
}



void RPGenMgr::convertMiterLitsToCandLits( const VecLit& miterLits, VecLit& candLits ) {
   candLits.clear();
   candLits.growTo( miterLits.size() );
   for( int i = 0; i < miterLits.size(); ++i ) candLits[i] = getCandLitByMiterLit( miterLits[i] );
}

void RPGenMgr::convertCandLitsToMiterLits( const VecLit& candLits, VecLit& miterLits ) {
   miterLits.clear();
   miterLits.growTo( candLits.size() );
   for( int i = 0; i < candLits.size(); ++i ) miterLits[i] = getMiterLitByCandLit( candLits[i] );
}

Vec_Ptr_t * RPGenMgr::convertCandLitsToObjects( const VecLit& candLits ) {
   Vec_Ptr_t * ret = Vec_PtrStart( candLits.size() );
   for( int i = 0; i < candLits.size(); ++i ) {
      Abc_Obj_t * pObj = getObjectByCandVar( var(candLits[i]) );
      Vec_PtrWriteEntry( ret, i, pObj );
   }
   return ret;
}

void RPGenMgr::genAllMiterLits( VecLit& allMiterLits, VecLit& PiMiterLits ) { // Hank
   using namespace Hank;
   Abc_Obj_t * pObj;
   int i;
   allMiterLits.clear();
   PiMiterLits.clear();

   Abc_NtkForEachPi( _pNtkF , pObj , i )
   {
      if ( i == Abc_NtkPiNum(_pNtkG) ) break;
      Lit lit = getMiterLit( pObj ); 
      PiMiterLits.push( lit );
      allMiterLits.push( lit );
   }
   assert( isPositiveAndSorted( PiMiterLits ) );
   cout << "  > PiMiterLits = " << PiMiterLits << endl;

   Abc_NtkForEachNode( _pNtkF , pObj , i )
   {
      //if ( !Abc_ObjFanoutNum(pObj) ) continue;
      if ( getWeight( pObj ) == -1 ) continue;
      Lit lit = getMiterLit( pObj ); 
      allMiterLits.push( lit );
   }
   assert( isPositiveAndSorted( allMiterLits ) );
   cout << "  > allMiterLits = " << allMiterLits << endl;
}

void RPGenMgr::targetAffectedLits( Abc_Obj_t* pTarget, VecLit& vTfos ) {
   using namespace Hank;
   vTfos.clear();
   Abc_NtkIncrementTravId( _pNtkF );
   targetAffectedLits_rec( pTarget, vTfos );
   sort( vTfos );
   cout << " tfo = " << vTfos << endl; 
   assert( isPositiveAndSorted( vTfos ) );
}

void RPGenMgr::targetAffectedLits_rec( Abc_Obj_t * pObj, VecLit& vTfos ) {
   if( Abc_NodeIsTravIdCurrent(pObj) ) return;
   Var miterVar = getMiterVar( pObj ); 
   #if 1 // we shows that a gate in TFO of Target points may has weight! ( even the case is single-target-case. )
   if( miterVar != -1 ) { 
      assert( getWeight( pObj ) != -1 );
      Var candVar = getCandVarByMiterVar( miterVar );
      vTfos.push( mkLit( candVar ) );
   }
   #else // this section is not correct.
   assert(0);
   if( miterVar == -1 ) return; 
   assert( getWeight( pObj ) != -1 );
   Var candVar = getCandVarByMiterVar( miterVar );
   vTfos.push( mkLit( candVar ) );
   #endif
   Abc_Obj_t * pFanout;
   int i;
   Abc_ObjForEachFanout( pObj, pFanout, i ) targetAffectedLits_rec( pFanout, vTfos );   
   Abc_NodeSetTravIdCurrent( pObj );
}

void RPGenMgr::popOverWeightLits( VecLit& candLits ) {

}



void RPGenMgr::genTargetCandidateLits( const VecLit& allLits, const VecLit& tfoLits, VecLit& candLits ) {
   candLits.clear();
   Hank::minus( allLits, tfoLits, candLits );
   popOverWeightLits( candLits );
}

int RPGenMgr::solve( sat_solver* pSat, const VecLit& ass ) {
   static const ABC_INT64_T nBTLimit = 1000000;
   Vec_Int_t * assump = Hank::mkVecLit( ass );
   int status = sat_solver_solve( pSat, Vec_IntArray(assump), Vec_IntArray(assump) + Vec_IntSize(assump), nBTLimit, 0, 0, 0 );
   Vec_IntFree( assump );
   assert( status == -1 || status == 0 || status == 1 );
   #define PRINT 1
   #if PRINT 
   if( status == -1 ) cout << ass <<  " is a legal base." << endl;
   else if( status == 1 ) cout << ass << " is NOT a ligal base" << endl;
   else assert(0);
   #endif
   #undef PRINT
   return status;
}

