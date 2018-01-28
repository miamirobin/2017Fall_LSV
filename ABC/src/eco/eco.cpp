/**CFile****************************************************************

  FileName    [eco.cpp]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [eco: resource-aware patch generation.]

  Synopsis    [Cadcontest 2017 problem A.]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [May. 22, 2017.]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "eco/eco.h"
#include "eco/RPGenMgr.h"

////////////////////////////////////////////////////////////////////////
///                        FUNCTIONS DECLARATIONS                    ///
////////////////////////////////////////////////////////////////////////

extern "C" void Eco_Init ( Abc_Frame_t * );
extern "C" void Eco_End  ( Abc_Frame_t * );

// commands
static int  Eco_CommandRPGen         ( Abc_Frame_t * , int , char ** );
static int  Eco_CommandPatchMiter    ( Abc_Frame_t * , int , char ** );
static int  Eco_CommandWriteQbf      ( Abc_Frame_t * , int , char ** );

// helpers
static int  Eco_NtkPrepareTwoNtks    ( FILE * , Abc_Ntk_t * , char ** , int , Abc_Ntk_t ** , Abc_Ntk_t ** , int * , int * );
static void sig_handler              ( int );

////////////////////////////////////////////////////////////////////////
///                        VARIABLES DECLARATIONS                    ///
////////////////////////////////////////////////////////////////////////

RPGenMgr * rpGen;

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Signal handling function]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
sig_handler( int sig )
{
   if ( rpGen ) {
      rpGen->interrupt();
      delete rpGen;
      rpGen = NULL;
   }
   Abc_Stop();
   exit(1);
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

void
Eco_Init( Abc_Frame_t * pAbc )
{
   rpGen = NULL;
   signal( SIGINT  , sig_handler );
   signal( SIGTERM , sig_handler );
   Cmd_CommandAdd( pAbc , "z ECO" , "rpgen"       , Eco_CommandRPGen      , 1 );
   Cmd_CommandAdd( pAbc , "z ECO" , "patch_miter" , Eco_CommandPatchMiter , 1 );
   Cmd_CommandAdd( pAbc , "z ECO" , "write_qbf"   , Eco_CommandWriteQbf   , 0 );
}

void
Eco_End( Abc_Frame_t * pAbc )
{
   if ( rpGen ) {
      delete rpGen;
      rpGen = NULL;
   }
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Eco_CommandRPGen( Abc_Frame_t * pAbc , int argc , char ** argv )
{
   Abc_Ntk_t * pNtk , * pNtkF , * pNtkG , * pNtkRes;
   char ** pArgvNew;
   int nArgcNew , fDelete1 , fDelete2 , c;
   bool fCheck , fVerbose;

   fCheck   = 1;
   fVerbose = 0;
   pNtk     = Abc_FrameReadNtk( pAbc );

   Extra_UtilGetoptReset();
   while ( ( c = Extra_UtilGetopt( argc , argv , "cvh" ) ) != EOF )
   {
       switch ( c )
       {
       case 'c':
          fCheck ^= 1;
          break;
       case 'v':
          fVerbose ^= 1;
          break;
       case 'h':
       default:
           goto usage;
       }
   }
   pArgvNew = argv + globalUtilOptind;
   nArgcNew = argc - globalUtilOptind;
   if ( !Eco_NtkPrepareTwoNtks( stdout , pNtk , pArgvNew , nArgcNew , &pNtkF , &pNtkG , &fDelete1 , &fDelete2 ) )
       return 1;
   // pArgvNew[0]: F.v , pArgvNew[1]: G.v , pArgvNew[2]: weight.txt , pArgvNew[3]: patch.v , pArgvNew[4]: out.v
   rpGen   = new RPGenMgr( pNtkF , pNtkG , pArgvNew[0] , pArgvNew[2] );
   pNtkRes = rpGen->genPatch(fVerbose);
   if ( pNtkRes ) {
      rpGen->writeOutput( pArgvNew[3] , pArgvNew[4] , fCheck );
      Abc_FrameReplaceCurrentNetwork( pAbc , pNtkRes );
   }
   else Abc_Print( -1 , "Patch generation has failed.\n" );
   
   delete rpGen;
   rpGen = NULL;
   return 0;

usage:
   Abc_Print( -2 , "usage: rpgen [-cvh] <F.v> <G.v> <weight.txt> <patch.v> <out.v>\n" );
   Abc_Print( -2 , "\t        resource-aware patch generation\n" );
   Abc_Print( -2 , "\tF.v        : old implementation\n");
   Abc_Print( -2 , "\tG.v        : new specification\n");
   Abc_Print( -2 , "\tweight.txt : resource of gates\n");
   Abc_Print( -2 , "\tpatch.v    : generated patch\n");
   Abc_Print( -2 , "\tout.v      : rectified implementation\n");
   Abc_Print( -2 , "\t-c         : toggles writing out.v with patch.v for cec [default = %s]\n" , fCheck ? "yes" : "no" );
   Abc_Print( -2 , "\t-v         : toggles printing verbose INFO [default = %s]\n" , fVerbose ? "yes" : "no" );
   Abc_Print( -2 , "\t-h         : prints the command usage\n");
   return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Eco_CommandPatchMiter( Abc_Frame_t * pAbc , int argc , char ** argv )
{
   Abc_Ntk_t * pNtk , * pNtkF , * pNtkG , * pNtkRes;
   int fDelete1 , fDelete2;
   char ** pArgvNew;
   int nArgcNew , c;

   pNtk = Abc_FrameReadNtk( pAbc );

   Extra_UtilGetoptReset();
   while ( ( c = Extra_UtilGetopt( argc , argv , "h" ) ) != EOF )
   {
       switch ( c )
       {
       case 'h':
       default:
           goto usage;
       }
   }
   pArgvNew = argv + globalUtilOptind;
   nArgcNew = argc - globalUtilOptind;
   if ( !Eco_NtkPrepareTwoNtks( stdout , pNtk , pArgvNew , nArgcNew , &pNtkF , &pNtkG , &fDelete1 , &fDelete2 ) )
       return 1;
   pNtkRes = Eco_BuildPatchMiter( pNtkF , pNtkG );
   Abc_NtkDelete( pNtkF );
   Abc_NtkDelete( pNtkG );
   if ( pNtkRes ) Abc_FrameReplaceCurrentNetwork( pAbc , pNtkRes );
   return 0;

usage:
   Abc_Print( -2 , "usage: patch_miter [-h] <file1> <file2>\n" );
   Abc_Print( -2 , "\t        build a miter for patch generation\n" );
   Abc_Print( -2 , "\tfile1    : old implementation (F.v)\n");
   Abc_Print( -2 , "\tfile2    : new specification  (G.v)\n");
   Abc_Print( -2 , "\t-h       : print the command usage\n");
   return 1;
}

/**Function*************************************************************

  Synopsis    [Read in two networks but DO NOT strash them.]

  Description []
               
  SideEffects []

  SeeAlso     [Abc_NtkPrepareTwoNtks()]

***********************************************************************/

