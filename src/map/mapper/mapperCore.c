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
    double para[10] = {0.736, 0.144, 0.349, 0.458, 1.025, 0.407, 0.020, 0.889, 1.288, 0.252};
    for(int i = 0; i < para_size; i++) {
            p->delayParams[i] = para[i];
     } 


    
    // sin  Area =     5018.32 ( 80.0 %)   Delay =  2843.28 ps 
    //  Area =    10747.21 ( 97.4 %)   Delay =  3969.87 ps
    // para[0]=0.814, para[1]=0.366, para[2]=0.262, para[3]=0.705, para[4]=1.826, para[5]=0.392, para[6]=0.283, para[7]=0.461, para[8]=0.716, para[9]=0.322, 
    // double para[10] = {0.814, 0.366, 0.262, 0.705, 1.826, 0.392, 0.283, 0.461, 0.716, 0.322};
    // for(int i = 0; i < para_size; i++) {
    //         p->delayParams[i] = para[i];
    //  } 

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
       4. update the parameter of CUT delay
   */

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

   /*
   // print the mapped_network
   printf("\nReturn mapped Ntk...\n");
   Abc_Obj_t * pNode;
   int i1 = 0;
   Abc_NtkForEachObj(pNtkMapped, pNode, i1 ) {
       if(Abc_ObjIsCi(pNode)){
           printf("CI index=(%d), mappingID=(%d) \n", i1, Abc_ObjMapNtkId(pNode));
       }
       if (Abc_ObjIsNode(pNode)){
           printf("Cell index=(%d), mappingID=(%d) \n", i1, Abc_ObjMapNtkId(pNode));
       }
       if (Abc_ObjIsCo(pNode)){
           printf("CO index=(%d), mappingID=(%d) \n", i1, Abc_ObjMapNtkId(pNode));
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
   Abc_Obj_t *pNodej; int j;
   Abc_NtkForEachObj(pNtkTopoed, pNodej, j ) {
       if (j < 100 || j > 110) continue;
       if(Abc_ObjIsCi(pNodej)){
           printf("node(%d), index(%d), CI\n", Abc_ObjId(pNodej), j);
       }
       if (Abc_ObjIsNode(pNodej)){
           printf("node(%d), index(%d), mapNtkID(%d), mapNtkPhase(%d), Node\n", Abc_ObjId(pNodej) , j, Abc_ObjMapNtkId(pNodej),
                   Abc_ObjMapNtkPhase(pNodej));
       }
       if (Abc_ObjIsCo(pNodej)){
           printf("node(%d), index(%d), CO\n", Abc_ObjId(pNodej), j);
       }
   }
  */

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

   // 4. update delay of STA in the the mapping graph (rather the mapped network)
   Abc_Obj_t * pObj;
   Map_Node_t * pNodeMap;
   Map_Cut_t * pCutBest;
   Map_Super_t *       pSuperBest; 
   int i,  mappingID, fPhase;
   float gateDelay;
   
   double *grad = malloc(sizeof(double) * MAP_TAO);
   memset(grad, 0, sizeof(double) * MAP_TAO);
   int gate_params_size = 5;
   double *gateParams = malloc(sizeof(double) * gate_params_size);
   memset(gateParams, 0, sizeof(double) * gate_params_size);
   
   
    Abc_NtkForEachNode1( pNtkTopoed, pObj, i ){  
        mappingID = Abc_ObjMapNtkId(pObj);
        fPhase =  Abc_ObjMapNtkPhase(pObj);
        gateDelay = Abc_ObjMapNtkTime(pObj);
        pNodeMap = p->vMapObjs->pArray[mappingID];
        // update the tauRef using gradient descent
        // skip the node that has no cut
        if ( Map_NodeReadCutBest(pNodeMap, fPhase) == NULL ) 
            continue; 
        pCutBest = Map_NodeReadCutBest(pNodeMap, fPhase);    
        pSuperBest = pCutBest->M[fPhase].pSuperBest;

        Map_MappingGradient(p, pCutBest,  pSuperBest, fPhase, grad, gateParams);
        Map_MappingUpdateTauRef(p, pNodeMap, pCutBest, pSuperBest, fPhase, gateDelay, grad, gateParams);
        // pNodeMap->tauRefs[1],
    }


   /*
   Abc_NtkForEachCi( pNtkTopoed, pObj, i ){
       printf("CI node id=(%d) MappingID=(%d) Phase(%d) Delay=(%4.4f) \n", Abc_ObjId(pObj), Abc_ObjMapNtkId(pObj), Abc_ObjMapNtkPhase(pObj), Abc_ObjMapNtkTime(pObj));
   }
   Abc_NtkForEachNode1( pNtkTopoed, pObj, i ){
       printf("Cell node id=(%d) MappingID=(%d) Phase(%d) Delay=(%4.4f) \n", Abc_ObjId(pObj), Abc_ObjMapNtkId(pObj), Abc_ObjMapNtkPhase(pObj), Abc_ObjMapNtkTime(pObj));
   }
   Abc_NtkForEachCo( pNtkTopoed, pObj, i ){
       printf("Co node id=(%d) MappingID=(%d) Phase(%d) Delay=(%4.4f) \n", Abc_ObjId(pObj), Abc_ObjMapNtkId(pObj), Abc_ObjMapNtkPhase(pObj), Abc_ObjMapNtkTime(pObj));
   }
   */


