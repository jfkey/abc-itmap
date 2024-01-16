/*
-------------------------------------------------------------------------
   This file is part of BayesOpt, an efficient C++ library for 
   Bayesian optimization.

   Copyright (C) 2011-2015 Ruben Martinez-Cantin <rmcantin@unizar.es>
 
   BayesOpt is free software: you can redistribute it and/or modify it 
   under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BayesOpt is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with BayesOpt.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------
*/

#include "testfunctions.hpp"
#include "bayesopt/bayesopt.hpp"
#include <ctime>
#include <fstream>
#include "param_loader.hpp"
#include <cmath>
#include "bayesopt/parameters.hpp"
#include "posteriormodel.hpp"
#include "bayesopt/bayesopt.h"


// class MyBayesOpt {
//     // to init
// public:
// //     MyBayesOpt(bayesopt::Parameters par, size_t _mDims) : mParameters(par), mDims(_mDims){
// //         if (mParameters.random_seed < 0) mParameters.random_seed = std::time(0); 
// //         mEngine.seed(mParameters.random_seed);
// //     }
//     MyBayesOpt( ){
//     }
//     ~MyBayesOpt() { }

// public:
//     bayesopt::Parameters mParameters;                    ///< Configuration parameters
//     size_t mDims;                                   ///< Number of dimensions
//     size_t mCurrentIter;                        ///< Current iteration number
//     boost::mt19937 mEngine;                      ///< Random number generator
    
//     boost::scoped_ptr<bayesopt::PosteriorModel> mModel;
//     double mYPrev;
//     size_t mCounterStuck;    

// public:
//     void initializeOptimization()  {
//         // Posterior surrogate model
//         mModel.reset(bayesopt::PosteriorModel::create(mDims,mParameters,mEngine));        
//   } 
// }; 

// inline vectord samplePoint(boost::mt19937 &mEngine, int mDims) {	    
//     randFloat drawSample(mEngine,realUniformDist(0,1));
//     vectord Xnext(mDims);    
//     for(vectord::iterator x = Xnext.begin(); x != Xnext.end(); ++x) {	
// 	    *x = drawSample(); 
//     }
//     return Xnext;
// };
    
// inline void initcOptimizer(boost::scoped_ptr<bayesopt::NLOPT_Optimization> cOptimizer){
//     // cOptimizer.reset(new NLOPT_Optimization(mCallback.get(),dim));
//     cOptimizer->setAlgorithm(bayesopt::COMBINED);
//     cOptimizer->setMaxEvals(parameters.n_inner_iterations);
// };

// inline findOptimal(vectord &xOpt, boost::mt19937 &mEngine) { 

//     double minf = cOptimizer->run(xOpt);

//     //Let's try some local exploration like spearmint
//     randNFloat drawSample(mEngine,normalDist(0,0.001));
//     for(size_t ii = 0;ii<5; ++ii)
//       {
// 	vectord pert = getPointAtMinimum();
// 	for(size_t j=0; j<xOpt.size(); ++j)
// 	  {
// 	    pert(j) += drawSample();
// 	  }
// 	try
// 	  {
// 	    double minf2 = cOptimizer->localTrialAround(pert);	    
// 	    if (minf2<minf) 
// 	      {
// 		minf = minf2;
// 		FILE_LOG(logDEBUG) << "Local beats Global";
// 		xOpt = pert;
// 	      }
// 	  }
// 	catch(std::invalid_argument& e)
// 	  {
// 	    //We ignore this one
// 	  }
//       }
//   };



// inline vectord nextPoint(bayesopt::Parameters &mParameters, boost::mt19937 &mEngine, int mDims, boost::scoped_ptr<bayesopt::PosteriorModel> &mModel){
//     //Epsilon-Greedy exploration (see Bull 2011)
//     if ((mParameters.epsilon > 0.0) && (mParameters.epsilon < 1.0)){
//         randFloat drawSample(mEngine,realUniformDist(0,1));
//         double result = drawSample();
//         FILE_LOG(logINFO) << "Trying random jump with prob:" << result;
//         if (mParameters.epsilon > result){
//             FILE_LOG(logINFO) << "Epsilon-greedy random query!";
//             return samplePoint();
//         }
//     }

//     vectord Xnext(mDims);    

//     // GP-Hedge and related algorithms
//     if (mModel->criteriaRequiresComparison()) {
//         bool changed = true; 
// 	    mModel->setFirstCriterium();
// 	    while (changed) {
//             findOptimal(Xnext);
//             changed = mModel->setNextCriterium(Xnext);
// 	    }
//         std::string name = mModel->getBestCriteria(Xnext);
//         FILE_LOG(logINFO) << name << " was selected.";
//     }
//     else  // Standard "Bayesian optimization" {
// 	    FILE_LOG(logDEBUG) << "------ Optimizing criteria ------";
// 	    findOptimal(Xnext);
//     }
//     return Xnext;
// };

// inline double sqr( double x ){ return x*x; };
 
// int main(int nargs, char *args[])
// {
//     bayesopt::Parameters par;
//     par = initialize_parameters_to_default();
//     par.n_iterations = 190;
//     //  par.n_iter_relearn = 0;
//     par.random_seed = 0;
//     par.verbose_level = 1;
//     par.noise = 1e-10;
    
    
//     // init the model 
//     size_t mDims = 2;
//     bayesopt::ContinuousModelIt mybayes(mDims, par);

//     size_t nSamples = 1;
//     matrixd xPoints(nSamples,mDims);
//     vectord yPoints(nSamples,0);
//     int mCounterStuck = 0, mCurrentIter = 0;
   
//     double x = 0.6, y = 0.8, braninValue = 0; 
//     const double pi = boost::math::constants::pi<double>();
//     const double rpi = pi*pi;


