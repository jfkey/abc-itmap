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
#include "map/scl/sclSize.h"
#include "map/mio/mio.h"
#include "map/mio/mioInt.h"
#include <Python.h>

// #include "bayesopt/bayesopt.h" 
// #include "bayesopt/parameters.h"
 
 
ABC_NAMESPACE_IMPL_START

typedef struct {
    double* rec_x;          // parameters 
    double rec_y;           // the mapping result wrt the parameters
}  ItResults;



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
    printf("node gain among area recovery: %6d, leaves gain: %6d\n", p->nodeGainArea, p->leavesGainArea);

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

    int para_size = 10;
    p->delayParams = malloc(sizeof(double) * para_size);

    // p->delayParams[0] = 0.5;
    // p->delayParams[1] = 0.3;
    // p->delayParams[2] = 0.1;
    // p->delayParams[3] = 0.5;
    // p->delayParams[4] = 1.0; 
    // p->delayParams[5] = 0.3;
    // p->delayParams[6] = 0.1;
    // p->delayParams[7] = 0.25;
    // p->delayParams[8] = 1.0;
    // p->delayParams[9] = 0.5;

    // log2 
    // Area =    59758.87 ( 99.8 %)   Delay =  8025.91 ps
    // double para[10] = {0.773, 0.197, 0.286, 0.068, 0.872, 0.140, 0.467, 0.203, 1.963, 0.168};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }

    // Area =   223061.42 ( 99.9 %)   Delay = 46256.72 ps
    // double para[10] = {0.296, 0.314, 0.240, 0.173, 0.956, 0.487, 0.024, 0.033, 1.252, 0.143};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }

    // square   36268.97 ( 99.8 %)   Delay =  2998.03 p
    // double para[10] = {0.278, 0.142, 0.355, 0.014, 1.338, 0.190, 0.239, 0.044, 0.925, 0.979};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    // // square Area =   109383.66 (100.0 %)   Delay = 14994.08 p
    // double para[10] = {0.178, 0.078, 0.134, 0.569, 0.880, 0.170, 0.025, 0.029, 0.966, 0.609};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    // adder Area =     1565.78 ( 99.8 %)   Delay =  3360.78 ps
    // double para[10] = {0.228, 0.199, 0.431, 0.938, 0.684, 0.097, 0.368, 0.097, 0.814, 0.873};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    // // adder Area =     5106.15 ( 75.0 %)   Delay = 16130.87 ps
    // double para[10] = {0.647, 0.212, 0.146, 0.738, 1.923, 0.361, 0.412, 0.006, 1.893, 0.602};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    
    // // sin  Area =    13029.39 ( 99.5 %)   Delay =  3327.49 p
    // double para[10] = {0.415, 0.077, 0.393, 0.290, 0.971, 0.106, 0.408, 0.134, 0.808, 0.268};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    // // sin Area =    34563.15 ( 96.4 %)   Delay = 20821.15 ps
    // double para[10] = {0.657, 0.142, 0.184, 0.318, 1.515, 0.155, 0.396, 0.062, 1.375, 0.328};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    // // hyp:  Area =   452946.94 ( 99.9 %)   Delay =330997.34 ps
    // double para[10] = {0.127, 0.353, 0.206, 0.368, 1.858, 0.037, 0.366, 0.121, 1.375, 0.380};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    // // hyp: Area =  1183954.25 (100.0 %)   Delay =1995950.88 p
    // double para[10] = {0.574, 0.070, 0.189, 0.675, 1.274, 0.053, 0.254, 0.017, 0.582, 0.934};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    
    // // priority Area =     1674.02 ( 90.4 %)   Delay =  2666.61 ps
    // // para[0]=0.715, para[1]=0.300, para[2]=0.222, para[3]=0.472, para[4]=1.011, para[5]=0.467, para[6]=0.167, para[7]=0.104, para[8]=0.578, para[9]=0.446,
    // double para[10] = {0.715, 0.300, 0.222, 0.472, 1.011, 0.467, 0.167, 0.104, 0.578, 0.446};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    
    // priority: Area =     4819.62 ( 85.7 %)   Delay = 13218.51 ps
    // para[0]=0.817, para[1]=0.444, para[2]=0.400, para[3]=0.463, para[4]=1.542, para[5]=0.123, para[6]=0.233, para[7]=0.398, para[8]=1.037, para[9]=0.324,
    // double para[10] = {0.817, 0.444, 0.400, 0.463, 1.542, 0.123, 0.233, 0.398, 1.037, 0.324};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }

    // // i2c  Area =     1311.27 ( 87.0 %)   Delay =   222.50 ps
    // double para[10] = {0.805, 0.157, 0.182, 0.899, 1.061, 0.004, 0.039, 0.469, 1.020, 0.135};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    // // i2c Area =     5981.99 ( 93.5 %)   Delay =  1138.71 ps 
    // // para[0]=0.970, para[1]=0.151, para[2]=0.000, para[3]=0.904, para[4]=0.699, para[5]=0.018, para[6]=0.353, para[7]=0.313, para[8]=0.965, para[9]=0.093, 
    // double para[10] = {0.970, 0.151, 0.000, 0.904, 0.699, 0.018, 0.353, 0.313, 0.965, 0.093};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }


    // router: Area =      390.98 ( 71.2 %)   Delay =   457.93 p
    // double para[10] = {0.506, 0.394, 0.251, 0.106, 0.935, 0.371, 0.057, 0.214, 1.238, 0.138};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }


    // mem_ctrl Area =    65178.20 ( 96.1 %)   Delay =  3666.65 ps Area =    63436.06 ( 97.6 %)   Delay =  4279.48 ps
    // double para[10] = {0.735, 0.092, 0.305, 0.441, 1.218, 0.361, 0.308, 0.762, 1.918, 0.273};
    // for(int i = 0; i < para_size; i++) {
    //    p->delayParams[i] = para[i];
    // }

    
    // // dec  Area =      290.67 ( 97.3 %)   Delay =   116.79 ps  
    // double para[10] = {0.942, 0.410, 0.059, 0.363, 1.237, 0.173, 0.433, 0.268, 1.867, 0.891};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // }
    
    // ctrl  Area =      143.70 ( 93.0 %)   Delay =   116.84 ps
    // double para[10] = {0.985, 0.343, 0.265, 0.277, 0.604, 0.152, 0.365, 0.357, 1.374, 0.887};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // } 

    // int2float Area =      187.56 ( 92.9 %)   Delay =   136.19 ps
    // double para[10] = {0.988, 0.031, 0.211, 0.503, 0.702, 0.171, 0.279, 0.881, 1.176, 0.578};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // } 

 // div Area =   121888.80 ( 99.9 %)   Delay = 80877.98 ps 
     
    // double para[10] = {0.708, 0.091, 0.108, 0.312, 1.085, 0.302, 0.147, 0.080, 1.139, 0.139};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // } 

    // sqrt Area =    40643.91 ( 99.2 %)   Delay =108927.12 ps
    // double para[10] = {0.797, 0.134, 0.124, 0.607, 1.574, 0.371, 0.427, 0.281, 0.918, 0.407};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // } 

    // // max: Area =     4579.99 ( 85.7 %)   Delay =  2628.48 ps 
    // double para[10] = {0.862, 0.305, 0.032, 0.568, 1.583, 0.305, 0.239, 0.547, 1.632, 0.211};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // } 

    
    // sin Area =     4963.97 ( 80.8 %)   Delay =  4006.83 ps  
    // Area =    11589.82 ( 99.5 %)   Delay =  6872.25 ps
    // double para[10] = {0.981, 0.429, 0.187, 0.980, 0.800, 0.271, 0.435, 0.700, 1.948, 0.157};
    // for(int i = 0; i < para_size; i++) {
    //     p->delayParams[i] = para[i];
    // } 

    // sin   Area =     6569.40 ( 76.1 %)   Delay =  3219.10 ps 
    //  Area =    15373.85 ( 97.3 %)   Delay =  3636.86 ps
    // double para[10] = {0.227, 0.335, 0.076, 0.563, 1.407, 0.026, 0.334, 0.314, 0.508, 0.794};
    // for(int i = 0; i < para_size; i++) {
    //         p->delayParams[i] = para[i];
    //  } 

    // sin  Area =     5624.61 ( 77.8 %)   Delay =  2898.05 ps
    // Area =    13472.62 ( 97.6 %)   Delay =  3689.19 ps
    // double para[10] = {0.736, 0.144, 0.349, 0.458, 1.025, 0.407, 0.020, 0.889, 1.288, 0.252};
    // for(int i = 0; i < para_size; i++) {
    //         p->delayParams[i] = para[i];
    //  } 

    double goodPara[3][10] = {
        {0.736, 0.144, 0.349, 0.458, 1.025, 0.407, 0.020, 0.889, 1.288, 0.252},
        {0.5, 0.3, 0.1, 0.5, 1.0, 0.3, 0.1, 0.25, 1.0, 0.5},  // Expert Design
        {0.348, 0.061, 0.017, 0.146, 1.832, 0.411, 0.260, 0.050, 1.954, 0.782} // best result for bar
    };
    double curDelay, firstDelay, firstArea, firstLevel;
    int good_itera_num = 3;
    double objective, min_Obj = MAP_FLOAT_LARGE;
    int best_idx = 0;
    
  /////////////////////////////////////////////////////////////////////////////////////////////////////// 
    // init samples using some ``good delay parameters''
    for (int i = 0; i < good_itera_num; i++ ) {
        // update the parameters for p->delayParas
        for (int j = 0; j < para_size; j++)  p->delayParams[j] = goodPara[i][j];

        double estDepth = 0.0; 

        ////////////////////////////////////////////////////////////////////// 
        clk = Abc_Clock();
        p->fMappingMode = 0;
        if ( !Map_MappingMatches2( p, &estDepth) )
            return 0;
        p->timeMatch = Abc_Clock() - clk;
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p ); 
        ////////////////////////////////////////////////////////////////////// 

        // 1. construct the mapped network, and store the mapped ID in Abc_obj_t
        extern Abc_Ntk_t *  Abc_NtkFromMap( Map_Man_t * pMan, Abc_Ntk_t * pNtk, int fUseBuffs );
        Abc_Ntk_t* pNtkMapped = Abc_NtkFromMap(p, pNtk, fUseBuffs || (DelayTarget == (double)ABC_INFINITY) );
        if ( Mio_LibraryHasProfile(pLib) )
                Mio_LibraryTransferProfile2( (Mio_Library_t *)Abc_FrameReadLibGen(), pLib );
        // Map_ManFree( p );
        if ( pNtkMapped == NULL )
            return 1;

        if ( pNtk->pExdc )
            pNtkMapped->pExdc = Abc_NtkDup( pNtk->pExdc );
        // make sure that everything is okay
        if ( !Abc_NtkCheck( pNtkMapped ) )
        {
            printf( "Abc_NtkMap: The network check has failed.\n" );
            Abc_NtkDelete( pNtkMapped );
            return 1;
        }
         
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
            
         // 3. perform STA
        int fShowAll      = 0;
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
        printf("####    NLDM (%d)", i);
        extern void Abc_SclTimePerform( SC_Lib * pLib, Abc_Ntk_t * pNtk, int nTreeCRatio, int fUseWireLoads, int fShowAll, int fPrintPath, int fDumpStats );
        Abc_SclTimePerform( Abc_FrameReadLibScl(), pNtkTopoed, nTreeCRatio, fUseWireLoads, fShowAll, fPrintPath, fDumpStats );
        
        curDelay = pNtkTopoed ->MaxDelay;
        double estArea = Map_MappingGetArea( p );
        // double estArea = p->AreaFinal;  

        if ( i == 0) {
            firstDelay = curDelay; 
            firstArea = estArea;
            firstLevel = Abc_NtkLevel(pNtkTopoed);
        }
        // double gapDelay = Abc_AbsFloat( estDepth - curDelay)/curDelay;
        double estLevel = Abc_NtkLevel(pNtkTopoed); 
        
        // rec_y[0] = curDelay/firstDelay + estArea/firstArea + (estLevel - firstLevel) * 0.05;
        objective = curDelay/firstDelay + estArea/firstArea; 
        printf("#### Heuristic (%d) Delay = %.3f, Depth = %.3f, Level = %.1f, Objective = %.3f \n", i, curDelay, estDepth, estLevel, objective);
  
        
        if (objective < min_Obj) {
            // record better delay parameters and its results
            min_Obj = objective;
            best_idx = i;
        }

        // 4. clean best matches of the mapped network
        if (i  <= good_itera_num - 1) {
            Map_Node_t * pNode;
            Map_Cut_t * pCut;
            p->nMatches = 0; 
            p->nPhases = 0; 
            for (int j = 0; j < p->vMapObjs->nSize; j++ ) { 
                pNode = p->vMapObjs->pArray[j];

                pNode->nRefAct[0] = pNode->nRefAct[1] = pNode->nRefAct[2] = 0;
                pNode->nRefEst[0] = pNode->nRefEst[1] = pNode->nRefEst[2] = 0;
                
                if ( Map_NodeIsBuf(pNode) )
                {
                    assert( pNode->p2 == NULL );
                    pNode->tArrival[0] = Map_Regular(pNode->p1)->tArrival[ Map_IsComplement(pNode->p1)];
                    pNode->tArrival[1] = Map_Regular(pNode->p1)->tArrival[!Map_IsComplement(pNode->p1)];
                    continue;
                }

                // skip primary inputs and secondary nodes if mapping with choices
                if ( !Map_NodeIsAnd( pNode ) || pNode->pRepr )
                    continue;

                // make sure that at least one non-trival cut is present
                if ( pNode->pCuts->pNext == NULL )
                {
                    // Extra_ProgressBarStop( pProgress );
                    printf( "\nError: A node in the mapping graph does not have feasible cuts.\n" );
                    return 0;
                }
                pNode->pCutBest[0] = NULL;
                pNode->pCutBest[1] = NULL; 
 
                pNode->tArrival[0].Rise = 0.0;
                pNode->tArrival[0].Fall = 0.0;
                pNode->tArrival[0].Worst = 0.0; 
                pNode->tArrival[1].Rise = 0.0;
                pNode->tArrival[1].Fall = 0.0;
                pNode->tArrival[1].Worst = 0.0; 

                pNode->tRequired[0].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[0].Fall = MAP_FLOAT_LARGE;
                pNode->tRequired[0].Worst =  MAP_FLOAT_LARGE; 
                pNode->tRequired[1].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Fall =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Worst =  MAP_FLOAT_LARGE;

                  
                for ( pCut = pNode->pCuts->pNext; pCut; pCut = pCut->pNext ) { 
                    Map_Match_t * pMatch =  pCut->M + 0;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL; 
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE;
                    pMatch->uPhaseBest = 286331153; 
                    
                    pMatch =  pCut->M + 1;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL;
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE;
                    pMatch->uPhaseBest = 286331153; 
                } 
            }
        }  
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    for (int i = 0; i < para_size; i++) p->delayParams[i] = goodPara[best_idx][i];
     ////////////////////////////////////////////////////////////////////// 
    clk = Abc_Clock();
    p->fMappingMode = 0;
    if ( !Map_MappingMatches( p) )
        return 0;
    p->timeMatch = Abc_Clock() - clk;
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
        
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using area flow 
    // compute the required times
    Map_TimeComputeRequiredGlobal( p );
    // recover area flow
    p->fMappingMode = 1;
    Map_MappingMatches( p );
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaFinal = Map_MappingGetArea( p );

 
    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area 
    // compute the required times
    Map_TimeComputeRequiredGlobal( p );
    // recover area
    p->fMappingMode = 2;
    Map_MappingMatches( p );
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaFinal = Map_MappingGetArea( p );

    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    // compute the required times
    Map_TimeComputeRequiredGlobal( p );
    // recover area
    p->fMappingMode = 3;
    Map_MappingMatches( p );
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaFinal = Map_MappingGetArea( p );
    //////////////////////////////////////////////////////////////////////
     
    


    // print the arrival times of the latest outputs
    if ( p->fVerbose )
        Map_MappingPrintOutputArrivals( p );
    return 1;
}

 
/*
int Map_MappingIteratable(Map_Man_t * p, Abc_Ntk_t *pNtk, Mio_Library_t *pLib, int fStime,  double DelayTarget, int fUseBuffs)
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
 
    // iterate mapping 
    int itera_num = 1;
    int para_size = 10; 
    int samplesize = 1;
    p->delayParams = malloc(sizeof(double) * para_size);
    // p->delayParams[0] = 0.5;
    // p->delayParams[1] = 0.3;
    // p->delayParams[2] = 0.1;
    // p->delayParams[3] = 0.5;
    // p->delayParams[4] = 1.0; 
    // p->delayParams[5] = 0.3;
    // p->delayParams[6] = 0.1;
    // p->delayParams[7] = 0.25;
    // p->delayParams[8] = 1.0;

    // p->delayParams[0] = 0.67;
    // p->delayParams[1] = 0.25;
    // p->delayParams[2] = 0.071;
    // p->delayParams[3] = 0.43;
    // p->delayParams[4] = 1.0; 
    // p->delayParams[5] = 0.34;
    // p->delayParams[6] = 0.22;
    // p->delayParams[7] = 0.27;
    // p->delayParams[8] = 0.97;

    // p->delayParams[0] = 0.986;
    // p->delayParams[1] = 0.660;
    // p->delayParams[2] = 0.57;
    // p->delayParams[3] = 0.056;
    // p->delayParams[4] = 0.527;
    // p->delayParams[5] = 0.443;
    // p->delayParams[6] = 0.986;
    // p->delayParams[7] = 0.986;
    // p->delayParams[8] = 0.833;
     
    // p->delayParams[0] = 0.920;
    // p->delayParams[1] = 0.389;
    // p->delayParams[2] = 0.338;
    // p->delayParams[3] = 0.639;
    // p->delayParams[4] = 0.639;
    // p->delayParams[5] = 0.431;
    // p->delayParams[6] = 0.611;
    // p->delayParams[7] = 0.389;
    // p->delayParams[8] = 0.505;
 

    // p->delayParams[0] = 0.565;
    // p->delayParams[1] = 0.000;
    // p->delayParams[2] = 0.668;
    // p->delayParams[3] = 0.579;
    // p->delayParams[4] = 0.803;
    // p->delayParams[5] = 0.000;
    // p->delayParams[6] = 0.127;
    // p->delayParams[7] = 0.052;
    // p->delayParams[8] = 0.448;
 
    bopt_params params = initialize_parameters_to_default();
    // set_learning(&params, "L_MCMC");
    double lb[] = {0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5};  
    double ub[] = {1.0, 0.5, 0.5, 1.0, 2.0, 0.5, 0.5, 1.0, 2.0};   
    double *xpoints = malloc(sizeof(double) * (para_size * samplesize)) ; 
    double *ypoints  = malloc(sizeof(double) * (samplesize)) ;
    void * bayesopt;
      
    for (int i  = 0; i < itera_num; i++) {
        double * xnext = malloc(sizeof(double) * para_size);
        double curDelay = 0.0;
        
        if ( i < samplesize ) {
            double randMax = 4294967295; 
            for (int j = 0; j < para_size; j++) {
                double randPV = fabs((double)Abc_Random(0));
                double norm = randPV / randMax;
                p->delayParams[j] = lb[j] + norm * (ub[j] - lb[j]);
                printf("%.3f, ", p->delayParams[j]);
            } 
            printf("\n");
        } else {
            nextPointIt(bayesopt, xnext);
            printf("predict xnext: ");
            // update the delay parameters
            for (int j = 0; j < para_size; j++) {
                p->delayParams[j] = xnext[j]; 
                printf("%.3f, ", xnext[j]);
            } 
            printf("\n");  
        }

        ////////////////////////////////////////////////////////////////////// 
        clk = Abc_Clock();
        p->fMappingMode = 0;
        if ( !Map_MappingMatches( p ) )
            return 0;
        p->timeMatch = Abc_Clock() - clk;
        // compute the references and collect the nodes used in the mapping
        // Map_MappingSetRefs( p );
        
        // p->AreaBase = Map_MappingGetArea( p );
        // if ( p->fVerbose )
        // {
        //     printf( "Delay    : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
        //             fShowSwitching? "Switch" : "Delay",
        //             fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
        //             Map_MappingGetAreaFlow(p), p->AreaBase, 0.0 );
        //     ABC_PRT( "Time", p->timeMatch );
        // }
        //////////////////////////////////////////////////////////////////////
    

        // 1. construct the mapped network, and store the mapped ID in Abc_obj_t
        extern Abc_Ntk_t *  Abc_NtkFromMap( Map_Man_t * pMan, Abc_Ntk_t * pNtk, int fUseBuffs );
        Abc_Ntk_t* pNtkMapped = Abc_NtkFromMap(p, pNtk, fUseBuffs || (DelayTarget == (double)ABC_INFINITY) );
        if ( Mio_LibraryHasProfile(pLib) )
                Mio_LibraryTransferProfile2( (Mio_Library_t *)Abc_FrameReadLibGen(), pLib );
        // Map_ManFree( p );
        if ( pNtkMapped == NULL )
            return 1;

        if ( pNtk->pExdc )
            pNtkMapped->pExdc = Abc_NtkDup( pNtk->pExdc );
        // make sure that everything is okay
        if ( !Abc_NtkCheck( pNtkMapped ) )
        {
            printf( "Abc_NtkMap: The network check has failed.\n" );
            Abc_NtkDelete( pNtkMapped );
            return 1;
        }
         
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
            
         // 3. perform STA
        int fShowAll      = 0;
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
        
        curDelay = pNtkTopoed ->MaxDelay; 
        printf("+++++++++++++++++curDelay: %f\n", curDelay);    
        if (i  < samplesize ){                               // init bayesopt 
            // double xpoints[] = {0.5, 0.3, 0.1, 0.5, 1.0, 0.3, 0.1, 0.25, 1.0}; 
            // double xpoints[] = {0.920, 0.389, 0.338, 0.639, 0.639, 0.431, 0.611, 0.389, 0.505};
            // double ypoints[]  = {curDelay};

            for (int j = 0; j < para_size; j++) {
                xpoints[i * para_size + j] = p->delayParams[j];
            }
            ypoints[i] = curDelay;
            if (i == samplesize -1)
                bayesopt = initializeOptimizationIt(para_size, lb, ub, samplesize, xpoints, ypoints, params);
        } 
        else {                                   // update bayesopt     
            addSampleIt(bayesopt, para_size, xnext,  curDelay, params, i);
        } 

        // 4. clean best matches of the mapped network
        if (i  < itera_num - 1) {
            Map_Node_t * pNode;
            Map_Cut_t * pCut;
            p->nMatches = 0; 
            p->nPhases = 0; 
            for (int j = 0; j < p->vMapObjs->nSize; j++ ) { 
                pNode = p->vMapObjs->pArray[j];
                if ( Map_NodeIsBuf(pNode) )
                {
                    assert( pNode->p2 == NULL );
                    pNode->tArrival[0] = Map_Regular(pNode->p1)->tArrival[ Map_IsComplement(pNode->p1)];
                    pNode->tArrival[1] = Map_Regular(pNode->p1)->tArrival[!Map_IsComplement(pNode->p1)];
                    continue;
                }

                // skip primary inputs and secondary nodes if mapping with choices
                if ( !Map_NodeIsAnd( pNode ) || pNode->pRepr )
                    continue;

                // make sure that at least one non-trival cut is present
                if ( pNode->pCuts->pNext == NULL )
                {
                    // Extra_ProgressBarStop( pProgress );
                    printf( "\nError: A node in the mapping graph does not have feasible cuts.\n" );
                    return 0;
                }
                pNode->pCutBest[0] = NULL;
                pNode->pCutBest[1] = NULL; 
 
                pNode->tArrival[0].Rise = 0.0;
                pNode->tArrival[0].Fall = 0.0;
                pNode->tArrival[0].Worst = 0.0; 
                pNode->tArrival[1].Rise = 0.0;
                pNode->tArrival[1].Fall = 0.0;
                pNode->tArrival[1].Worst = 0.0; 

                pNode->tRequired[0].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[0].Fall = MAP_FLOAT_LARGE;
                pNode->tRequired[0].Worst =  MAP_FLOAT_LARGE; 
                pNode->tRequired[1].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Fall =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Worst =  MAP_FLOAT_LARGE;

                 
                  
                for ( pCut = pNode->pCuts->pNext; pCut; pCut = pCut->pNext ) { 
                    Map_Match_t * pMatch =  pCut->M + 0;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL; 
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE;
                    pMatch->uPhaseBest = 286331153; 
                    
                    pMatch =  pCut->M + 1;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL;
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE;
                    pMatch->uPhaseBest = 286331153; 
                } 
            }
        } 

    }

   
    // print the arrival times of the latest outputs
    if ( p->fVerbose )
        Map_MappingPrintOutputArrivals( p );
    return 1;


}

*/