//    Abc_NtkForEachNode1( pNtkTopoed, pObj, i ){ 
//        if (i < 1000 || i > 1030) continue;
//        mappingID = Abc_ObjMapNtkId(pObj);
//        fPhase =  Abc_ObjMapNtkPhase(pObj);
//        gateDelay = Abc_ObjMapNtkTime(pObj);
//        pNodeMap = p->vMapObjs->pArray[mappingID];
//        // pNodeMap->bestDelay= gateDelay;
//        // pNodeMap->delay[fPhase] = gateDelay;
//     //    int num = Map_NodeReadNum(pNodeMap);
//     //    printf("%d,",  num);
//        if ( Map_NodeReadCutBest(pNodeMap, fPhase) != NULL ) {
//            pCutBest = Map_NodeReadCutBest(pNodeMap, fPhase);
//            pCutBest->delay[fPhase] = gateDelay;
//            pSuperBest = pCutBest->M[fPhase].pSuperBest;
//            pRoot = Map_SuperReadRoot(pSuperBest);
 
//         //    printf("name(%s), mappingID(%d), phase(%d), delay(%4.4f), ", Mio_GateReadName(pRoot), mappingID, fPhase, gateDelay);
 
//            printf("%24s, ",             Mio_GateReadName(pRoot));
//            printf("%4d, ",              mappingID );
            
//            printf("%4d, ",              fPhase );
//            printf("%4.2f, ",            gateDelay);
//            Abc_Obj_t * pObj2;
//            int k = 0;
//            int maxFaninDegree = 0;
//            Abc_ObjForEachFanin( pObj, pObj2, k ) {
//                if (maxFaninDegree < Abc_ObjFanoutNum(pObj2))
//                    maxFaninDegree = Abc_ObjFanoutNum(pObj2);
//            }
//            printf( "%4d, ",             maxFaninDegree);
//            printf( "%4d, ",             Abc_ObjFanoutNum(pObj) );
//            printf("%6.2f, ",            Abc_MaxFloat(pSuperBest->tDelayLDMax.Rise, pSuperBest->tDelayLDMax.Fall));
//            printf("%6.2f, ",            Abc_MaxFloat(pSuperBest->tDelayPDMax.Rise, pSuperBest->tDelayPDMax.Fall));
//            printf("\n"); 
//        } else {
           
//            // inveter
//            Mio_Gate_t * inveter =  (Mio_Gate_t *)pObj->pData; 
//            printf("%24s, ",               Mio_GateReadName(inveter));
//            printf("%4d, ",              mappingID ); 
//            printf("%4d, ",              fPhase );
//            printf("%6.2f, ",            gateDelay);
//            Abc_Obj_t * pObj2;
//            int k = 0;
//            int maxFaninDegree = 0;
//            Abc_ObjForEachFanin( pObj, pObj2, k ) {
//                if (maxFaninDegree < Abc_ObjFanoutNum(pObj2))
//                    maxFaninDegree = Abc_ObjFanoutNum(pObj2);
//            }
//            printf( "%4d, ",             maxFaninDegree);
//            printf( "%4d, ",             Abc_ObjFanoutNum(pObj) );