int 
Eco_NtkPrepareTwoNtks( FILE * pErr , Abc_Ntk_t * pNtk , char ** argv , int argc , 
                       Abc_Ntk_t ** ppNtk1 , Abc_Ntk_t ** ppNtk2 , int * pfDelete1 , int * pfDelete2 )
{
    int fCheck = 1;
    FILE * pFile;
    Abc_Ntk_t * pNtk1, * pNtk2;
    int util_optind = 0;

    *pfDelete1 = 0;
    *pfDelete2 = 0;
    if ( argc == util_optind ) 
    { // use the spec
        if ( pNtk == NULL )
        {
            fprintf( pErr, "Empty current network.\n" );
            return 0;
        }
        if ( pNtk->pSpec == NULL )
        {
            fprintf( pErr, "The external spec is not given.\n" );
            return 0;
        }
        pFile = fopen( pNtk->pSpec, "r" );
        if ( pFile == NULL )
        {
            fprintf( pErr, "Cannot open the external spec file \"%s\".\n", pNtk->pSpec );
            return 0;
        }
        else
            fclose( pFile );
        pNtk1 = Abc_NtkDup(pNtk);
        pNtk2 = Io_Read( pNtk->pSpec, Io_ReadFileType(pNtk->pSpec), fCheck, 0 );
        if ( pNtk2 == NULL )
            return 0;
        *pfDelete1 = 1;
        *pfDelete2 = 1;
    }
    else if ( argc == util_optind + 1 ) 
    {
        if ( pNtk == NULL )
        {
            fprintf( pErr, "Empty current network.\n" );
            return 0;
        }
        pNtk1 = Abc_NtkDup(pNtk);
        pNtk2 = Io_Read( argv[util_optind], Io_ReadFileType(argv[util_optind]), fCheck, 0 );
        if ( pNtk2 == NULL )
            return 0;
        *pfDelete1 = 1;
        *pfDelete2 = 1;
    }
    else if ( argc >= util_optind + 2 ) 
    {
        pNtk1 = Io_Read( argv[util_optind], Io_ReadFileType(argv[util_optind]), fCheck, 0 );
        if ( pNtk1 == NULL )
            return 0;
        pNtk2 = Io_Read( argv[util_optind+1], Io_ReadFileType(argv[util_optind+1]), fCheck, 0 );
        if ( pNtk2 == NULL )
        {
            Abc_NtkDelete( pNtk1 );
            return 0;
        }
        *pfDelete1 = 1;
        *pfDelete2 = 1;
    }
    else
    {
        fprintf( pErr, "Wrong number of arguments.\n" );
        return 0;
    }

    *ppNtk1 = pNtk1;
    *ppNtk2 = pNtk2;
    return 1;
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Eco_CommandWriteQbf( Abc_Frame_t * pAbc , int argc , char ** argv )
{
   Abc_Ntk_t * pNtk;
   char * pFileName;
   int fNeg , numPi , c;

   pNtk  = Abc_FrameReadNtk( pAbc );
   fNeg  = 0;
   numPi = 0;

   Extra_UtilGetoptReset();
   while ( ( c = Extra_UtilGetopt( argc , argv , "Inh" ) ) != EOF )
   {
       switch ( c )
       {
       case 'I':
          if ( globalUtilOptind >= argc ) {
             Abc_Print( -1 , "Command line switch \"-I\" should be followed by an integer.\n" );
             goto usage;
          }
          numPi = atoi(argv[globalUtilOptind]);
          globalUtilOptind++;
          break;
       case 'n':
          fNeg ^= 1;
          break;
       case 'h':
       default:
           goto usage;
       }
   }
   if ( !pNtk ) {
      Abc_Print( -1 , "Empty network.\n" );
      return 1;
   }
   if ( numPi < 1 ) {
      Abc_Print( -1 , "numPi should be greater than 0! (%d is invalid).\n" , numPi );
      return 1;
   }
   if ( argc != globalUtilOptind + 1 ) goto usage;
   pFileName = argv[globalUtilOptind];
   Eco_WriteGenPatchQbf( pNtk , fNeg , numPi , pFileName );
   printf( "  > [INFO] QDIMACS file %s has been written.\n" , pFileName );
   return 0;

usage:
   Abc_Print( -2 , "usage: write_qbf [-nh] <-I num> <file>\n" );
   Abc_Print( -2 , "\t        write a QDIMACS file for patch generation\n" );
   Abc_Print( -2 , "\t-n       : toggle writing negated formula [default = %s]\n", fNeg ? "yes" : "no" );
   Abc_Print( -2 , "\t-h       : print the command usage\n");
   Abc_Print( -2 , "\t-I num   : the number of Pi\n");
   Abc_Print( -2 , "\tfile     : the name of the file to write\n");
   return 1;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
