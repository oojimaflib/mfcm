/***********************************************************************
 * mfcm SaintVenant/Boundaries/BoundarySourceTerm.cpp
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

#include "BoundarySourceTerm.hpp"

#include "DischargeBoundarySourceTerm.cpp"
#include "HeadBoundarySourceTerm.cpp"
#include "StageBoundarySourceTerm.cpp"

template<typename TT,
	 typename T,
	 typename Mesh>
std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
BoundarySourceTerm<TT,T,Mesh>::
create_boundary(const Config& conf,
		const std::shared_ptr<MeshType>& mesh,
		bool on_device)
{
  std::string btype = conf.get_value<std::string>();
  if (btype == "discharge") {
    return DischargeBoundarySourceTerm<TT,T,Mesh>::create_boundary(conf, mesh, on_device);
  } else if (btype == "head") {
    return HeadBoundarySourceTerm<TT,T,Mesh>::create_boundary(conf, mesh, on_device);
  } else if (btype == "stage") {
    return StageBoundarySourceTerm<TT,T,Mesh>::create_boundary(conf, mesh, on_device);
  } else {
    std::cerr << "ERROR: Unknown boundary type: " << std::quoted(btype)
	      << std::endl;
    throw std::runtime_error("Unknown boundary type.");
  }
}