//            printf("%6.2f, ",            Abc_MaxFloat(inveter->pPins->dDelayLDRise, inveter->pPins->dDelayLDFall));
//            printf("%6.2f, ",            Abc_MaxFloat(inveter->pPins->dDelayPDRise, inveter->pPins->dDelayPDFall));
//            printf("\n"); 
//        }
//    }
//    printf("number nodes of topo network(%6d), itera num (%6d) \n", Abc_NtkNodeNum(pNtkTopoed), i);
   // LD, PD, faninD, D b
   // 2.4  1.1  -0.01   2.1  -10.9
   // 2.4 * 2.5 + 1.1 * 7.6 - 0.01*6 + 2.1 * 2 -10.9
   // 0.7 * LD * ( 0.9 * D + 0.2 * sqrt(faninD)) + 0.6 * PD + 5



   // 5. update the new delay of the mapped network
   // print the nRef of the mapping network
   
//    printf("the number of fanout of the mapping network \n");
//    Map_Node_t * pNodeM;
//    for ( i = 0; i < p->vMapObjs->nSize; i++ )
//    {
//        pNodeM = p->vMapObjs->pArray[i];
//        if ( Map_NodeIsBuf(pNodeM) )
//            continue;
//        printf("node(%d),  nRefs(%d), nRefAct(%d, %d), nRefEst(%2.2f, %2.2f) \n", Map_NodeReadNum(pNodeM), pNodeM->nRefs, pNodeM->nRefAct[0], pNodeM->nRefAct[1], pNodeM->nRefEst[0], pNodeM->nRefEst[1]);
//    }
   
