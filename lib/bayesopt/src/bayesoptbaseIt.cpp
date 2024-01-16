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

#include <ctime>
#include "bayesopt/bayesoptbase.hpp"
#include "bayesopt/parameters.hpp"

#include "log.hpp"
#include "posteriormodel.hpp"
#include "specialtypes.hpp"
#include "bopt_state.hpp"

 


namespace bayesopt
{
  BayesOptBaseIt::BayesOptBaseIt(size_t dim, Parameters parameters):
    mParameters(parameters), mDims(dim)
  {
    // Random seed
    if (mParameters.random_seed < 0) mParameters.random_seed = std::time(0); 
    mEngine.seed(mParameters.random_seed);
    
    // Setting verbose stuff (files, levels, etc.)
    int verbose = mParameters.verbose_level;
    if (verbose>=3)
      {
	FILE* log_fd = fopen( mParameters.log_filename.c_str() , "w" );
	Output2FILE::Stream() = log_fd; 
	verbose -= 3;
      }

    switch(verbose)
      {
      case 0: FILELog::ReportingLevel() = logWARNING; break;
      case 1: FILELog::ReportingLevel() = logINFO; break;
      case 2: FILELog::ReportingLevel() = logDEBUG; break;
      default:
	FILELog::ReportingLevel() = logERROR; break;
      }
  }

  BayesOptBaseIt::~BayesOptBaseIt()
  { } // Default destructor

  
  void BayesOptBaseIt::initializeOptimization(matrixd &xPoints, vectord &yPoints)
  {
    // Posterior surrogate model
    mModel.reset(PosteriorModel::create(mDims,mParameters,mEngine));
    
    mModel->setSamples(xPoints);
    mModel->setSamples(yPoints);
    int nums = yPoints.size(); 
    mModel->updateHyperParameters();
    mModel->fitSurrogateModel();
    mYPrev = 0.0;
  }
  

  vectord BayesOptBaseIt::getFinalResult()
  {
    return remapPoint(getPointAtMinimum());
  }
  
  // GETTERS AND SETTERS
  // Potential inline functions. Moved here to simplify API and header
  // structure.
  ProbabilityDistribution* BayesOptBaseIt::getPrediction(const vectord& query)
  { return mModel->getPrediction(query); };
  
  const Dataset* BayesOptBaseIt::getData()
  { return mModel->getData(); };

  Parameters* BayesOptBaseIt::getParameters() 
  {return &mParameters;};

  double BayesOptBaseIt::getValueAtMinimum()
  { return mModel->getValueAtMinimum(); };

  double BayesOptBaseIt::evaluateCriteria(const vectord& query)
  {
    if (checkReachability(query)) return mModel->evaluateCriteria(query);
    else return 0.0;
  }
 

  // PROTECTED
  vectord BayesOptBaseIt::getPointAtMinimum() 
  { return mModel->getPointAtMinimum(); };

    

  // PRIVATE MEMBERS
  vectord BayesOptBaseIt::nextPoint()
  {
    //Epsilon-Greedy exploration (see Bull 2011)
    if ((mParameters.epsilon > 0.0) && (mParameters.epsilon < 1.0)) {
	    randFloat drawSample(mEngine,realUniformDist(0,1));
	    double result = drawSample();
	    FILE_LOG(logINFO) << "Trying random jump with prob:" << result;
	    if (mParameters.epsilon > result) {
	      FILE_LOG(logINFO) << "Epsilon-greedy random query!";
	      return samplePoint();
	    }
    }
     
    vectord Xnext(mDims);     
    // GP-Hedge and related algorithms
    if (mModel->criteriaRequiresComparison()) {
	    bool changed = true;
    	mModel->setFirstCriterium();
	    while (changed) {
	      findOptimal(Xnext);
	      changed = mModel->setNextCriterium(Xnext);
	    }
	    std::string name = mModel->getBestCriteria(Xnext);
	    FILE_LOG(logINFO) << name << " was selected.";
    } else{// Standard "Bayesian optimization"
	    FILE_LOG(logDEBUG) << "------ Optimizing criteria ------";
	    findOptimal(Xnext);
    } 
    return Xnext;
  }
  
} //namespace bayesopt

