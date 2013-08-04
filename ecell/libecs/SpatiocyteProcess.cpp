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

#include <libecs/SpatiocyteProcess.hpp>
#include <libecs/SpatiocyteSpecies.hpp>

namespace libecs
{

LIBECS_DM_INIT_STATIC(SpatiocyteProcess, Process);

void SpatiocyteProcess::initializeThird()
{
  for(unsigned i(0); i != theSpecies.size(); ++i)
    {
      if(isDependentOnRemoveMolecule(theSpecies[i]))
        {
          theSpecies[i]->addInterruptRemoveMolecule(this);
        }
      if(isDependentOnAddMolecule(theSpecies[i]))
        {
          theSpecies[i]->addInterruptAddMolecule(this);
        }
      if(isDependentOnEndDiffusion(theSpecies[i]))
        {
          theSpecies[i]->addInterruptEndDiffusion(this);
        }
    }
}

String SpatiocyteProcess::getIDString(Voxel* aVoxel) const
{
  Variable* aVariable(theSpecies[getID(aVoxel)]->getVariable());
  return "["+aVariable->getSystemPath().asString()+":"+
    aVariable->getID()+"]["+int2str(getID(aVoxel))+"]";
}

String SpatiocyteProcess::getIDString(Species* aSpecies) const
{
  Variable* aVariable(aSpecies->getVariable());
  return "["+aVariable->getSystemPath().asString()+":"+
    aVariable->getID()+"]["+int2str(aSpecies->getID())+"]";
}

String SpatiocyteProcess::getIDString(Variable* aVariable) const
{
  return "["+aVariable->getSystemPath().asString()+":"+aVariable->getID()+"]";
}

String SpatiocyteProcess::getIDString(Comp* aComp) const
{
  return "["+aComp->system->getFullID().asString()+"]";
}

String SpatiocyteProcess::getIDString(unsigned int id) const
{
  Variable* aVariable(theSpecies[id]->getVariable());
  return "["+aVariable->getSystemPath().asString()+":"+
    aVariable->getID()+"]["+int2str(id)+"]";
}

int SpatiocyteProcess::getVariableNetCoefficient(const Process* aProcess,
                                               const Variable* aVariable) const
{
  int netCoefficient(0);
  const VariableReferenceVector& aVariableReferences(
                                 aProcess->getVariableReferenceVector()); 
  for(VariableReferenceVector::const_iterator i(aVariableReferences.begin());
      i != aVariableReferences.end(); ++i)
    {
      if((*i).getVariable() == aVariable)
        {
          netCoefficient += (*i).getCoefficient();
        }
    }
  if(netCoefficient)
    {
      const Species* src(theSpatiocyteStepper->variable2species(aVariable));
      for(VariableReferenceVector::const_iterator
          i(aVariableReferences.begin()); i != aVariableReferences.end(); ++i)
        {
          const Species* tar(theSpatiocyteStepper->variable2species(
                                                  (*i).getVariable()));
          if(src && tar && src == tar->getVacantSpecies())
            {
              netCoefficient += (*i).getCoefficient();
            }
        }
    }
  return netCoefficient;
}

}