//     xPoints(0,0) = 0.535411;
//     xPoints(0,1) = 0.0912871;
    
//     vectord t = mybayes.remapPoint(row(xPoints,0));
//     x = t(0)* 15 - 5; y = t(1) * 15; 
//     braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     yPoints(0) = braninValue;
//     std::cout << braninValue << std::endl;
  
//     // xPoints(1,0) = 0.0615618;
//     // xPoints(1,1) = 0.835183;
//     // t = mybayes.remapPoint(row(xPoints,1));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 
//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(1) = braninValue;
    

//     // xPoints(2,0) = 0.756241;
//     // xPoints(2,1) = 0.597978;
//     //  t = mybayes.remapPoint(row(xPoints,2));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 
     
//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(2) = braninValue;
    
//     // xPoints(3,0) = 0.870247;
//     // xPoints(3,1) = 0.663176;
//     //  t = mybayes.remapPoint(row(xPoints,3));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 
//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(3) = braninValue;

//     // xPoints(4,0) = 0.210823;
//     // xPoints(4,1) = 0.216738;
//     //  t = mybayes.remapPoint(row(xPoints,4));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 
//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(4) = braninValue;
 
//     // xPoints(5,0) = 0.494329;
//     // xPoints(5,1) = 0.404284;
//     //  t = mybayes.remapPoint(row(xPoints,5));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 
//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(5) = braninValue;

//     // xPoints(6,0) = 0.903634;
//     // xPoints(6,1) = 0.922184;
//     //  t = mybayes.remapPoint(row(xPoints,6));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 
//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(6) = braninValue;

//     // xPoints(7,0) = 0.172734;
//     // xPoints(7,1) = 0.185965;
//     //      t = mybayes.remapPoint(row(xPoints,7));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 

//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(7) = braninValue;

//     // xPoints(8,0) = 0.661656;
//     // xPoints(8,1) = 0.712999;
//     //  t = mybayes.remapPoint(row(xPoints,8));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 
//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(8) = braninValue;

//     // xPoints(9,0) = 0.352233;
//     // xPoints(9,1) = 0.312991;
//     // t = mybayes.remapPoint(row(xPoints,9));
//     // x = t(0)* 15 - 5; y = t(1) * 15; 

//     // braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//     // yPoints(9) = braninValue;

    
//     mybayes.initializeOptimization(xPoints, yPoints);

//     bool isfirst = false; 
//     for (size_t ii = 0; ii < par.n_iterations; ++ii){      
        
//             vectord xNext = mybayes.nextPoint();
//             x = xNext(0) * 15 - 5; y = xNext(1) * 15;
//             double yNext  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
//             std::cout << "x:" << x << "y:" << y << "predict:" << yNext << std::endl; 
//             if (par.force_jump) {
//                 if (std::pow( mybayes.mYPrev - yNext,2) < par.noise) {
//                     mCounterStuck++;
//                     std::cout << "Stuck for "<< mCounterStuck << " steps" << std::endl;
//                 }
//                 else {
//                     mCounterStuck = 0;
//                 }

//                 mybayes.mYPrev = yNext;

//                 if (mCounterStuck > par.force_jump) {
//                     std::cout << "Forced random query!";
//                     xNext = mybayes.samplePoint();
//                     x = xNext(0) * 15 - 5; y = xNext(1) * 15;
//                     double yNext  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10; 
//                     mCounterStuck = 0;
//                 }
//             } 
//             mybayes.mModel->addSample(xNext,yNext);
//             bool retrain = ((par.n_iter_relearn > 0) && ((mCurrentIter + 1) % par.n_iter_relearn == 0));
//             if (retrain) {
//               mybayes.mModel->updateHyperParameters();
//               mybayes.mModel->fitSurrogateModel();  
//             } else {
//               mybayes.mModel->updateSurrogateModel();
//             }
//             mybayes.mModel->updateCriteria(xNext);
 

//         mCurrentIter +=1; 
    
//     }

//     vectord result = mybayes.getFinalResult();
//     std::cout << "Result parameters: " << result[0] << ","  << result[1] << std::endl;
//     x = result[0]; y = result[1];
//     braninValue  =  sqr(y-(5.1/(4*rpi))*sqr(x)+5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
    
//     std::cout << "Result " << braninValue << std::endl;
             
//     return 0;
// }

inline double branin(double x, double y)  {
    x = x * 15 - 5;
    y = y * 15;
    const double pi = 3.14;
    const double rpi = pi*pi;
    return (y-(5.1/(4*rpi))*(x)*(x) + 5*x/pi-6) * (y-(5.1/(4*rpi))*(x)*(x) + 5*x/pi-6)+10*(1-1/(8*pi))*cos(x)+10;
};


int main(int nargs, char *args[]) {
    bopt_params params = initialize_parameters_to_default();
    
    params.n_iterations = 190;
    params.random_seed = 0;
    params.verbose_level = 1;
    params.noise = 1e-10;

    int nDim = 2; 
    double lb[2] = {0.0, 0.0};  
    double ub[2] = {1.0, 1.0};  
    double xpoints[2] = {0.535411, 0.0912871}; 
    double ypoints[1]  = {branin(xpoints[0], xpoints[1])};
    int samplesize = 1;

    void * bayesopt = initializeOptimizationIt(nDim, lb, ub, samplesize, xpoints, ypoints, params);
    for (int i = 0; i < 10; i ++) {
        double xnext[2];
        nextPointIt(bayesopt, xnext);
        double ynext = branin(xnext[0], xnext[1]);
        printf("xnext: %f, %f, ynext: %f\n", xnext[0], xnext[1], ynext);
        addSampleIt(bayesopt, nDim, xnext,  ynext, params, i);
    } 
    return 0; 
}