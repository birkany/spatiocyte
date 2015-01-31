//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//        This file is part of E-Cell Simulation Environment package
//
//                Copyright (C) 2006-2009 Keio University
//
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//
//
// E-Cell is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// E-Cell is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with E-Cell -- see the file COPYING.
// If not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
//END_HEADER
//
// written by Satya Arjunan <satya.arjunan@gmail.com>
// E-Cell Project, Institute for Advanced Biosciences, Keio University.
//


#ifndef __PeriodicBoundaryDiffusionProcess_hpp
#define __PeriodicBoundaryDiffusionProcess_hpp

#include <DiffusionProcess.hpp>

namespace libecs
{

LIBECS_DM_CLASS(PeriodicBoundaryDiffusionProcess, DiffusionProcess)
{ 
public:
  LIBECS_DM_OBJECT(PeriodicBoundaryDiffusionProcess, Process)
    {
      INHERIT_PROPERTIES(DiffusionProcess);
    }
  PeriodicBoundaryDiffusionProcess() {}
  virtual ~PeriodicBoundaryDiffusionProcess() {}
  virtual void initialize()
    {
      if(isInitialized)
        {
          return;
        }
      DiffusionProcess::initialize();
      theSpatiocyteStepper->setPeriodicEdge();
    }
  void initializeFourth()
    {
      DiffusionProcess::initializeFourth();
      //For PeriodicBoundaryDiffusionProcess, make sure Origins = 0 (default).
      //isOrigins of SpatiocyteSpecies should only be true when using
      //CompartmentProcess to set up compartment, and in that case use
      //DifFusionProcess with Origins = 1. PeriodicBoundaryDiffusionProcess
      //should not be used with CompartmentProcess. CompartmentProcess
      //is the newer process with built in periodic boundary.
      theDiffusionSpecies->resetMoleculeOrigins();
    }
  virtual void fire()
    {
      DiffusionProcess::fire();
      theDiffusionSpecies->relocateBoundaryMolecules();
    }
};

}

#endif /* __PeriodicBoundaryDiffusionProcess_hpp */
