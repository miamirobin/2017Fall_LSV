/**CFile****************************************************************

  FileName    [ecoWriteQbf.cpp]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [eco: resource-aware patch generation.]

  Synopsis    [Write qdimacs for patch generation.]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [May. 22, 2017.]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "eco/eco.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// external functions
extern "C" {
   void Abc_NtkShow             ( Abc_Ntk_t * , int , int , int );
   void Abc_NtkMiterAddOne      ( Abc_Ntk_t * , Abc_Ntk_t * );
   void Abc_NtkMiterFinalize    ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , int , int , int , int );
}
// main functions
Abc_Ntk_t *        Eco_BuildPatchMiter     ( Abc_Ntk_t * , Abc_Ntk_t * );
void               Eco_WriteGenPatchQbf    ( Abc_Ntk_t * , int , int , char * );
// helpers
static Abc_Ntk_t * Eco_NtkBuildMiter       ( Abc_Ntk_t * , Abc_Ntk_t * , int );
static void        Eco_NtkMiterCreatePio   ( Abc_Ntk_t * , Abc_Ntk_t * , Abc_Ntk_t * , int );
static void        Eco_WriteQdimacsPrefix  ( FILE * , Abc_Ntk_t * , int , int );
static void        Eco_WriteClause         ( FILE * , Abc_Ntk_t * , int );
   
////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Write out a QBF formula for patch generation.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

Abc_Ntk_t*
Eco_BuildPatchMiter( Abc_Ntk_t * pNtkF , Abc_Ntk_t * pNtkG )
{
   Abc_Ntk_t * pNtkMiter;
   Abc_Obj_t * pObj , * pFanin , * pObjNew;
   int nTarget = 0 , i , j;
   // step.1: Create additional Pis
   Abc_NtkForEachNode( pNtkF , pObj , i )
   {
      Abc_ObjForEachFanin( pObj , pFanin , j )
      {
         if ( strncmp( Abc_ObjName(pFanin) , "t_num" , 2 ) == 0 ) {
            printf( "  > [INFO] Replace targer point %s by Pi at node %s\n" , Abc_ObjName(pFanin) , Abc_ObjName(pObj) );
            pObjNew = Abc_NtkCreatePi( pNtkF );
            Abc_ObjAssignName( pObjNew , Abc_ObjName(pFanin) , "" );
            Abc_ObjPatchFanin( pObj , pFanin , pObjNew );
            ++nTarget;
         }
      }
   }
   assert( nTarget == Abc_NtkPiNum(pNtkF)-Abc_NtkPiNum(pNtkG) );
   printf( "  > [INFO] Number of targets = %d\n" , nTarget );
   // step.2: Build miter
   pNtkMiter = Eco_NtkBuildMiter( pNtkF , pNtkG , nTarget );
   if ( !pNtkMiter ) {
      Abc_Print( -1 , "Miter computation failed ...\n" );
      return NULL;
   }
   //Abc_NtkShow( pNtkMiter , 0 , 0 , 1 );
   return pNtkMiter;
}

/**Function*************************************************************

  Synopsis    [Build a miter considering target points.]

  Description [Cannot use Abc_NtkMiter() since Pi mismatch.]
               
  SideEffects []

  SeeAlso     [Abc_NtkMiter()]

***********************************************************************/

Abc_Ntk_t*
Eco_NtkBuildMiter( Abc_Ntk_t * pNtkF , Abc_Ntk_t * pNtkG , int nTarget )
{
   char Buffer[1000];
   Abc_Ntk_t * pNtk1 , * pNtk2 , * pNtkMiter;

   pNtk1     = Abc_NtkStrash( pNtkF , 0 , 1 , 0 );
   pNtk2     = Abc_NtkStrash( pNtkG , 0 , 1 , 0 );
   if ( !pNtk1 || !pNtk2 ) {
      if ( pNtk1 ) Abc_NtkDelete( pNtk1 );
      if ( pNtk2 ) Abc_NtkDelete( pNtk2 );
      return NULL;
   }
   pNtkMiter = Abc_NtkAlloc( ABC_NTK_STRASH , ABC_FUNC_AIG , 1 );
   sprintf( Buffer , "%s_%s_miter" , pNtk1->pName , pNtk2->pName );
   pNtkMiter->pName = Extra_UtilStrsav(Buffer);
   Eco_NtkMiterCreatePio( pNtk1 , pNtk2 , pNtkMiter , nTarget );
   Abc_NtkMiterAddOne( pNtk1 , pNtkMiter );
   Abc_NtkMiterAddOne( pNtk2 , pNtkMiter );
   Abc_NtkMiterFinalize( pNtk1 , pNtk2 , pNtkMiter , 0 , 0 , 0 , 0 );
   // negate the miter to assert EQUIVALENCE for qdimacs generation
   //Abc_ObjXorFaninC( Abc_NtkPo( pNtkMiter , 0 ) , 0 );
   Abc_AigCleanup( (Abc_Aig_t *)pNtkMiter->pManFunc );
   Abc_NtkDelete( pNtk1 );
   Abc_NtkDelete( pNtk2 );
   if ( !Abc_NtkCheck( pNtkMiter ) ) {
      printf( "Abc_NtkMiter: The network check has failed.\n" );
      Abc_NtkDelete( pNtkMiter );
      return NULL;
   }
   return pNtkMiter;
}

