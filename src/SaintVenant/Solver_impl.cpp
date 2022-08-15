/***********************************************************************
 * mfcm SaintVenant/Solver_impl.cpp
 *
 * Copyright (C) Edenvale Young Associates 2022
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************/

#if __INCLUDE_LEVEL__
#error "This file should not be included."
#endif

#include "Solver.cpp"
#include "State.cpp"
#include "Constants.cpp"
#include "Fluxes.cpp"

#include "FluxKernel.cpp"
#include "TemporalDerivativeKernel.cpp"

#include "SourceTerm.cpp"

#include "Boundaries/DischargeBoundarySourceTerm.cpp"
#include "Boundaries/HeadBoundarySourceTerm.cpp"
#include "Boundaries/StageBoundarySourceTerm.cpp"

#include "Cartesian2DMesh.hpp"

template class SaintVenantSolver<float,float,Cartesian2DMesh>;
template class SaintVenantSolver<double,float,Cartesian2DMesh>;
template class SaintVenantState<float,Cartesian2DMesh>;
