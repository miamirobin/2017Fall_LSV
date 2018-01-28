/**CFile****************************************************************

  FileName    [RPGenMgr.cpp]

  SystemName  [2017 CAD contest problem A]

  Synopsis    [Implementations of member functions for RPGenMgr]

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
   void Io_WriteVerilog( Abc_Ntk_t * , char * );
   void Eco_WriteVerilog( Abc_Ntk_t * , char * );
   void Io_WriteVerilogInt( FILE * , Abc_Ntk_t * );
}

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Constructor / Destructor]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

RPGenMgr::RPGenMgr( Abc_Ntk_t * pNtkF , Abc_Ntk_t * pNtkG , char * F , char * weight )
{
   _pNtkF      = pNtkF;
   _pNtkG      = pNtkG;
   _pNtkP      = NULL;
   _weight     = NULL;
   _pSat       = NULL;
   _vAssume    = NULL;
   _vVarToNode = NULL;
   _vNodeMapPo = NULL;
   _nameF      = F;
   readWeight( weight );
}

RPGenMgr::~RPGenMgr()
{
   if ( _pNtkF ) {
      Abc_NtkDelete( _pNtkF );
      _pNtkF = NULL;
   }
   if ( _pNtkG ) {
      Abc_NtkDelete( _pNtkG );
      _pNtkG = NULL;
   }
   // pass out to ABC
   /*if ( _pNtkP ) {
      Abc_NtkDelete( _pNtkP );
      _pNtkP = NULL;
   }*/
   if ( _weight ) {
      Vec_IntFree( _weight );
      _weight = NULL;
   }
   if ( _pSat ) {
      sat_solver_delete( _pSat );
      _pSat = NULL;
   }
   if ( _vAssume ) {
      Vec_IntFree( _vAssume );
      _vAssume = NULL;
   }
   if ( _vVarToNode ) {
      Vec_PtrFree( _vVarToNode );
      _vVarToNode = NULL;
   }
   if ( _vNodeMapPo ) {
      Vec_IntFree( _vNodeMapPo );
      _vNodeMapPo = NULL;
   }
}

/**Function*************************************************************

  Synopsis    [RPGenMgr testing interface]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
RPGenMgr::test()
{
   printf( "[INFO] Testing RPGenMgr ... \n" );
   Abc_Obj_t * pObj;
   int status , pLits[7] , i;

   Abc_NtkForEachPi( _pNtkF , pObj , i )
   {
      if ( i == Abc_NtkPiNum(_pNtkG) ) break;
      printf( "  > Pi %s , id = %d\n" , Abc_ObjName(pObj) , Abc_ObjId(pObj) );
      printf( "    > w = %d , ctrl var = %d\n" , Vec_IntEntry( _weight , Abc_ObjId(pObj) ) , Vec_IntEntry( _vAssume , Abc_ObjId(pObj) ) );
   }
   Abc_NtkForEachNode( _pNtkF , pObj , i )
   {
      if ( Vec_IntEntry( _weight , Abc_ObjId(pObj) ) == -1 ) continue;
      printf( "  > Node %s , id = %d\n" , Abc_ObjName(pObj) , Abc_ObjId(pObj) );
      printf( "    > w = %d , ctrl var = %d\n" , Vec_IntEntry( _weight , Abc_ObjId(pObj) ) , Vec_IntEntry( _vAssume , Abc_ObjId(pObj) ) );
   }
   pLits[0] = Abc_Var2Lit( Vec_IntEntry( _vAssume , 1  ) , 1 );
   pLits[1] = Abc_Var2Lit( Vec_IntEntry( _vAssume , 2  ) , 1 );
   pLits[2] = Abc_Var2Lit( Vec_IntEntry( _vAssume , 3  ) , 1 );
   pLits[3] = Abc_Var2Lit( Vec_IntEntry( _vAssume , 6  ) , 0 );
   pLits[4] = Abc_Var2Lit( Vec_IntEntry( _vAssume , 7  ) , 0 );
   pLits[5] = Abc_Var2Lit( Vec_IntEntry( _vAssume , 8  ) , 1 );
   pLits[6] = Abc_Var2Lit( Vec_IntEntry( _vAssume , 9  ) , 1 );
   status   = sat_solver_solve( _pSat , pLits , pLits+7 , 1000000 , 0 , 0 , 0 );
   printf( "status = %d\n" , status );
}

/**Function*************************************************************

  Synopsis    [Handling interruption.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
RPGenMgr::interrupt()
{
   printf( "[WARNING] interruption occurs, return current results before exiting\n" );
}

/**Function*************************************************************

  Synopsis    [Read weights of gates.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
RPGenMgr::readWeight( char * wFile )
{
   ifstream in;
   string temp;
   Abc_Obj_t * pObj;
   char name[128];
   int Id;
  
   _weight = Vec_IntStart( Abc_NtkObjNumMax( _pNtkF ) );
   Vec_IntFill( _weight , Vec_IntSize(_weight) , -1 );
   in.open( wFile , ifstream::in );
   if ( !in.is_open() ) {
      Abc_Print( -1 , "Open resource file %s has failed ...\n" , wFile );
      exit(1);
   }
   while ( in >> temp ) {
      strcpy( name , temp.c_str() );
      Id = Nm_ManFindIdByNameTwoTypes( _pNtkF->pManName , name , ABC_OBJ_NODE , ABC_OBJ_PI );
      if ( Id == -1 ) {
         Abc_Print( -1 , "Gate %s is not found ...\n" , name );
         exit(1);
      }
      pObj = Abc_NtkObj( _pNtkF , Id );
      in >> temp;
      Vec_IntWriteEntry( _weight , Abc_ObjId(pObj) , atoi(temp.c_str()) );
   }
   in.close();
}

/**Function*************************************************************

  Synopsis    [Write output files.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
RPGenMgr::writeOutput( char * patch , char * out , bool fCheck )
{
   writePatch( patch );
   writeRectF( out , fCheck );
}

void
RPGenMgr::writePatch( char * patch )
{
   Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist( _pNtkP );
   Abc_NtkToAig( pNtkTemp );
   Eco_WriteVerilog( pNtkTemp , patch );
   Abc_NtkDelete( pNtkTemp );
}

void
RPGenMgr::writeRectF( char * out , bool fCheck )
{
   ifstream inFile;
   FILE * pFile;
   string temp;
   Abc_Obj_t * pObj;
   int i;
  
   inFile.open( _nameF.c_str() , ifstream::in );
   if ( !inFile.is_open() ) {
      Abc_Print( -1 , "Open file %s has failed ...\n" , _nameF.c_str() );
      exit(1);
   }
   pFile = fopen( out , "w" );
   if ( !pFile ) {
      Abc_Print( -1 , "Open file %s has failed ...\n" , out );
      exit(1);
   }
   while ( getline( inFile , temp ) ) {
      if ( !strcmp( temp.c_str() , "endmodule" ) ) {
         fprintf( pFile , "patch p0( .t_0(t_0)" );
         Abc_NtkForEachPi( _pNtkP , pObj , i )
            fprintf( pFile , " , .%s(%s)" , Abc_ObjName(pObj) , Abc_ObjName(pObj) );
         fprintf( pFile , " );\n" );
      }
      fprintf( pFile , "%s\n" , temp.c_str() );
   }
   inFile.close();
   if ( fCheck ) {
      fprintf( pFile , "\n" );
      Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist( _pNtkP );
      Abc_NtkToAig( pNtkTemp );
      Io_WriteVerilogInt( pFile , pNtkTemp );
      Abc_NtkDelete( pNtkTemp );
   }
	fclose( pFile );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