void
Eco_NtkMiterCreatePio( Abc_Ntk_t * pNtk1 , Abc_Ntk_t * pNtk2 , Abc_Ntk_t * pNtkMiter , int nTarget )
{
   Abc_Obj_t * pObj , * pObjNew;
   int i;
   Abc_AigConst1(pNtk1)->pCopy = Abc_AigConst1(pNtkMiter);
   Abc_AigConst1(pNtk2)->pCopy = Abc_AigConst1(pNtkMiter);

   // create Pis
   Abc_NtkForEachPi( pNtk2 , pObj , i )
   {
      pObjNew = Abc_NtkCreatePi( pNtkMiter );
      Abc_ObjAssignName( pObjNew , Abc_ObjName(pObj) , NULL );
      pObj->pCopy = pObjNew;
      pObj = Abc_NtkPi( pNtk1 , i );
      pObj->pCopy = pObjNew;
   }
   // handle target points
   for ( int i = 0 ; i < nTarget ; ++i ) {
      pObj    = Abc_NtkPi( pNtk1 , Abc_NtkPiNum(pNtk2)+i );
      pObjNew = Abc_NtkCreatePi( pNtkMiter );
      Abc_ObjAssignName( pObjNew , Abc_ObjName(pObj) , NULL );
      pObj->pCopy = pObjNew;
   }
   pObjNew = Abc_NtkCreatePo( pNtkMiter );
   Abc_ObjAssignName( pObjNew , "miter" , NULL );
}

/**Function*************************************************************

  Synopsis    [Write out patch generation QBF files.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Eco_WriteGenPatchQbf( Abc_Ntk_t * pNtkMiter , int fNeg , int numPi , char * name )
{
   FILE * out;
	int  numVar , numCla;

	out    = fopen( name , "w" );
	numVar = Abc_NtkObjNum( pNtkMiter ) - 1;          // substract constant node
	numCla = 3 * Abc_NtkNodeNum( pNtkMiter ) + 2 + 1; // 2 for Po connection , 1 for Po assertion
	fprintf( out , "c QDIMACS file for patch generation written by Nian-Ze Lee (d04943019@ntu.edu.tw)\n" );
   fprintf( out , "p cnf %d %d\n" , numVar , numCla );
	Eco_WriteQdimacsPrefix( out , pNtkMiter , fNeg , numPi ); 
	Eco_WriteClause( out , pNtkMiter , fNeg );
   fclose( out );
}

void 
Eco_WriteQdimacsPrefix( FILE * out , Abc_Ntk_t * pNtkMiter , int fNeg , int numPi )
{
   Abc_Obj_t * pObj;
	int i;
   if ( fNeg ) {
	   Abc_NtkForEachPi( pNtkMiter , pObj , i ) 
	      fprintf( out , "%s %d 0\n" , (i < numPi) ? "e" : "a" , Abc_ObjId(pObj) );
   }
   else {
	   Abc_NtkForEachPi( pNtkMiter , pObj , i ) 
	      fprintf( out , "%s %d 0\n" , (i < numPi) ? "a" : "e" , Abc_ObjId(pObj) );
   }
	Abc_NtkForEachPo( pNtkMiter , pObj , i )
	   fprintf( out , "e %d 0\n" , Abc_ObjId(pObj) );
	Abc_NtkForEachNode( pNtkMiter , pObj , i )
	   fprintf( out , "e %d 0\n" , Abc_ObjId(pObj) );
}

void
Eco_WriteClause( FILE * out , Abc_Ntk_t * pNtk , int fNeg )
{
	Abc_Obj_t * pObj , * pFanin0 , * pFanin1;
	int i;
	Abc_NtkForEachNode( pNtk , pObj , i )
	{
		pFanin0 = Abc_ObjFanin0( pObj );
		pFanin1 = Abc_ObjFanin1( pObj );
		if ( !Abc_ObjFaninC0(pObj) && !Abc_ObjFaninC1(pObj) ) {
			fprintf( out , "%d -%d -%d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) , Abc_ObjId(pFanin1) );
			fprintf( out , "-%d %d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) );
			fprintf( out , "-%d %d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin1) );
		}
		if ( Abc_ObjFaninC0(pObj) && !Abc_ObjFaninC1(pObj) ) {
			fprintf( out , "%d %d -%d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) , Abc_ObjId(pFanin1) );
			fprintf( out , "-%d -%d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) );
			fprintf( out , "-%d %d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin1) );
		}
		if ( !Abc_ObjFaninC0(pObj) && Abc_ObjFaninC1(pObj) ) {
			fprintf( out , "%d -%d %d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) , Abc_ObjId(pFanin1) );
			fprintf( out , "-%d %d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) );
			fprintf( out , "-%d -%d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin1) );
		}
		if ( Abc_ObjFaninC0(pObj) && Abc_ObjFaninC1(pObj) ) {
			fprintf( out , "%d %d %d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) , Abc_ObjId(pFanin1) );
			fprintf( out , "-%d -%d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin0) );
			fprintf( out , "-%d -%d 0\n" , Abc_ObjId(pObj) , Abc_ObjId(pFanin1) );
		}
	}
	pObj    = Abc_NtkPo( pNtk , 0 );
	pFanin0 = Abc_ObjFanin0( pObj );
	if ( !Abc_ObjFaninC0( pObj ) ) {
	   fprintf( out , "%d -%d 0\n" , Abc_ObjId( pObj ) , Abc_ObjId( pFanin0 ) );
	   fprintf( out , "-%d %d 0\n" , Abc_ObjId( pObj ) , Abc_ObjId( pFanin0 ) );
	}
	else {
	   fprintf( out , "%d %d 0\n"   , Abc_ObjId( pObj ) , Abc_ObjId( pFanin0 ) );
	   fprintf( out , "-%d -%d 0\n" , Abc_ObjId( pObj ) , Abc_ObjId( pFanin0 ) );
	}
   // assert Po according to fNeg
	fprintf( out , "%s%d 0\n" , fNeg ? "" : "-" , Abc_ObjId( pObj ) );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
