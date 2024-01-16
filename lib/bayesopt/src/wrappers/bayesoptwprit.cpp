/*
-----------------------------------------------------------------------------
   This file is part of BayesOptimization, an efficient C++ library for 
   Bayesian optimization.

   Copyright (C) 2011-2015 Ruben Martinez-Cantin <rmcantin@unizar.es>
 
   BayesOptimization is free software: you can redistribute it and/or modify
   it under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BayesOptimization is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Affero General Public License for more details.

   You should have received a copy of the GNU Affero General Public License
   along with BayesOptimization.  If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------
*/

#include "bayesopt/bayesopt.h"
#include "bayesopt/bayesopt.hpp"     
#include "posteriormodel.hpp" 

#include "log.hpp"
#include "ublas_extra.hpp"
#include "specialtypes.hpp"

static const int BAYESOPT_FAILURE = -1; /* generic failure code */
static const int BAYESOPT_INVALID_ARGS = -2;
static const int BAYESOPT_OUT_OF_MEMORY = -3;
static const int BAYESOPT_RUNTIME_ERROR = -4;

/**
 * \brief Version of ContinuousModel for the C wrapper
 */
class CContinuousModelIt: public bayesopt::ContinuousModelIt
{
 public:

  CContinuousModelIt(size_t dim, bopt_params params):
    ContinuousModelIt(dim,params)  { }; 

  virtual ~CContinuousModelIt(){};
 
};

void * initializeOptimizationIt(int nDim, const double *lb, const double *ub, int samplesize, const double * xpoints, const double * ypoints, bopt_params params) {
  vectord result(nDim);
  vectord lowerBound = bayesopt::utils::array2vector(lb,nDim); 
  vectord upperBound = bayesopt::utils::array2vector(ub,nDim); 
  matrixd xPoints = bayesopt::utils::array2matrix(xpoints, samplesize, nDim);
  vectord yPoints = bayesopt::utils::array2vector(ub,samplesize); 
 
  CContinuousModelIt *optimizer;
  try {
    optimizer = new CContinuousModelIt(nDim, params);
    }
  catch (std::bad_alloc& e)
    {
     printf ("BAYESOPT_OUT_OF_MEMORY \n");
     return nullptr;  
    }
  catch (std::invalid_argument& e)
    {
        printf("BAYESOPT_INVALID_ARGS\n"); 
      
      return nullptr;
    }
  catch (std::runtime_error& e)
    { 
      printf("BAYESOPT_RUNTIME_ERROR\n");
      return nullptr;
    }
  catch (...)
    { 
        printf("Unknown error\n");
      
      return nullptr; 
    }
    optimizer->setBoundingBox(lowerBound,upperBound);
    optimizer->initializeOptimization(xPoints, yPoints); 
    return static_cast<void*>(optimizer);
} 

void nextPointIt (void * optimizer, double *xnext ) {
    if (optimizer == nullptr) {
        printf ("Error: optimizer is null pointer!");
        return; 
    }
    CContinuousModelIt* opt = static_cast<CContinuousModelIt*>(optimizer);
    vectord xNext = opt -> nextPoint();

    std::copy(xNext.begin(), xNext.end(), xnext);
}
 
void addSampleIt (void * optimizer, int nDim, const double * xnext, const double  ynext,  bopt_params params, int mCurrentIter) {
    vectord xNext = bayesopt::utils::array2vector(xnext,nDim); 
    double yNext = ynext; 
    CContinuousModelIt* opt = static_cast<CContinuousModelIt*>(optimizer); 
    opt->mModel->addSample(xNext,yNext);  
    bool retrain = (params.n_iter_relearn > 0) && ((mCurrentIter + 1) % params.n_iter_relearn == 0);
    if (retrain) {
        opt->mModel->updateHyperParameters();
        opt->mModel->fitSurrogateModel();  
    } else {
        opt-> mModel->updateSurrogateModel();
    }
    opt-> mModel->updateCriteria(xNext); 
}

 