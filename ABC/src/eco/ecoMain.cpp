/**CFile****************************************************************

  FileName    [ecoMain.cpp]

  SystemName  [2017 CAD contest problem A]

  Synopsis    [main() function to meet contest i/o format]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [22, July, 2017]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "eco/RPGenMgr.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [main() function for CAD contest]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

#if 0
int 
main( int argc , char ** argv )
{
    Abc_Frame_t * pAbc;
    Vec_Str_t * sCommandUsr = Vec_StrAlloc(1000);
    int fStatus , i;

    if ( argc != 6 ) {
       Abc_Print( -1 , "Missing arguments ...\n" );
       return 1;
    }
    pAbc = Abc_FrameGetGlobalFrame();
    pAbc->sBinary = argv[0];
    Vec_StrAppend( sCommandUsr , "rpgen" );
    for ( i = 1 ; i < 6 ; ++i ) { 
       Vec_StrPush  ( sCommandUsr , ' ' );
       Vec_StrAppend( sCommandUsr , argv[i] );
    }
    Vec_StrPush(sCommandUsr, '\0');

    printf( "[INFO] Command line: \"%s\".\n\n" , Vec_StrArray(sCommandUsr) );
    fStatus = Cmd_CommandExecute( pAbc , Vec_StrArray(sCommandUsr) );
    
    Vec_StrFree(sCommandUsr);
    Abc_Stop();
    return fStatus;
}
#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////