int Map_MappingHeboIt(Map_Man_t * p, Abc_Ntk_t *pNtk, Mio_Library_t *pLib, int fStime,  double DelayTarget, int fUseBuffs)
{   
    // the parameters for default `map` operator 
    int fShowSwitching         = 0;
    int fUseAreaFlow           = 1;
    int fUseExactArea          = !p->fSwitching;
    int fUseExactAreaWithPhase = !p->fSwitching;
    abctime clk, clk2;
    abctime clkInitPy = 0, clkIterExp = 0, clkIterBayes = 0, clkDeterPara = 0, clkGradient = 0, clkAreaRecovery = 0, clkDelayMap = 0, clkmapTT = 0, clkSTA = 0 ;

    clk = Abc_Clock();
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
    clkmapTT = Abc_Clock() - clk;
      
    // parameters for iteration 
    int itera_num = 30;
    int para_size = 10; 
    int rec_y_size = 1;
    p->delayParams = malloc(sizeof(double) * para_size);
    // some good delay parameters for both delay- and area- oriented mapping  
    // double goodPara[3][10] = {
    //     {0.736, 0.144, 0.349, 0.458, 1.025, 0.407, 0.020, 0.889, 1.288, 0.252},
    //     {0.5, 0.3, 0.1, 0.5, 1.0, 0.3, 0.1, 0.25, 1.0, 0.5},  // Expert Design
    //     {0.348, 0.061, 0.017, 0.146, 1.832, 0.411, 0.260, 0.050, 1.954, 0.782} // best result for bar
    // };
    // [0]=0.042, [1]=0.041, [2]=0.261, [3]=0.755, [4]=0.589, [5]=0.189, [6]=0.035, [7]=0.026, [8]=1.836, [9]=0.731, 
    // [0]=0.139, [1]=0.022, [2]=0.040, [3]=0.174, [4]=1.424, [5]=0.324, [6]=0.298, [7]=0.138, [8]=1.989, [9]=0.631,
    double goodPara[4][10] = {
        {0.042, 0.041, 0.261, 0.755, 0.589, 0.189, 0.035, 0.026, 1.836, 0.731},
        {0.139, 0.022, 0.040, 0.174, 1.424, 0.324, 0.298, 0.138, 1.989, 0.631},  // Expert Design
        {0.348, 0.061, 0.017, 0.146, 1.832, 0.411, 0.260, 0.050, 1.954, 0.782},  // best result for bar
        {0.905, 0.243, 0.114, 0.205, 1.363, 0.120, 0.384, 0.780, 1.407, 0.044}
        };


    int good_itera_num = 4; 
    
    // double goodPara[1][10] = {
    //     {0.6, 0.4, 0.4, 0.5, 1.0, 0.5, 0.5, 0.5, 0.5, 0.5} // random init to test the perfromance of the bayesian model. 
    // };
    // int good_itera_num = 3; 
    

    // for recording the delay parameters and the corresponding delay
    double * rec_x = (double *)malloc(para_size * sizeof(double));
    double * rec_y = (double *)malloc(rec_y_size * sizeof(double));
    memset(rec_x, 0, para_size * sizeof(double));
    memset(rec_y, 0, rec_y_size * sizeof(double));

    clk = Abc_Clock();
    // parameters for recording the best results
    ItResults* itRes = (ItResults*)malloc((itera_num+ good_itera_num) * sizeof(ItResults));
    double min_Y = MAP_FLOAT_LARGE;
    double *min_rec_x = NULL; 
     
    // parameters for iteration parameters
    double firstDelay = 0.0, firstArea = 0.0, firstLevel = 0.0, firstGate = 0.0, firstEdge = 0.0; 
    double curDelay = 0.0, curArea = 0.0, curLevel = 0.0, curGate = 0.0, curEdge = 0.0;
  
    // PyRun_SimpleString("import sys; import os; sys.path.append(os.getcwd());");
    PyRun_SimpleString("import sys;");
    PyRun_SimpleString("sys.path.append('/home/liujunfeng/ABC/abc_itmap/abc-itmap/src/map/mapper/')");
    

    PyObject* pModule = NULL;
    PyObject* pFuncInit = NULL, *pFuncIterate = NULL;
      
    pModule = PyImport_ImportModule("hebo_opt");
    if (!pModule) {
        printf("can't find hebo_opt.py\n");
        Py_Finalize();
        return 1;
    } 
    pFuncInit = PyObject_GetAttrString(pModule, "init_opt");
    if (!pFuncInit) {
        printf("can't find function init_opt\n");
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    pFuncIterate = PyObject_GetAttrString(pModule, "iterate_opt");
    if (!pFuncIterate) {
        printf("can't find function iterate_opt\n");
        Py_DECREF(pFuncInit);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }

    PyObject* pOpt = PyObject_CallObject(pFuncInit, NULL);
    if (!pOpt) {
        printf("init_opt failed to initialize\n");
        Py_DECREF(pFuncIterate);
        Py_DECREF(pFuncInit);
        Py_DECREF(pModule);
        Py_Finalize();
        return 1;
    }
    clkInitPy = Abc_Clock() - clk;
    
    clk = Abc_Clock();
    /////////////////////////////////////////////////////////////////////////////////////////////////////// 
    // init samples using some ``good delay parameters''
    for (int i = 0; i < good_itera_num; i++ ) {
        // update the parameters for p->delayParas
        for (int j = 0; j < para_size; j++)  p->delayParams[j] = goodPara[i][j];

        double estDepth = 0.0; 

        ////////////////////////////////////////////////////////////////////// 
        clk = Abc_Clock();
        p->fMappingMode = 0;
        if ( !Map_MappingMatches2( p, &estDepth) )
            return 0;
        p->timeMatch = Abc_Clock() - clk;
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p ); 
        //////////////////////////////////////////////////////////////////////
   
        /* area oriented mapping.  */
        //////////////////////////////////////////////////////////////////////
        // perform area recovery using area flow 
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area flow
        p->fMappingMode = 1;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
 

        //////////////////////////////////////////////////////////////////////
        // perform area recovery using exact area 
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 2;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );

        //////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////
        // perform area recovery using exact area
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 3;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
        //////////////////////////////////////////////////////////////////////
       
        clk = Abc_Clock();
        // 1. construct the mapped network, and store the mapped ID in Abc_obj_t
        extern Abc_Ntk_t *  Abc_NtkFromMap( Map_Man_t * pMan, Abc_Ntk_t * pNtk, int fUseBuffs );
        Abc_Ntk_t* pNtkMapped = Abc_NtkFromMap(p, pNtk, fUseBuffs || (DelayTarget == (double)ABC_INFINITY) );
        if ( Mio_LibraryHasProfile(pLib) )
                Mio_LibraryTransferProfile2( (Mio_Library_t *)Abc_FrameReadLibGen(), pLib );
        // Map_ManFree( p );
        if ( pNtkMapped == NULL )
            return 1;

        if ( pNtk->pExdc )
            pNtkMapped->pExdc = Abc_NtkDup( pNtk->pExdc );
        // make sure that everything is okay
        if ( !Abc_NtkCheck( pNtkMapped ) )
        {
            printf( "Abc_NtkMap: The network check has failed.\n" );
            Abc_NtkDelete( pNtkMapped );
            return 1;
        }
         
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
        
        /*// 3. perform Buffer   */
        SC_BusPars Pars, * pPars = &Pars;
        Abc_Ntk_t * pNtkResBuf; 
        memset( pPars, 0, sizeof(SC_BusPars) );
        pPars->GainRatio     =  300;
        pPars->Slew          =  Abc_SclComputeAverageSlew(Abc_FrameReadLibScl());
        pPars->nDegree       =   10;
        pPars->fSizeOnly     =    0;
        pPars->fAddBufs      =    1;
        pPars->fBufPis       =    0;
        pPars->fUseWireLoads =    0;
        pPars->fVerbose      =    0;
        pPars->fVeryVerbose  =    0;
        pNtkResBuf = Abc_SclBufferingPerform( pNtkTopoed, Abc_FrameReadLibScl(), pPars );

        {
        // upsize;
        SC_SizePars Pars, * pPars = &Pars; 
        memset( pPars, 0, sizeof(SC_SizePars) );
        pPars->nIters        = 1000;
        pPars->nIterNoChange =   50;
        pPars->Window        =    1;
        pPars->Ratio         =   10;
        pPars->Notches       = 1000;
        pPars->DelayUser     =    0;
        pPars->DelayGap      =    0;
        pPars->TimeOut       =    0;
        pPars->BuffTreeEst   =    0;
        pPars->BypassFreq    =    0;
        pPars->fUseDept      =    1;
        pPars->fUseWireLoads =    0;
        pPars->fDumpStats    =    0;
        pPars->fVerbose      =    0;
        pPars->fVeryVerbose  =    0;
        Abc_SclUpsizePerform(Abc_FrameReadLibScl(), pNtkResBuf, pPars);

        // dnsize; 
        pPars->nIters        =    5;
        pPars->nIterNoChange =   50;
        pPars->Notches       = 1000;
        pPars->DelayUser     =    0;
        pPars->DelayGap      = 1000;
        pPars->TimeOut       =    0;
        pPars->BuffTreeEst   =    0;
        pPars->fUseDept      =    1;
        pPars->fUseWireLoads =    0;
        pPars->fDumpStats    =    0;
        pPars->fVerbose      =    0;
        pPars->fVeryVerbose  =    0;
        Abc_SclDnsizePerform(Abc_FrameReadLibScl(), pNtkResBuf, pPars);
        }
       

        // 4. perform STA   pNtkTopoed   pNtkResBuf
        int fShowAll      = 0;
        int fUseWireLoads = 0;
        int fPrintPath    = 0;
        int fDumpStats    = 0;
        int nTreeCRatio   = 0;
        if ( !Abc_NtkHasMapping(pNtkResBuf) )
        {
            Abc_Print(-1, "The current network is not mapped.\n" );
            return 1;
        }
        if ( !Abc_SclCheckNtk(pNtkResBuf, 0) )
        {
            Abc_Print(-1, "The current network is not in a topo order (run \"topo\").\n" );
            return 1;
        }
        if ( Abc_FrameReadLibScl() == NULL )
        {
            Abc_Print(-1, "There is no Liberty library available.\n" );
            return 1;
        }
        printf("####    NLDM (%d)", i);
        extern void Abc_SclTimePerform( SC_Lib * pLib, Abc_Ntk_t * pNtk, int nTreeCRatio, int fUseWireLoads, int fShowAll, int fPrintPath, int fDumpStats );
        Abc_SclTimePerform( Abc_FrameReadLibScl(), pNtkResBuf, nTreeCRatio, fUseWireLoads, fShowAll, fPrintPath, fDumpStats );
        
        
        
        curDelay = pNtkResBuf ->MaxDelay;
        curArea = Map_MappingGetArea( p );
        curLevel = Abc_NtkLevel(pNtkResBuf);
        curGate = Abc_NtkGetLargeNodeNum(pNtkResBuf);
        curEdge = Abc_NtkGetTotalFanins(pNtkResBuf);

        if ( i == 0) {
            firstDelay = curDelay; 
            firstArea = curArea;
            firstLevel = curLevel;
            firstGate = curGate;
            firstEdge  = curEdge;
        }
        float gapDepth = 0.0;  
        if (curDelay > estDepth)
            gapDepth = estDepth/ curDelay;
        else 
            gapDepth = curDelay/estDepth; 

        rec_y[0] = curDelay/firstDelay + curArea/firstArea; 
        // rec_y[0] = curDelay/firstDelay + curArea/firstArea + curLevel/firstLevel + curGate/firstGate + curEdge/firstEdge;
        printf("#### Heuristic (%d) Delay = %.3f, Depth = %.3f, Level = %.1f, Edge = %.1f, Area = %.3f, Gate = %.1f, Objective = %.3f \n", i, curDelay, estDepth, curLevel, curEdge, curArea, curGate,  rec_y[0]);
  
        double* tmpParas = (double*)malloc(para_size * sizeof(double)); 
        memcpy(tmpParas, p->delayParams, para_size * sizeof(double));
        itRes[i].rec_x = tmpParas;
        itRes[i].rec_y = rec_y[0];

        if (itRes[i].rec_y < min_Y) {
            // record better delay parameters and its results
            min_Y = itRes[i].rec_y;
            min_rec_x = itRes[i].rec_x; 
        }

        // 4. clean best matches of the mapped network
        if (i  <= good_itera_num - 1) {
            Map_Node_t * pNode;
            Map_Cut_t * pCut;
            p->nMatches = 0; 
            p->nPhases = 0;

            // recovery matching state. 
            for (int j = 0; j < p->vMapObjs->nSize; j++ ) { 
                pNode = p->vMapObjs->pArray[j];
                // clear the references
                pNode->nRefAct[0] = pNode->nRefAct[1] = pNode->nRefAct[2] = 0;
                pNode->nRefEst[0] = pNode->nRefEst[1] = pNode->nRefEst[2] = 0;
                // clear the arrival times
                pNode->tArrival[0].Rise = 0.0;
                pNode->tArrival[0].Fall = 0.0;
                pNode->tArrival[0].Worst = 0.0; 
                pNode->tArrival[1].Rise = 0.0;
                pNode->tArrival[1].Fall = 0.0;
                pNode->tArrival[1].Worst = 0.0; 

                // clear the required times
                pNode->tRequired[0].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[0].Fall = MAP_FLOAT_LARGE;
                pNode->tRequired[0].Worst =  MAP_FLOAT_LARGE; 
                pNode->tRequired[1].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Fall =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Worst =  MAP_FLOAT_LARGE;
                 
                if ( Map_NodeIsBuf(pNode) )
                {
                    assert( pNode->p2 == NULL );
                    pNode->tArrival[0] = Map_Regular(pNode->p1)->tArrival[ Map_IsComplement(pNode->p1)];
                    pNode->tArrival[1] = Map_Regular(pNode->p1)->tArrival[!Map_IsComplement(pNode->p1)];
                    continue;
                }

                // // skip primary inputs and secondary nodes if mapping with choices
                if ( !Map_NodeIsAnd( pNode ) || pNode->pRepr ){
                    continue;
                }
                     
                // make sure that at least one non-trival cut is present
                if ( pNode->pCuts->pNext == NULL )
                {
                    // Extra_ProgressBarStop( pProgress );
                    return 0;
                }

                pNode->pCutBest[0] = NULL;
                pNode->pCutBest[1] = NULL; 

                // clean the node matches   
                for ( pCut = pNode->pCuts->pNext; pCut; pCut = pCut->pNext ) { 
                    Map_Match_t * pMatch =  pCut->M + 0;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL; 
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE;
                    // pMatch->uPhaseBest = 286331153; 
                      
                    pMatch =  pCut->M + 1;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL;
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE; 
                    // pMatch->uPhaseBest = 286331153; 
                } 
            }
        } 
 
        clkSTA += Abc_Clock() - clk;
        // create args for iterate_opt: 
        // def iterate_opt(opt, i_iter,  given_rec_x : List[float], given_rec_y : List[float]):

        // 1. the first one: opt
        PyObject* pArgs = PyTuple_New(4);
        PyTuple_SetItem(pArgs, 0, pOpt);  
        Py_INCREF(pOpt); // Increase ref count because PyTuple_SetItem steals a reference
        
        // 2. the second one: iteration number -1 stands for only calling opt.oberserve
        PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", -1)); 
        
        // 3. the third one: rec_x
        PyObject* pListX = PyList_New(para_size); 
        for (int j = 0; j < para_size; j++) 
            PyList_SetItem(pListX, j, PyFloat_FromDouble(p->delayParams[j]));
        PyTuple_SetItem(pArgs, 2, pListX);

        // 4. the fourth one: rec_y
        PyObject* pListY = PyList_New(rec_y_size);
        PyList_SetItem(pListY, 0, PyFloat_FromDouble(rec_y[0]));
        PyTuple_SetItem(pArgs, 3, pListY);

        PyObject* pReturn = PyObject_CallObject(pFuncIterate, pArgs);
        Py_DECREF(pArgs); // Decrease ref count after calling the function
        Py_DECREF(pListX);
        Py_DECREF(pListY);

        // parse the return values 
        if (pReturn != NULL && PyTuple_Check(pReturn)) {
            PyObject* pNewOpt;
            PyObject* pNewListX;
            if (PyArg_ParseTuple(pReturn, "OO", &pNewOpt, &pNewListX)) {
                Py_INCREF(pNewOpt); // ParseTuple does not increase ref count
                Py_DECREF(pOpt); // Replace the old opt object
                pOpt = pNewOpt; 
            } else {
                printf("Failed to parse return value\n");
            }

            Py_DECREF(pReturn);

        } else {
            printf("Function call failed or returned NULL\n");
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    clkIterExp = Abc_Clock() - clk;

    clk = Abc_Clock();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    memset(rec_x, 0, para_size * sizeof(double));
    memset(rec_y, 0, rec_y_size * sizeof(double));
    // fit models using GP Kernel 
    for (int i = 0; i <  itera_num; i ++ ) {
        // create args for iterate_opt: 
        // def iterate_opt(opt, i_iter,  given_rec_x : List[float], given_rec_y : List[float]):
        clk2 = Abc_Clock();
        // 1. the first one: opt
        PyObject* pArgs = PyTuple_New(4);
        PyTuple_SetItem(pArgs, 0, pOpt);  
        Py_INCREF(pOpt); // Increase ref count because PyTuple_SetItem steals a reference
        
        // 2. the second one: iteration number
        PyTuple_SetItem(pArgs, 1, Py_BuildValue("i", i)); 
        
        // 3. the third one: rec_x
        PyObject* pListX = PyList_New(para_size); 
        for (int j = 0; j < para_size; j++) 
            PyList_SetItem(pListX, j, PyFloat_FromDouble(rec_x[j]));
        PyTuple_SetItem(pArgs, 2, pListX);

        // 4. the fourth one: rec_y
        PyObject* pListY = PyList_New(rec_y_size);
        PyList_SetItem(pListY, 0, PyFloat_FromDouble(rec_y[0]));
        PyTuple_SetItem(pArgs, 3, pListY);

        PyObject* pReturn = PyObject_CallObject(pFuncIterate, pArgs);
        Py_DECREF(pArgs); // Decrease ref count after calling the function
        Py_DECREF(pListX);
        Py_DECREF(pListY);

        // parse the return values 
        printf("#### Parameters(%d) ", i);
        if (pReturn != NULL && PyTuple_Check(pReturn)) {
            PyObject* pNewOpt;
            PyObject* pNewListX;
            if (PyArg_ParseTuple(pReturn, "OO", &pNewOpt, &pNewListX)) {
                Py_INCREF(pNewOpt); // ParseTuple does not increase ref count
                Py_DECREF(pOpt); // Replace the old opt object
                pOpt = pNewOpt;
                
                int x_size = PyList_Size(pNewListX);
                if (x_size == para_size) {
                    for (int k = 0; k < x_size; k++) {
                        PyObject* pItem = PyList_GetItem(pNewListX, k); // Borrowed reference, no need to DECREF
                        if (PyFloat_Check(pItem)) {
                            rec_x[k] = PyFloat_AsDouble(pItem);
                            printf("[%d]=%.3f, ", k, rec_x[k]);
                        } else {
                            printf("List item is not a float!\n");
                        }
                    }
                } else {
                    printf("Returned list size does not match expected size\n");
                }
                printf("\n");
            } else {
                printf("Failed to parse return value\n");
            }

            Py_DECREF(pReturn);

        } else {
            printf("Function call failed or returned NULL\n");
        }
        // update the parameters for p->delayParas
        for (int j = 0; j < para_size; j++) p->delayParams[j] = rec_x[j];
        
        clkDeterPara += Abc_Clock() - clk2;

        double estDepth = 0.0; 

        ////////////////////////////////////////////////////////////////////// 
        clk = Abc_Clock();
        p->fMappingMode = 0;
        if ( !Map_MappingMatches2( p, &estDepth) )
            return 0;
        p->timeMatch = Abc_Clock() - clk;
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        clkDelayMap += Abc_Clock() - clk;
        //////////////////////////////////////////////////////////////////////

        /* area oriented mapping. */
        //////////////////////////////////////////////////////////////////////
        // perform area recovery using area flow 
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area flow
        p->fMappingMode = 1;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
 

        //////////////////////////////////////////////////////////////////////
        // perform area recovery using exact area 
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 2;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );

        //////////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////////
        // perform area recovery using exact area
        // compute the required times
        Map_TimeComputeRequiredGlobal( p );
        // recover area
        p->fMappingMode = 3;
        Map_MappingMatches( p );
        // compute the references and collect the nodes used in the mapping
        Map_MappingSetRefs( p );
        p->AreaFinal = Map_MappingGetArea( p );
        //////////////////////////////////////////////////////////////////////
        
        abctime clk_t2 = Abc_Clock();
        // 1. construct the mapped network, and store the mapped ID in Abc_obj_t
        extern Abc_Ntk_t *  Abc_NtkFromMap( Map_Man_t * pMan, Abc_Ntk_t * pNtk, int fUseBuffs );
        Abc_Ntk_t* pNtkMapped = Abc_NtkFromMap(p, pNtk, fUseBuffs || (DelayTarget == (double)ABC_INFINITY) );
        if ( Mio_LibraryHasProfile(pLib) )
                Mio_LibraryTransferProfile2( (Mio_Library_t *)Abc_FrameReadLibGen(), pLib );
        // Map_ManFree( p );
        if ( pNtkMapped == NULL )
            return 1;

        if ( pNtk->pExdc )
            pNtkMapped->pExdc = Abc_NtkDup( pNtk->pExdc );
        // make sure that everything is okay
        if ( !Abc_NtkCheck( pNtkMapped ) )
        {
            printf( "Abc_NtkMap: The network check has failed.\n" );
            Abc_NtkDelete( pNtkMapped );
            return 1;
        }
         
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
        
        /*
        // 3. perform Buffer*/
        SC_BusPars Pars, * pPars = &Pars;
        Abc_Ntk_t * pNtkResBuf; 
        memset( pPars, 0, sizeof(SC_BusPars) );
        pPars->GainRatio     =  300;
        pPars->Slew          =  Abc_SclComputeAverageSlew(Abc_FrameReadLibScl());
        pPars->nDegree       =   10;
        pPars->fSizeOnly     =    0;
        pPars->fAddBufs      =    1;
        pPars->fBufPis       =    0;
        pPars->fUseWireLoads =    0;
        pPars->fVerbose      =    0;
        pPars->fVeryVerbose  =    0;
        pNtkResBuf = Abc_SclBufferingPerform( pNtkTopoed, Abc_FrameReadLibScl(), pPars );

        {
        // upsize;
        SC_SizePars Pars, * pPars = &Pars; 
        memset( pPars, 0, sizeof(SC_SizePars) );
        pPars->nIters        = 1000;
        pPars->nIterNoChange =   50;
        pPars->Window        =    1;
        pPars->Ratio         =   10;
        pPars->Notches       = 1000;
        pPars->DelayUser     =    0;
        pPars->DelayGap      =    0;
        pPars->TimeOut       =    0;
        pPars->BuffTreeEst   =    0;
        pPars->BypassFreq    =    0;
        pPars->fUseDept      =    1;
        pPars->fUseWireLoads =    0;
        pPars->fDumpStats    =    0;
        pPars->fVerbose      =    0;
        pPars->fVeryVerbose  =    0;
        Abc_SclUpsizePerform(Abc_FrameReadLibScl(), pNtkResBuf, pPars);

        // dnsize; 
        pPars->nIters        =    5;
        pPars->nIterNoChange =   50;
        pPars->Notches       = 1000;
        pPars->DelayUser     =    0;
        pPars->DelayGap      = 1000;
        pPars->TimeOut       =    0;
        pPars->BuffTreeEst   =    0;
        pPars->fUseDept      =    1;
        pPars->fUseWireLoads =    0;
        pPars->fDumpStats    =    0;
        pPars->fVerbose      =    0;
        pPars->fVeryVerbose  =    0;
        Abc_SclDnsizePerform(Abc_FrameReadLibScl(), pNtkResBuf, pPars);
        }

        

        // 4. perform STA  pNtkTopoed  pNtkResBuf
        int fShowAll      = 0;
        int fUseWireLoads = 0;
        int fPrintPath    = 0;
        int fDumpStats    = 0;
        int nTreeCRatio   = 0;
        if ( !Abc_NtkHasMapping(pNtkResBuf) )
        {
            Abc_Print(-1, "The current network is not mapped.\n" );
            return 1;
        }
        if ( !Abc_SclCheckNtk(pNtkResBuf, 0) )
        {
            Abc_Print(-1, "The current network is not in a topo order (run \"topo\").\n" );
            return 1;
        }
        if ( Abc_FrameReadLibScl() == NULL )
        {
            Abc_Print(-1, "There is no Liberty library available.\n" );
            return 1;
        }
        printf("####    NLDM (%d)", i);
        extern void Abc_SclTimePerform( SC_Lib * pLib, Abc_Ntk_t * pNtk, int nTreeCRatio, int fUseWireLoads, int fShowAll, int fPrintPath, int fDumpStats );
        Abc_SclTimePerform( Abc_FrameReadLibScl(), pNtkResBuf, nTreeCRatio, fUseWireLoads, fShowAll, fPrintPath, fDumpStats );
         
        curDelay = pNtkResBuf ->MaxDelay;
        curArea = Map_MappingGetArea( p );
        curLevel = Abc_NtkLevel(pNtkResBuf);
        curGate = Abc_NtkGetLargeNodeNum(pNtkResBuf);
        curEdge = Abc_NtkGetTotalFanins(pNtkResBuf);

         float gapDepth = 0.0;  
        if (curDelay > estDepth)
            gapDepth = estDepth/ curDelay;
        else 
            gapDepth = curDelay/estDepth; 
            
        rec_y[0] = curDelay/firstDelay + curArea/firstArea; 

        // rec_y[0] = curDelay/firstDelay + curArea/firstArea + curLevel/firstLevel + curGate/firstGate + curEdge/firstEdge;
        printf("#### Heuristic (%d) Delay = %.3f, Depth = %.3f, Level = %.1f, Edge = %.1f, Area = %.3f, Gate = %.1f, Objective = %.3f \n", i, curDelay, estDepth, curLevel, curEdge, curArea, curGate,  rec_y[0]);
          
       
        double* tmpParas = (double*)malloc(para_size * sizeof(double)); 
        memcpy(tmpParas, p->delayParams, para_size * sizeof(double));
        itRes[i+good_itera_num].rec_x = tmpParas;
        itRes[i+good_itera_num].rec_y = rec_y[0];

        clk2 = Abc_Clock();
        if (itRes[i+good_itera_num].rec_y < min_Y) {
            min_Y = itRes[i+good_itera_num].rec_y;
            min_rec_x = itRes[i+good_itera_num].rec_x;
         
            // do not update local references. 
            //  // update local References. 
            // Abc_Obj_t * pObj;
            // Map_Node_t * pNodeMap;
            // Map_Cut_t * pCutBest;
            // Map_Super_t *  pSuperBest; 
            // int ni,  mappingID, fPhase;
            // float gateDelay;
            
            // double *grad = malloc(sizeof(double) * (MAP_TAO*2));
            // memset(grad, 0, sizeof(double) * (MAP_TAO*2));
            // int gate_params_size = 6;
            // double *gateParams = malloc(sizeof(double) * gate_params_size);
            // memset(gateParams, 0, sizeof(double) * gate_params_size);
            
            // int updatedNode = 0; 
                
            // Abc_NtkForEachNode1( pNtkTopoed, pObj, ni ){  
            //     mappingID = Abc_ObjMapNtkId(pObj);
            //     fPhase =  Abc_ObjMapNtkPhase(pObj);
            //     gateDelay = Abc_ObjMapNtkTime(pObj);
            //     pNodeMap = p->vMapObjs->pArray[mappingID];
            //     // update the tauRef using gradient descent
            //     // skip the node that has no cut
            //     if ( Map_NodeReadCutBest(pNodeMap, fPhase) == NULL ) 
            //         continue; 
            //     pCutBest = Map_NodeReadCutBest(pNodeMap, fPhase);    
            //     pSuperBest = pCutBest->M[fPhase].pSuperBest;

            //     Map_MappingGradient(p, pCutBest,  pSuperBest, fPhase, grad, gateParams);
            //     if (Map_MappingUpdateTauRef(p, pNodeMap, pCutBest, pSuperBest, fPhase, gateDelay, grad, gateParams)) {
            //         updatedNode += 1; 
            //     }
            //     // pNodeMap->tauRefs[1],
            // }
            // printf("#### Updated node proportion(%.3f) \n", (updatedNode*1.0)/ni);
            // free(grad);
            // free(gateParams); 

        }
        clkGradient += Abc_Clock() - clk2;

        // 4. clean best matches of the mapped network
        if (i  <= itera_num - 1) {
            Map_Node_t * pNode;
            Map_Cut_t * pCut;
            p->nMatches = 0; 
            p->nPhases = 0; 
            
             // recovery matching state. 
            for (int j = 0; j < p->vMapObjs->nSize; j++ ) { 
                pNode = p->vMapObjs->pArray[j];
                // clear the references
                pNode->nRefAct[0] = pNode->nRefAct[1] = pNode->nRefAct[2] = 0;
                pNode->nRefEst[0] = pNode->nRefEst[1] = pNode->nRefEst[2] = 0;
                // clear the arrival times
                pNode->tArrival[0].Rise = 0.0;
                pNode->tArrival[0].Fall = 0.0;
                pNode->tArrival[0].Worst = 0.0; 
                pNode->tArrival[1].Rise = 0.0;
                pNode->tArrival[1].Fall = 0.0;
                pNode->tArrival[1].Worst = 0.0; 

                // clear the required times
                pNode->tRequired[0].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[0].Fall = MAP_FLOAT_LARGE;
                pNode->tRequired[0].Worst =  MAP_FLOAT_LARGE; 
                pNode->tRequired[1].Rise =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Fall =  MAP_FLOAT_LARGE;
                pNode->tRequired[1].Worst =  MAP_FLOAT_LARGE;
                 
                
                if ( Map_NodeIsBuf(pNode) )
                {
                    assert( pNode->p2 == NULL );
                    pNode->tArrival[0] = Map_Regular(pNode->p1)->tArrival[ Map_IsComplement(pNode->p1)];
                    pNode->tArrival[1] = Map_Regular(pNode->p1)->tArrival[!Map_IsComplement(pNode->p1)];
                    continue;
                }

                // // skip primary inputs and secondary nodes if mapping with choices
                if ( !Map_NodeIsAnd( pNode ) || pNode->pRepr ){
                    continue;
                }
                     
                // make sure that at least one non-trival cut is present
                if ( pNode->pCuts->pNext == NULL )
                {
                    // Extra_ProgressBarStop( pProgress );
                    return 0;
                }
                
                pNode->pCutBest[0] = NULL;
                pNode->pCutBest[1] = NULL; 

                // clean the node matches   
                for ( pCut = pNode->pCuts->pNext; pCut; pCut = pCut->pNext ) { 
                    Map_Match_t * pMatch =  pCut->M + 0;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL; 
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE;
                    // pMatch->uPhaseBest = 286331153; 
                      
                    pMatch =  pCut->M + 1;
                    if (pMatch->pSuperBest) {
                        pMatch->pSuperBest = NULL;
                    }
                    pMatch->tArrive.Rise = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Fall = MAP_FLOAT_LARGE;
                    pMatch->tArrive.Worst = MAP_FLOAT_LARGE;
                    pMatch->AreaFlow = MAP_FLOAT_LARGE; 
                    // pMatch->uPhaseBest = 286331153; 
                } 
            }
 

        } 
        clkSTA += Abc_Clock() - clk_t2;
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    clkIterBayes = Abc_Clock() - clk;

    // set parameters for the best iteration

    printf("#### Best para");
    for (int j = 0; j < para_size; j++){
        p->delayParams[j] = min_rec_x[j];
        printf("[%d]=%.3f, ", j, min_rec_x[j]);
    }
    printf("\n");

    clk = Abc_Clock();
    ////////////////////////////////////////////////////////////////////// 
    clk = Abc_Clock();
    p->fMappingMode = 0;
    if ( !Map_MappingMatches( p) )
        return 0;
    p->timeMatch = Abc_Clock() - clk;
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
        
    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using area flow 
    // compute the required times
    Map_TimeComputeRequiredGlobal( p );
    // recover area flow
    p->fMappingMode = 1;
    Map_MappingMatches( p );
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaFinal = Map_MappingGetArea( p );

 
    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area 
    // compute the required times
    Map_TimeComputeRequiredGlobal( p );
    // recover area
    p->fMappingMode = 2;
    Map_MappingMatches( p );
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaFinal = Map_MappingGetArea( p );

    //////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    // perform area recovery using exact area
    // compute the required times
    Map_TimeComputeRequiredGlobal( p );
    // recover area
    p->fMappingMode = 3;
    Map_MappingMatches( p );
    // compute the references and collect the nodes used in the mapping
    Map_MappingSetRefs( p );
    p->AreaFinal = Map_MappingGetArea( p );
    //////////////////////////////////////////////////////////////////////
    clkAreaRecovery = Abc_Clock() - clk;
    

     
    // print the arrival times of the latest outputs
    if ( p->fVerbose ){
        Map_MappingPrintOutputArrivals( p );
        ABC_PRT("Runtime for init Python", clkInitPy);
        ABC_PRT("Runtime for iter expert parameters", clkIterExp);
        // ABC_PRT("Runtime for determining parameters", clkDeterPara);
        Abc_Print( 1, "Runtime for determining parameters = %.1f sec ",   1.0*clkDeterPara/CLOCKS_PER_SEC  );
        ABC_PRT("Runtime for iter bayesian opt", clkIterBayes); 
        Abc_Print( 1, "Runtime for local gradient = %.1f sec ",   1.0*clkGradient/CLOCKS_PER_SEC  );
        ABC_PRT("Runtime for area recovery", clkAreaRecovery);
        ABC_PRT("Runtime for delay oriented mapping", clkDelayMap);
        ABC_PRT("Runtime for mapping TT", clkmapTT);
        Abc_Print( 1, "Update graph and STA= %.1f sec ",   1.0*clkSTA/CLOCKS_PER_SEC  );
    }
        
    return 1;


}


void  Map_MappingGradient(Map_Man_t * p,  Map_Cut_t *pCut,  Map_Super_t *pSuper, int  fPhase,  double * grad, double *gatePara){
     /*
     fanoutEffortTrans  = pCut->ppLeaves[i]->nRefEst[fPhase] + pCut->ppLeaves[i]->tauRefs[1] * pNode->p->delayParams[1] + pCut->ppLeaves[i]->tauRefs[2] * 0.2 * pNode->p->delayParams[1] + 10* pNode->p->delayParams[2];
            estTransDelay = pNode->p->delayParams[0] * (fanoutEffortTrans * pSuper->tDelaysRTransLD[i].Rise * 10 * pNode->p->delayParams[3]+ pSuper->tDelaysRTransPD[i].Rise * pNode->p->delayParams[4]);
            fanoutEffortCap = pNode->nRefEst[fPhase] + pNode->tauRefs[1]* pNode->p->delayParams[5] +  pNode->tauRefs[2]* 0.2* pNode->p->delayParams[5] +  10 * pNode->p->delayParams[6];
            estCapDelay =  (1-pNode->p->delayParams[0]) * (fanoutEffortCap * pSuper->tDelaysRLD[i].Rise * 10* pNode->p->delayParams[7] + pSuper->tDelaysRPD[i].Rise * pNode->p->delayParams[8]);
            estDelay = estCapDelay + estTransDelay + tInvDelayRise;
    */


    // compute the gradient of the mapping delay with respect to the delay parameters
    int nLeaves = pSuper->nFanins; 
    double avgTLD = 0.0, avgCLD =0.0, avgTPD = 0.0, avgCPD = 0.0;
    double maxTransEfforts = 0, curTransEfforts = 0; 
   
    int pi = 0; 

    for (int i = 0; i < nLeaves; i ++) {
        curTransEfforts = pCut->ppLeaves[i]->nRefEst[fPhase] + pCut->ppLeaves[i]->tauRefs[1] * p->delayParams[1] + pCut->ppLeaves[i]->tauRefs[2] * 0.2 * p->delayParams[1] + 10* p->delayParams[2];
        if (maxTransEfforts < curTransEfforts) {
            maxTransEfforts = curTransEfforts;
            pi = i;
        } 
        avgTLD += pSuper->tDelaysRTransLD[i].Rise + pSuper->tDelaysRTransLD[i].Fall;
        avgCLD += pSuper->tDelaysRLD[i].Rise + pSuper->tDelaysRLD[i].Fall;
        avgTPD += pSuper->tDelaysRTransPD[i].Rise + pSuper->tDelaysRTransPD[i].Fall;
        avgCPD = pSuper->tDelaysRPD[i].Rise + pSuper->tDelaysRPD[i].Fall;        
    }
    avgTLD /= nLeaves*2;
    avgCLD /= nLeaves*2;
    avgTPD /= nLeaves*2;
    avgCPD /= nLeaves*2;
    grad[0] = p->delayParams[0] * avgTLD * 10 * p->delayParams[3];
    grad[1] = p->delayParams[1] * grad[0];
    grad[2] = 0.2 * p->delayParams[1] * grad[0];
    
    grad[3] = (1-p->delayParams[0]) * avgCLD * 10 * p->delayParams[7];
    grad[4] = p->delayParams[4] * grad[0];
    grad[5] = 0.2 * p->delayParams[5] * grad[0];

    // 0: for max cut delay pin
    gatePara[0] = pi; 
    gatePara[1] =avgTLD;
    gatePara[2] =avgCLD;
    gatePara[3] =avgTPD;
    gatePara[4] =avgCPD;
    // return grad;
}

int  Map_MappingUpdateTauRef(Map_Man_t * p, Map_Node_t *pNode, Map_Cut_t *pCut, Map_Super_t *pSuper, int fPhase, double gateDelay, double * grad, double *gatePara){ 
    int max_iterations = 3;
    double lr = 0.08;
    double tolerance = 40; 
    int pi = gatePara[0];

    double cutDelay = Map_MappingEstCutDelay(p, pCut, pNode, fPhase, pi, gatePara);
    double gradDirection = 0;
    if (cutDelay - gateDelay > tolerance) { 
        gradDirection = 1;
    } else if ( cutDelay - gateDelay < -1* tolerance ) { 
        gradDirection = -1;
    } else {
        return 0;
    } 

    for (int iteration = 0; iteration < max_iterations; iteration++) { 
        pNode->nRefEst[0] = pNode->nRefEst[0] - lr * gradDirection * grad[3];
        pNode->nRefEst[1] = pNode->nRefEst[1] - lr * gradDirection * grad[3];
        pNode->nRefEst[2] = pNode->nRefEst[2] - lr * gradDirection * grad[3];  
        
        pNode->tauRefs[1] = pNode->tauRefs[1] - lr * gradDirection * grad[4];
        pNode->tauRefs[2] = pNode->tauRefs[2] - lr * gradDirection * grad[5];

        pCut->ppLeaves[pi]->nRefEst[0] = pCut->ppLeaves[pi]->nRefEst[0] - lr * gradDirection * grad[0];
        pCut->ppLeaves[pi]->nRefEst[1] = pCut->ppLeaves[pi]->nRefEst[1] - lr * gradDirection * grad[0];
        pCut->ppLeaves[pi]->nRefEst[2] = pCut->ppLeaves[pi]->nRefEst[2] - lr * gradDirection * grad[0];
        pCut->ppLeaves[pi]->tauRefs[1] = pCut->ppLeaves[pi]->tauRefs[1] - lr * gradDirection * grad[1];
        pCut->ppLeaves[pi]->tauRefs[2] = pCut->ppLeaves[pi]->tauRefs[2] - lr * gradDirection * grad[2];

        cutDelay = Map_MappingEstCutDelay(p, pCut, pNode, fPhase, pi, gatePara);

        if (fabs(cutDelay- gateDelay ) < tolerance) {
            // printf("Converged after %d iterations.\n", iteration + 1);
            break;
        }
    }
   
    return 1; 

}

double Map_MappingEstCutDelay (Map_Man_t *p, Map_Cut_t *pCut, Map_Node_t *pNode, int fPhase, int pi,  double *gatePara) {
    double transEfforts = pCut->ppLeaves[pi]->nRefEst[fPhase] + pCut->ppLeaves[pi]->tauRefs[1] * p->delayParams[1] +
             pCut->ppLeaves[pi]->tauRefs[2] * 0.2 * p->delayParams[1] + 10* p->delayParams[2];
    double capEfforts = pNode->nRefEst[fPhase] + pNode->tauRefs[1]* p->delayParams[5] + pNode->tauRefs[2]* 0.2* p->delayParams[5] + 10 * p->delayParams[6]; 
    double cutDelay = p->delayParams[0] * (transEfforts * gatePara[1] * 10 * p->delayParams[3] + gatePara[3] * p->delayParams[4]) + 
                     (1-p->delayParams[0]) * (capEfforts * gatePara[2] * 10* p->delayParams[7] + gatePara[4] * p->delayParams[8]);
    return cutDelay;
}

ABC_NAMESPACE_IMPL_END

