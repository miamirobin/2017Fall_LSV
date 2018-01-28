/**CFile****************************************************************

  FileName    [lsvCmd.cpp]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [lsv: Logic Synthesis and Verification PA.]

  Synopsis    [command file.]

  Author      [Nian-Ze Lee]
  
  Affiliation [NTU]

  Date        [17, Sep., 2017.]

***********************************************************************/

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "base/main/mainInt.h"

ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

// export to mainInit.c
extern "C" void Lsv_Init ( Abc_Frame_t * );
extern "C" void Lsv_End  ( Abc_Frame_t * );

// command functions
static int Abc_Command1SubFind( Abc_Frame_t * , int , char ** );

// external functions defined in lsv package
extern void Lsv_Ntk1SubFind( Abc_Ntk_t * );

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
Lsv_Init( Abc_Frame_t * pAbc )
{
   Cmd_CommandAdd( pAbc, "z LSV", "1subfind" , Abc_Command1SubFind , 0 );
}

void
Lsv_End( Abc_Frame_t * pAbc )
{
}

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/

int
Abc_Command1SubFind( Abc_Frame_t * pAbc , int argc , char ** argv )
{
   // TODO:
   // step.1: get the current network
   // step.2: check whether the current network is strashed
   // step.3: call Lsv_Ntk1SubFind() to report 1SubFind
  Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
  if (!pNtk){
           Abc_Print(-1 ,"Empty network.\n");
           return 1;
  }
  if (Abc_NtkIsStrash(pNtk)){
           Lsv_Ntk1SubFind(pNtk);
  }
  else {
          pNtk=Abc_NtkStrash(pNtk,0,1,0);
          Lsv_Ntk1SubFind(pNtk);
          Abc_NtkDelete(pNtk);
  }

   return 0;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