//
//    //////////////////////////////////////////////////////////////////////
//    // delay-depenpent mapping
//    clk = Abc_Clock();
//    p->fMappingMode = 5;
//    if ( !Map_MappingMatches( p ) )
//        return 0;
//    p->timeMatch = Abc_Clock() - clk;
//    // compute the references and collect the nodes used in the mapping
//    Map_MappingSetRefs( p );
//    p->AreaBase = Map_MappingGetArea( p );
//    if ( p->fVerbose )
//    {
//        printf( "Delay    : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
//                fShowSwitching? "Switch" : "Delay",
//                fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
//                Map_MappingGetAreaFlow(p), p->AreaBase, 0.0 );
//        ABC_PRT( "Time", p->timeMatch );
//    }
//    //////////////////////////////////////////////////////////////////////
//
//
//
//    //////////////////////////////////////////////////////////////////////
//    // delay-depenpent mapping
//    clk = Abc_Clock();
//    p->fMappingMode = 5;
//    if ( !Map_MappingMatches( p ) )
//        return 0;
//    p->timeMatch = Abc_Clock() - clk;
//    // compute the references and collect the nodes used in the mapping
//    Map_MappingSetRefs( p );
//    p->AreaBase = Map_MappingGetArea( p );
//    if ( p->fVerbose )
//    {
//        printf( "Delay    : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
//                fShowSwitching? "Switch" : "Delay",
//                fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
//                Map_MappingGetAreaFlow(p), p->AreaBase, 0.0 );
//        ABC_PRT( "Time", p->timeMatch );
//    }
//    //////////////////////////////////////////////////////////////////////
//
//  //////////////////////////////////////////////////////////////////////
//    // delay-depenpent mapping
//    clk = Abc_Clock();
//    p->fMappingMode = 5;
//    if ( !Map_MappingMatches( p ) )
//        return 0;
//    p->timeMatch = Abc_Clock() - clk;
//    // compute the references and collect the nodes used in the mapping
//    Map_MappingSetRefs( p );
//    p->AreaBase = Map_MappingGetArea( p );
//    if ( p->fVerbose )
//    {
//        printf( "Delay    : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
//                fShowSwitching? "Switch" : "Delay",
//                fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
//                Map_MappingGetAreaFlow(p), p->AreaBase, 0.0 );
//        ABC_PRT( "Time", p->timeMatch );
//    }
//    //////////////////////////////////////////////////////////////////////
//
//
//    //////////////////////////////////////////////////////////////////////
//    // delay-depenpent mapping
//    clk = Abc_Clock();
//    p->fMappingMode = 5;
//    if ( !Map_MappingMatches( p ) )
//        return 0;
//    p->timeMatch = Abc_Clock() - clk;
//    // compute the references and collect the nodes used in the mapping
//    Map_MappingSetRefs( p );
//    p->AreaBase = Map_MappingGetArea( p );
//    if ( p->fVerbose )
//    {
//        printf( "Delay    : %s = %8.2f  Flow = %11.1f  Area = %11.1f  %4.1f %%   ",
//                fShowSwitching? "Switch" : "Delay",
//                fShowSwitching? Map_MappingGetSwitching(p) : p->fRequiredGlo,
//                Map_MappingGetAreaFlow(p), p->AreaBase, 0.0 );
//        ABC_PRT( "Time", p->timeMatch );
//    }
//    //////////////////////////////////////////////////////////////////////


 
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
 
    // parameters for iteration 
    int itera_num = 10;
    int para_size = 10; 
    int rec_y_size = 1;
    p->delayParams = malloc(sizeof(double) * para_size);
    // some good delay parameters for both delay- and area- oriented mapping  
    double goodPara[3][10] = {
        {0.736, 0.144, 0.349, 0.458, 1.025, 0.407, 0.020, 0.889, 1.288, 0.252},
        {0.5, 0.3, 0.1, 0.5, 1.0, 0.3, 0.1, 0.25, 1.0, 0.5},  // Expert Design
        {0.348, 0.061, 0.017, 0.146, 1.832, 0.411, 0.260, 0.050, 1.954, 0.782}, // best result for bar
        // {0.506, 0.070, 0.315, 0.413, 1.800, 0.004, 0.122, 0.118, 0.989, 0.558},
        // {0.581, 0.129, 0.082, 0.251, 0.890, 0.297, 0.099, 0.114, 1.028, 0.546} 

    };
    int good_itera_num = 3; 
      
    // for recording the delay parameters and the corresponding delay
    double * rec_x = (double *)malloc(para_size * sizeof(double));
    double * rec_y = (double *)malloc(rec_y_size * sizeof(double));
    memset(rec_x, 0, para_size * sizeof(double));
    memset(rec_y, 0, rec_y_size * sizeof(double));

    // parameters for recording the best results
    ItResults* itRes = (ItResults*)malloc((itera_num+ good_itera_num) * sizeof(ItResults));
    double min_Y = MAP_FLOAT_LARGE;
    double *min_rec_x = NULL; 
     
    // parameters for iteration parameters
    double firstDelay = 0.0, firstArea = 0.0, firstLevel = 0.0; 
    double curDelay = 0.0;
  
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('/workspaces/abc-itmap/src/map/mapper/')");

    PyObject* pModule = NULL;
    PyObject* pFuncInit = NULL, *pFuncIterate = NULL;
      
    pModule = PyImport_ImportModule("hebo_opt");
    if (!pModule) {
        printf("can't find hebo_opt.py\n");
        Py_Finalize();
        return;
    } 
    pFuncInit = PyObject_GetAttrString(pModule, "init_opt");
    if (!pFuncInit) {
        printf("can't find function init_opt\n");
        Py_DECREF(pModule);
        Py_Finalize();
        return;
    }
    pFuncIterate = PyObject_GetAttrString(pModule, "iterate_opt");
    if (!pFuncIterate) {
        printf("can't find function iterate_opt\n");
        Py_DECREF(pFuncInit);
        Py_DECREF(pModule);
        Py_Finalize();
        return;
    }

    PyObject* pOpt = PyObject_CallObject(pFuncInit, NULL);
    if (!pOpt) {
        printf("init_opt failed to initialize\n");
        Py_DECREF(pFuncIterate);
        Py_DECREF(pFuncInit);
        Py_DECREF(pModule);
        Py_Finalize();
        return;
    }
    
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

        /* area oriented mapping. 
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
        */

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
        rec_y[0] = curDelay/firstDelay + estArea/firstArea;
        printf("+++++curDelay: %f, #####estDepth: %f, -----estArea:%f, record_y:%f, level:%.2f\n", curDelay, estDepth, estArea, rec_y[0], estLevel);    
 
        double* tmpParas = (double*)malloc(para_size * sizeof(double)); 
        memcpy(tmpParas, p->delayParams, para_size * sizeof(double));
        itRes[i].rec_x = tmpParas;
        itRes[i].rec_y = rec_y[0];

        if (itRes[i].rec_y < min_Y) {
            min_Y = itRes[i].rec_y;
            min_rec_x = itRes[i].rec_x;
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


    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    memset(rec_x, 0, para_size * sizeof(double));
    memset(rec_y, 0, rec_y_size * sizeof(double));
    // fit models using GP Kernel 
    for (int i = 0; i <  itera_num; i ++ ) {
        // create args for iterate_opt: 
        // def iterate_opt(opt, i_iter,  given_rec_x : List[float], given_rec_y : List[float]):

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
                            printf("para[%d]=%.3f, ", k, rec_x[k]);
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

        /* area oriented mapping. 
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
        */

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
        double estArea = Map_MappingGetArea( p );
        // double estArea = p->AreaFinal;  

        // if ( i == 0) {
        //     firstDelay = curDelay; 
        //     firstArea = estArea;
        //     firstLevel = Abc_NtkLevel(pNtkTopoed);
        // }
        // double gapDelay = Abc_AbsFloat( estDepth - curDelay)/curDelay;
        double estLevel = Abc_NtkLevel(pNtkTopoed); 
        
        // rec_y[0] = curDelay/firstDelay + estArea/firstArea + (estLevel - firstLevel) * 0.05;
        rec_y[0] = curDelay/firstDelay + estArea/firstArea;
        printf("+++++curDelay: %f, #####estDepth: %f, -----estArea:%f, record_y:%f, level:%.2f\n", curDelay, estDepth, estArea, rec_y[0], estLevel);    

       
        double* tmpParas = (double*)malloc(para_size * sizeof(double)); 
        memcpy(tmpParas, p->delayParams, para_size * sizeof(double));
        itRes[i+good_itera_num].rec_x = tmpParas;
        itRes[i+good_itera_num].rec_y = rec_y[0];

        if (itRes[i+good_itera_num].rec_y < min_Y) {
            min_Y = itRes[i+good_itera_num].rec_y;
            min_rec_x = itRes[i+good_itera_num].rec_x;
        }

        // 4. clean best matches of the mapped network
        if (i  <= itera_num - 1) {
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

    // set parameters for the best iteration
    for (int j = 0; j < para_size; j++){
        p->delayParams[j] = min_rec_x[j];
        printf("best para[%d]=%.3f, ", j, min_rec_x[j]);
        printf("\n");
    } 
        
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

void  Map_MappingUpdateTauRef(Map_Man_t * p, Map_Node_t *pNode, Map_Cut_t *pCut, Map_Super_t *pSuper, int fPhase, double gateDelay, double * grad, double *gatePara){ 
    int max_iterations = 3;
    double lr = 0.05;
    double tolerance = 3; 
    int pi = gatePara[0];

    double cutDelay = Map_MappingEstCutDelay(p, pCut, pNode, fPhase, pi, gatePara);
    double gradDirection = cutDelay - gateDelay > tolerance  ? 1 : - 1;  // TODO:

   
    for (int iteration = 0; iteration < max_iterations; iteration++) { 
        pNode->nRefEst[0] = pNode->nRefEst[0] - 2 * lr * gradDirection * grad[3];
        pNode->nRefEst[1] = pNode->nRefEst[1] - lr * gradDirection * grad[3];
        pNode->nRefEst[2] = pNode->nRefEst[2] - lr * gradDirection * grad[3];  
        pNode->tauRefs[1] = pNode->tauRefs[1] - lr * gradDirection * grad[4];
        pNode->tauRefs[2] = pNode->tauRefs[2] - lr * gradDirection * grad[5];

        pCut->ppLeaves[pi]->nRefEst[0] = pCut->ppLeaves[pi]->nRefEst[0] - 2 * lr * gradDirection * grad[0];
        pCut->ppLeaves[pi]->nRefEst[1] = pCut->ppLeaves[pi]->nRefEst[1] - lr * gradDirection * grad[0];
        pCut->ppLeaves[pi]->nRefEst[2] = pCut->ppLeaves[pi]->nRefEst[2] - lr * gradDirection * grad[0];
        pCut->ppLeaves[pi]->tauRefs[1] = pCut->ppLeaves[pi]->tauRefs[1] - lr * gradDirection * grad[1];
        pCut->ppLeaves[pi]->tauRefs[2] = pCut->ppLeaves[pi]->tauRefs[2] - lr * gradDirection * grad[2];

        cutDelay = Map_MappingEstCutDelay(p, pCut, pNode, fPhase, pi, gatePara);

        if (fabs(cutDelay- gateDelay ) < tolerance) {
            printf("Converged after %d iterations.\n", iteration + 1);
            break;
        }
    }

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

