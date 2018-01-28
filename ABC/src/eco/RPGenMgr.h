/**HFile****************************************************************

  FileName    [RPGenMgr.h]

  SystemName  [2017 CAD contest problem A]

  PackageName [eco]

  Synopsis    [Resource-aware Patch Generation top manager]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [4, July, 2017]

***********************************************************************/

#ifndef RPGEN_MGR_H
#define RPGEN_MAG_H

#include <stdio.h>
#include <vector>
#include <string.h>
#include <map>
#include <signal.h>
#include <fstream>
#include <iostream>
#include <eco/ecoMinisat.h>

using namespace std;
using Hank::Var;
using Hank::Lit;
using Hank::mkLit;
using Hank::vec;
using Hank::VecLit;
using Hank::VecVecLit;

// ABC headers
extern "C" {
   #include "base/abc/abc.h"
   #include "base/main/mainInt.h"
   #include "sat/cnf/cnf.h"
   #include "aig/aig/aig.h"
   #include "sat/bsat/satStore.h"
}

// runtime profilier
typedef struct RPGenTimer_ {
   abctime time;
   } RPGenTimer;

//=================================================================================================
// RPGenMgr -- main class:

class RPGenMgr {

public:
   // Constructor / Destructor
   RPGenMgr    ( Abc_Ntk_t * , Abc_Ntk_t * , char * , char * );
   ~RPGenMgr   ();
   // Patch generation: main entrance
   Abc_Ntk_t * genPatch( bool );
   // write output
   void        writeOutput( char * , char * , bool );
   // Testing interface
   void        test();
   // Signal handling
   void        interrupt();
private:
   // member functions
   // read helpers
   void         readWeight             ( char * weight );

   // searchBase() helpers
   Vec_Ptr_t * searchBase( sat_solver * );
   void genAllMiterLits( VecLit&, VecLit& ); // Hank
   void targetAffectedLits( Abc_Obj_t* , VecLit& );
   void targetAffectedLits_rec(  Abc_Obj_t *, VecLit& );
   void genTargetCandidateLits( const VecLit&, const VecLit&, VecLit& );
   void popOverWeightLits( VecLit& );

   int getWeight( Abc_Obj_t * pObj ) const { return Vec_IntEntry( _weight, Abc_ObjId(pObj) ); } //Hank
   Var getMiterVar( Abc_Obj_t * pObj ) const { return Var( Vec_IntEntry( _vAssume, Abc_ObjId(pObj) ) ); } //Hank
   Lit getMiterLit( Abc_Obj_t * pObj, bool inv = false ) const { return mkLit( getMiterVar(pObj), inv ); } //Hank
   Var getCandVarByMiterVar( Var v ) const { return _miterVarToCandVar[ v ]; }
   Var getMiterVarByCandVar( Var v ) const { return _candVarToMiterVar[ v ]; }
   Lit getCandLitByMiterLit( Lit l ) const { return mkLit( getCandVarByMiterVar( var(l) ), sign(l) ); }
   Lit getMiterLitByCandLit( Lit l ) const { return mkLit( getMiterVarByCandVar( var(l) ), sign(l) ); }
   Var getCandVar( Abc_Obj_t * pObj ) const { return getCandVarByMiterVar( getMiterVar(pObj) ); }
   Lit getCandLit( Abc_Obj_t * pObj, bool inv = false ) const { return mkLit( getCandVar( pObj ), inv ); }
   Abc_Obj_t * getObjectByMiterVar( Var v ) { return (Abc_Obj_t*)Vec_PtrEntry( _vVarToNode, v ); }
   Abc_Obj_t * getObjectByCandVar( Var v ) { return getObjectByMiterVar( getMiterVarByCandVar( v ) ); }

   void compactLits( const VecLit&, vec<Var>&, vec<Var>& );
   void convertMiterLitsToCandLits( const VecLit&, VecLit& );
   void convertCandLitsToMiterLits( const VecLit&, VecLit& );
   Vec_Ptr_t * convertCandLitsToObjects( const VecLit& );
   // find y
   void searchBaseSingle( const VecLit&, const VecLit&, Abc_Obj_t* , VecLit& );
   int solve( sat_solver*, const VecLit& ); //abc solve

   // profile networks
   void         profileNetwork         () const;
   Vec_Ptr_t *  targetAffectedPos      ( Abc_Obj_t * ) const;
   void         targetAffectedPos_rec  ( Abc_Obj_t * , Vec_Ptr_t * ) const;
   // create checking SAT solver helpers
   sat_solver * buildCheckSatSolver    ();
   Abc_Ntk_t  * addTargetPiWirePo      ();
   // build checking miter
   Abc_Ntk_t  * buildCheckMiter        ( Abc_Ntk_t * , Abc_Ntk_t * ) const;
   void         buildMiterCreatePio    ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * ) const;
   void         buildMiterCreateGate   ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * ) const;
   void         buildMiterFinalize     ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * ) const;
   // derive sat solver (check mode)
   sat_solver * deriveCheckSatSolver   ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * );
   // interpolate patch helpers
   Abc_Ntk_t  * interpolatePatch       ( Vec_Ptr_t * , bool ) const;
   Abc_Ntk_t  * interpolatePatch_int   ( Vec_Ptr_t * ) const;
   // build on/off set networks
   Abc_Ntk_t  * buildOnOffNtk          ( Abc_Ntk_t * , Abc_Ntk_t * , bool ) const;
   void         buildOnOffCreatePio    ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , bool ) const;
   void         buildOnOffCreateGate   ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , bool ) const;
   void         buildOnOffFinalize     ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , bool ) const;
   // derive patch network
   Abc_Ntk_t  * derivePatch            ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , Vec_Ptr_t * ) const;
   Abc_Ntk_t  * initPatchModel         ( Vec_Ptr_t * ) const;
   Abc_Ntk_t  * fixPatchInputPhase     ( Abc_Ntk_t * , Abc_Ntk_t * , Vec_Ptr_t * ) const;
   // verify ECO result
   bool         verifyEcoResult        ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , Vec_Ptr_t * ) const;
   void         buildRecFCreatePio     ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * ) const;
   void         buildRecFCreateGate    ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , Vec_Ptr_t * ) const;
   void         buildRecFFinalize      ( Abc_Ntk_t * , Abc_Ntk_t * ) const;
   // write output files
   void         writePatch             ( char * );
   void         writeRectF             ( char * , bool );
   // data members
   Abc_Ntk_t       * _pNtkF;        // old implementation
   Abc_Ntk_t       * _pNtkG;        // new specification
   Abc_Ntk_t       * _pNtkP;        // patch circuit
   Vec_Int_t       * _weight;       // gate Id --> weight
   sat_solver      * _pSat;         // clauses of the checking miter
   vec<Var>          _miterVarToCandVar; 
   vec<Var>          _candVarToMiterVar;
   Vec_Int_t       * _vAssume;      // assumption variables: gate Id --> var. (!a + w = w')
   Vec_Ptr_t       * _vVarToNode;   // assumption variables -> pObj
   Vec_Int_t       * _vNodeMapPo;   // gate Id --> index of Po
   string            _nameF;        // old ntk file name
};

#endif
