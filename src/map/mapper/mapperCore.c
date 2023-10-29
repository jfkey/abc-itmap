/**CFile****************************************************************

  FileName    [mapperCore.c]

  PackageName [MVSIS 1.3: Multi-valued logic synthesis system.]

  Synopsis    [Generic technology mapping engine.]

  Author      [MVSIS Group]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - June 1, 2004.]

  Revision    [$Id: mapperCore.c,v 1.7 2004/10/01 23:41:04 satrajit Exp $]

***********************************************************************/

#include "mapperInt.h"
#include "map/scl/sclLib.h"
//#include "resm.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Performs technology mapping for the given object graph.]

  Description [The object graph is stored in the mapping manager.
  First, the AND nodes that fanout into POs are collected in the DFS order.
  Two preprocessing steps are performed: the k-feasible cuts are computed 
  for each node and the truth tables are computed for each cut. Next, the 
  delay-optimal matches are assigned for each node, followed by several 
  iterations of area recoveryd: using area flow (global optimization) 
  and using exact area at a node (local optimization).]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Map_Mapping( Map_Man_t * p )
{
    int fShowSwitching         = 0;
    int fUseAreaFlow           = 1;
    int fUseExactArea          = !p->fSwitching;
    int fUseExactAreaWithPhase = !p->fSwitching;
    abctime clk;

    //////////////////////////////////////////////////////////////////////
    // perform pre-mapping computations
    if ( p->fVerbose )
        Map_MappingReportChoices( p ); 
    Map_MappingSetChoiceLevels( p ); // should always be called before mapping!
//    return 1;

    // compute the cuts of nodes in the DFS order
    clk = Abc_Clock();
    Map_MappingCuts( p );
    p->timeCuts = Abc_Clock() - clk;
    // derive the truth tables 
    clk = Abc_Clock();
    Map_MappingTruths( p );
    p->timeTruth = Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////
//ABC_PRT( "Truths", Abc_Clock() - clk );

    //////////////////////////////////////////////////////////////////////
    // compute the minimum-delay mapping
    clk = Abc_Clock();
    p->fMappingMode = 0;
    if ( !Map_MappingMatches( p ) )
        return 0;
    p->timeMatch = Abc_Clock() - clk;
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaBase = Map_MappingGetArea( p );
if ( p->fVerbose )
{
printf( "Delay    : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ", 
                    fShowSwitching? "Switch" : "Delay", 
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo, 
                    Map_MappingGetAreaFlow(p), p->AreaBase, 0.0 );
ABC_PRT( "Time", p->timeMatch );
}
    //////////////////////////////////////////////////////////////////////

    if ( !p->fAreaRecovery )
    {
        if ( p->fVerbose )
            Map_MappingPrintOutputArrivals( p );
        return 1;
    }


    //////////////////////////////////////////////////////////////////////
    // perform area recovery using area flow
    clk = Abc_Clock();
    if ( fUseAreaFlow )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area flow
        p->fMappingMode = 1;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
if ( p->fVerbose )
{
printf( "AreaFlow : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ", 
                    fShowSwitching? "Switch" : "Delay", 
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo, 
                    Map_MappingGetAreaFlow(p), p->AreaFinal, 
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
ABC_PRT( "Time", Abc_Clock() - clk );
}
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    clk = Abc_Clock();
    if ( fUseExactArea )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 2;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
if ( p->fVerbose )
{
printf( "Area     : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ", 
                    fShowSwitching? "Switch" : "Delay", 
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo, 
                    0.0, p->AreaFinal, 
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
ABC_PRT( "Time", Abc_Clock() - clk );
}
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    clk = Abc_Clock();
    if ( fUseExactAreaWithPhase )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 3;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
if ( p->fVerbose )
{
printf( "Area     : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ", 
                    fShowSwitching? "Switch" : "Delay", 
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo, 
                    0.0, p->AreaFinal, 
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
ABC_PRT( "Time", Abc_Clock() - clk );
}
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    clk = Abc_Clock();
    if ( p->fSwitching )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover switching activity
        p->fMappingMode = 4;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
if ( p->fVerbose )
{
printf( "Switching: %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ", 
                    fShowSwitching? "Switch" : "Delay", 
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo, 
                    0.0, p->AreaFinal, 
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
ABC_PRT( "Time", Abc_Clock() - clk );
}

        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover switching activity
        p->fMappingMode = 4;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
if ( p->fVerbose )
{
printf( "Switching: %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ", 
                    fShowSwitching? "Switch" : "Delay", 
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo, 
                    0.0, p->AreaFinal, 
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
ABC_PRT( "Time", Abc_Clock() - clk );
}
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

    // print the arrival times of the latest outputs
    if ( p->fVerbose )
        Map_MappingPrintOutputArrivals( p );
    return 1;
}

// pNtkNew = Abc_NtkFromMap( pMan, pNtk, fUseBuffs || (DelayTarget == (double)ABC_INFINITY) );
//     if ( Mio_LibraryHasProfile(pLib) )
//         Mio_LibraryTransferProfile2( (Mio_Library_t *)Abc_FrameReadLibGen(), pLib );
//     Map_ManFree( pMan );
//     if ( pNtkNew == NULL )
//         return NULL;

//     if ( pNtk->pExdc )
//         pNtkNew->pExdc = Abc_NtkDup( pNtk->pExdc );
// if ( fVerbose )
// {
// ABC_PRT( "Total runtime", Abc_Clock() - clkTotal );
// }

//     // make sure that everything is okay
//     if ( !Abc_NtkCheck( pNtkNew ) )
//     {
//         printf( "Abc_NtkMap: The network check has failed.\n" );
//         Abc_NtkDelete( pNtkNew );
//         return NULL;
//     }

void Map_MappingIteratable() 
{

}

/**Function*************************************************************

  Synopsis    [Performs technology mapping for the given object graph.]

  Description [The object graph is stored in the mapping manager.
  First, the AND nodes that fanout into POs are collected in the DFS order.
  Two preprocessing steps are performed: the k-feasible cuts are computed
  for each node and the truth tables are computed for each cut. Next, the
  delay-optimal matches are assigned for each node, followed by several
  iterations of area recoveryd: using area flow (global optimization)
  and using exact area at a node (local optimization).]

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Map_MappingSTA( Map_Man_t * p, Abc_Ntk_t *pNtk, Mio_Library_t *pLib, int fStime,  double DelayTarget, int fUseBuffs)
{
    int fShowSwitching         = 0;
    int fUseAreaFlow           = 1;
    int fUseExactArea          = !p->fSwitching;
    int fUseExactAreaWithPhase = !p->fSwitching;
    abctime clk;

    //////////////////////////////////////////////////////////////////////
    // perform pre-mapping computations
    if ( p->fVerbose )
        Map_MappingReportChoices( p );
    Map_MappingSetChoiceLevels( p ); // should always be called before mapping!
//    return 1;

    // compute the cuts of nodes in the DFS order
    clk = Abc_Clock();
    Map_MappingCuts( p );
    p->timeCuts = Abc_Clock() - clk;
    // derive the truth tables
    clk = Abc_Clock();
    Map_MappingTruths( p );
    p->timeTruth = Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////
//ABC_PRT( "Truths", Abc_Clock() - clk );

    //////////////////////////////////////////////////////////////////////
    // compute the minimum-delay mapping
    clk = Abc_Clock();
    p->fMappingMode = 0;
    if ( !Map_MappingMatches( p ) )
        return 0;
    p->timeMatch = Abc_Clock() - clk;
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaBase = Map_MappingGetArea( p );
    if ( p->fVerbose )
    {
        printf( "Delay    : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
                fShowSwitching? "Switch" : "Delay",
                fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
                Map_MappingGetAreaFlow(p), p->AreaBase, 0.0 );
        ABC_PRT( "Time", p->timeMatch );
    }
    //////////////////////////////////////////////////////////////////////

    if ( !p->fAreaRecovery )
    {
        if ( p->fVerbose )
            Map_MappingPrintOutputArrivals( p );
        return 1;
    }

    
    /* perform STA,  update the load-dependent delay for the cut
        1. construct the mapped network. (also store the mapped network)
        2. perform topo 
        3. perfrom STA  
        -> update the delay of  Match_t, Cut_t, or Supergate? 
    */

    // 1. construct the mapped network, and store the mapped ID in Abc_obj_t
    extern Abc_Ntk_t *  Abc_NtkFromMap( Map_Man_t * pMan, Abc_Ntk_t * pNtk, int fUseBuffs );
    Abc_Ntk_t* pNtkMapped = Abc_NtkFromMap(p, pNtk, fUseBuffs || (DelayTarget == (double)ABC_INFINITY) );
    if ( Mio_LibraryHasProfile(pLib) )
            Mio_LibraryTransferProfile2( (Mio_Library_t *)Abc_FrameReadLibGen(), pLib );
    Map_ManFree( p );
    if ( pNtkMapped == NULL )
        return NULL;

    if ( pNtk->pExdc )
        pNtkMapped->pExdc = Abc_NtkDup( pNtk->pExdc );
    // make sure that everything is okay
    if ( !Abc_NtkCheck( pNtkMapped ) )
    {
        printf( "Abc_NtkMap: The network check has failed.\n" );
        Abc_NtkDelete( pNtkMapped );
        return NULL;
    }
    /* debug
    // print the mapped_network
    printf("\nReturn mapped Ntk...\n"); 
    Abc_Obj_t * pNode;
    int i = 0; 
    Abc_NtkForEachObj(pNtkMapped, pNode, i ) {
        if (i < 2950) continue;
        if(Abc_ObjIsCi(pNode)){
            printf("node(%d), index(%d), CI\n", Abc_ObjId(pNode), i);
        }
        if (Abc_ObjIsNode(pNode)){
            printf("node(%d), index(%d), mapNtkID(%d), mapNtkPhase(%d), Node\n", Abc_ObjId(pNode) , i, Abc_ObjMapNtkId(pNode),
                    Abc_ObjMapNtkPhase(pNode));

        }
        if (Abc_ObjIsCo(pNode)){
            printf("node(%d), index(%d), CO\n", Abc_ObjId(pNode), i);
        }
    }
    */

    // 2. execute topo command 
    if ( pNtkMapped == NULL )
    {
        Abc_Print( -1, "Empty network.\n" );
        return 1;
    }
    if ( !Abc_NtkIsLogic(pNtkMapped) )
    {
        Abc_Print( -1, "This command can only be applied to a logic network.\n" );
        return 1;
    }
    // modify the current network
    Abc_Ntk_t* pNtkTopoed  = Abc_NtkDupDfs( pNtkMapped );
    if ( pNtkTopoed == NULL )
    {
        Abc_Print( -1, "The command has failed.\n" );
        return 1;
    } 
    // replace the current network
    // Abc_FrameReplaceCurrentNetwork( pAbc, pNtkRes );
    /*
    // debug
    printf("\nReturn the topological ordered Ntk...\n"); 
    Abc_NtkForEachObj(pNtkTopoed, pNode, i ) {
        if (i < 2950) continue;
        if(Abc_ObjIsCi(pNode)){
            printf("node(%d), index(%d), CI\n", Abc_ObjId(pNode), i);
        }
        if (Abc_ObjIsNode(pNode)){
            printf("node(%d), index(%d), mapNtkID(%d), mapNtkPhase(%d), Node\n", Abc_ObjId(pNode) , i, Abc_ObjMapNtkId(pNode),
                    Abc_ObjMapNtkPhase(pNode));
        }
        if (Abc_ObjIsCo(pNode)){
            printf("node(%d), index(%d), CO\n", Abc_ObjId(pNode), i);
        }
    }
    */
   
    // 3. perform STA 
    int fShowAll      = 1;
    int fUseWireLoads = 0;
    int fPrintPath    = 0;
    int fDumpStats    = 0;
    int nTreeCRatio   = 0; 
    if ( !Abc_NtkHasMapping(pNtkTopoed) )
    {
        Abc_Print(-1, "The current network is not mapped.\n" );
        return 1;
    }
    if ( !Abc_SclCheckNtk(pNtkTopoed, 0) )
    {
        Abc_Print(-1, "The current network is not in a topo order (run \"topo\").\n" );
        return 1;
    }
    if ( Abc_FrameReadLibScl() == NULL )
    {
        Abc_Print(-1, "There is no Liberty library available.\n" );
        return 1;
    }
    extern void Abc_SclTimePerform( SC_Lib * pLib, Abc_Ntk_t * pNtk, int nTreeCRatio, int fUseWireLoads, int fShowAll, int fPrintPath, int fDumpStats );
    Abc_SclTimePerform( Abc_FrameReadLibScl(), pNtkTopoed, nTreeCRatio, fUseWireLoads, fShowAll, fPrintPath, fDumpStats );

/*
    //////////////////////////////////////////////////////////////////////
    // perform area recovery using area flow
    clk = Abc_Clock();
    if ( fUseAreaFlow )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area flow
        p->fMappingMode = 1;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
        if ( p->fVerbose )
        {
            printf( "AreaFlow : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
                    fShowSwitching? "Switch" : "Delay",
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
                    Map_MappingGetAreaFlow(p), p->AreaFinal,
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
            ABC_PRT( "Time", Abc_Clock() - clk );
        }
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    clk = Abc_Clock();
    if ( fUseExactArea )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 2;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
        if ( p->fVerbose )
        {
            printf( "Area     : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
                    fShowSwitching? "Switch" : "Delay",
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
                    0.0, p->AreaFinal,
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
            ABC_PRT( "Time", Abc_Clock() - clk );
        }
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    clk = Abc_Clock();
    if ( fUseExactAreaWithPhase )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 3;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
        if ( p->fVerbose )
        {
            printf( "Area     : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
                    fShowSwitching? "Switch" : "Delay",
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
                    0.0, p->AreaFinal,
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
            ABC_PRT( "Time", Abc_Clock() - clk );
        }
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    clk = Abc_Clock();
    if ( p->fSwitching )
    {
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover switching activity
        p->fMappingMode = 4;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
        if ( p->fVerbose )
        {
            printf( "Switching: %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
                    fShowSwitching? "Switch" : "Delay",
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
                    0.0, p->AreaFinal,
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
            ABC_PRT( "Time", Abc_Clock() - clk );
        }

        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover switching activity
        p->fMappingMode = 4;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
        if ( p->fVerbose )
        {
            printf( "Switching: %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
                    fShowSwitching? "Switch" : "Delay",
                    fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
                    0.0, p->AreaFinal,
                    100.0*(p->AreaBase-p->AreaFinal)/p->AreaBase );
            ABC_PRT( "Time", Abc_Clock() - clk );
        }
    }
    p->timeArea += Abc_Clock() - clk;
    //////////////////////////////////////////////////////////////////////

*/

    // print the arrival times of the latest outputs
    if ( p->fVerbose )
        Map_MappingPrintOutputArrivals( p );
    return 1;
}

ABC_NAMESPACE_IMPL_END

