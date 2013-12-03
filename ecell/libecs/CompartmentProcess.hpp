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


#ifndef __CompartmentProcess_hpp
#define __CompartmentProcess_hpp

#include <sstream>
#include <libecs/SpatiocyteProcess.hpp>
#include <libecs/SpatiocyteSpecies.hpp>

LIBECS_DM_CLASS(CompartmentProcess, SpatiocyteProcess)
{ 
public:
  LIBECS_DM_OBJECT(CompartmentProcess, Process)
    {
      INHERIT_PROPERTIES(Process);
      PROPERTYSLOT_SET_GET(Integer, Autofit);
      PROPERTYSLOT_SET_GET(Integer, Filaments);
      PROPERTYSLOT_SET_GET(Integer, Periodic);
      PROPERTYSLOT_SET_GET(Integer, RegularLattice);
      PROPERTYSLOT_SET_GET(Integer, Subunits);
      PROPERTYSLOT_SET_GET(Integer, SurfaceDirection);
      PROPERTYSLOT_SET_GET(Real, DiffuseRadius); //off-lattice voxel radius
      PROPERTYSLOT_SET_GET(Real, Length);
      PROPERTYSLOT_SET_GET(Real, LipidRadius); //radius of lipid voxels
      PROPERTYSLOT_SET_GET(Real, OriginX);
      PROPERTYSLOT_SET_GET(Real, OriginY);
      PROPERTYSLOT_SET_GET(Real, OriginZ);
      PROPERTYSLOT_SET_GET(Real, RotateX);
      PROPERTYSLOT_SET_GET(Real, RotateY);
      PROPERTYSLOT_SET_GET(Real, RotateZ);
      PROPERTYSLOT_SET_GET(Real, SubunitAngle);
      //SubunitRadius is the actual radius of a molecule.
      //For a normal molecule the SubunitRadius = DiffuseRadius.
      //For a multiscale molecule, the SubunitRadius can be larger
      //than the DiffuseRadius:
      PROPERTYSLOT_SET_GET(Real, SubunitRadius);
      PROPERTYSLOT_SET_GET(Real, Width);
    }
  CompartmentProcess():
    isCompartmentalized(false),
    Autofit(1),
    Filaments(1),
    LipidCols(0),
    LipidRows(0),
    Periodic(0),
    RegularLattice(1),
    Subunits(1),
    SurfaceDirection(2),
    theDiffuseSize(6),
    theDimension(1),
    DiffuseRadius(0),
    Length(0),
    LipidRadius(0),
    nVoxelRadius(0.5),
    OriginX(0),
    OriginY(0),
    OriginZ(0),
    RotateX(0),
    RotateY(0),
    RotateZ(0),
    SubunitAngle(0),
    SubunitRadius(0),
    Width(0),
    theLipidSpecies(NULL),
    theVacantSpecies(NULL) {}
  virtual ~CompartmentProcess() {}
  SIMPLE_SET_GET_METHOD(Integer, Autofit);
  SIMPLE_SET_GET_METHOD(Integer, Filaments);
  SIMPLE_SET_GET_METHOD(Integer, Periodic);
  SIMPLE_SET_GET_METHOD(Integer, RegularLattice);
  SIMPLE_SET_GET_METHOD(Integer, Subunits);
  SIMPLE_SET_GET_METHOD(Integer, SurfaceDirection);
  SIMPLE_SET_GET_METHOD(Real, DiffuseRadius);
  SIMPLE_SET_GET_METHOD(Real, Length);
  SIMPLE_SET_GET_METHOD(Real, LipidRadius);
  SIMPLE_SET_GET_METHOD(Real, OriginX);
  SIMPLE_SET_GET_METHOD(Real, OriginY);
  SIMPLE_SET_GET_METHOD(Real, OriginZ);
  SIMPLE_SET_GET_METHOD(Real, RotateX);
  SIMPLE_SET_GET_METHOD(Real, RotateY);
  SIMPLE_SET_GET_METHOD(Real, RotateZ);
  SIMPLE_SET_GET_METHOD(Real, SubunitAngle);
  SIMPLE_SET_GET_METHOD(Real, SubunitRadius);
  SIMPLE_SET_GET_METHOD(Real, Width);
  virtual void prepreinitialize();
  virtual void initialize();
  virtual void initializeFirst();
  virtual unsigned getLatticeResizeCoord(unsigned);
  virtual void initializeThird();
  virtual void printParameters();
  virtual void updateResizedLattice();
  virtual void setCompartmentDimension();
  virtual void initializeVectors();
  virtual void connectFilaments(unsigned, unsigned, unsigned);
  virtual void setSubunitStart();
  virtual void elongateFilaments(Species*, unsigned, unsigned, unsigned,
                                 double);
  virtual void initializeFilaments(Point&, unsigned, unsigned, double, Species*,
                                   unsigned);
  virtual void addPlaneIntersectInterfaceVoxel(Voxel&, Point&);
  virtual bool isInside(Point&);
  virtual bool isOnAboveSurface(Point&);
  void connectSubunit(unsigned, unsigned, unsigned, unsigned);
  void addInterfaceVoxel(unsigned, unsigned, const double);
  void addInterfaceVoxel(Voxel&, Point&);
  void setVacantCompSpeciesProperties();
  void setLipidCompSpeciesProperties();
  void setDiffuseSize(unsigned, unsigned);
  void interfaceSubunits();
  void enlistSubunitIntersectInterfaceVoxels();
  void enlistOrphanSubunitInterfaceVoxels();
  void enlistPlaneIntersectInterfaceVoxels();
  void enlistSubunitInterfaceAdjoins();
  void rotate(Point&);
  void addAdjoin(Voxel&, unsigned);
  void setSpeciesIntersectLipids();
  void getStartVoxelPoint(Point&, Point&, Point&);
  void setAdjoinOffsets();
  int getCoefficient(Species*);
  void allocateGrid();
  void setGrid(Species*, std::vector<std::vector<unsigned> >&, unsigned);
  bool setSubunitInterfaceVoxels(const unsigned, const double, 
                                 const bool isSingle=false);
  bool isCorrectSide(const unsigned);
  Species* coefficient2species(int);
  Voxel* addCompVoxel(unsigned, unsigned, Point&, Species*, unsigned, unsigned);
protected:
  bool isCompartmentalized;
  unsigned Autofit;
  unsigned endCoord;
  unsigned Filaments;
  unsigned intStartIndex;
  unsigned LipidCols;
  unsigned LipidRows;
  unsigned lipStartCoord;
  unsigned Periodic;
  unsigned subStartCoord;
  unsigned RegularLattice;
  unsigned Subunits;
  unsigned SurfaceDirection;
  unsigned theDiffuseSize;
  unsigned theDimension;
  unsigned vacStartIndex;
  int gridCols;
  int gridLayers;
  int gridRows;
  double DiffuseRadius;
  double nGridSize;
  double Height;
  double nHeight;
  double Length;
  double lengthDisplace;
  double lengthDisplaceOpp;
  double LipidRadius;
  double nDiffuseRadius;
  double nLength;
  double nLipidRadius;
  double nParentHeight;
  double nParentLength;
  double nParentWidth;
  double nSubunitRadius;
  double nVoxelRadius;
  double nWidth;
  double OriginX;
  double OriginY;
  double OriginZ;
  double RotateX;
  double RotateY;
  double RotateZ;
  double SubunitAngle;
  double SubunitRadius;
  double surfaceDisplace;
  double VoxelRadius;
  double Width;
  double widthDisplace;
  double widthDisplaceOpp;
  Comp* theComp;
  Point heightEnd;
  Point heightVector;
  Point lengthEnd;
  Point lengthStart;
  Point lengthVector;
  Point lipidStart;
  Point Origin;
  Point subunitStart;
  Point surfaceNormal;
  Point parentOrigin;
  Point widthEnd;
  Point widthVector;
  Species* theLipidSpecies;
  Species* theInterfaceSpecies;
  Species* theVacantSpecies;
  Variable* theInterfaceVariable;
  Variable* theLipidVariable;
  Variable* theVacantVariable;
  std::vector<std::vector<unsigned> > theVacGrid;
  std::vector<std::vector<unsigned> > theLipGrid;
  std::vector<Point> thePoints;
  std::vector<Species*> theLipidCompSpecies;
  std::vector<Species*> theVacantCompSpecies;
  std::vector<std::vector<unsigned> > subunitInterfaces;
  std::vector<std::vector<int> > theAdjoinOffsets;
  std::vector<int> theRowOffsets;
};

#endif /* __CompartmentProcess_hpp */




