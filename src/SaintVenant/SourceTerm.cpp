/***********************************************************************
 * mfcm SaintVenant/SourceTerm.cpp
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

#include "SourceTerm.hpp"

#include "Boundaries/BoundarySourceTerm.cpp"
#include "SourceTerms/ManningRoughnessSourceTerm.cpp"
#include "SourceTerms/EnergyLossSourceTerm.cpp"
#include "SourceTerms/EddyViscositySourceTerm.cpp"
#include "SourceTerms/InfiltrationSourceTerm.cpp"

template<typename TT,
	 typename T,
	 typename Mesh>
std::shared_ptr<SaintVenantSourceTerm<TT,T,Mesh>>
SaintVenantSourceTerm<TT,T,Mesh>::
create_source_term(const Config& conf,
		   const std::shared_ptr<MeshType>& mesh,
		   bool on_device)
{
  std::string st_type = conf.get_value<std::string>();
  if (st_type == "manning roughness") {
    return ManningRoughnessSourceTerm<TT,T,Mesh>::create_source_term(conf, mesh, on_device);
  } else if (st_type == "energy loss") {
    return EnergyLossSourceTerm<TT,T,Mesh>::create_source_term(conf, mesh, on_device);
  } else if (st_type == "eddy viscosity") {
    return EddyViscositySourceTerm<TT,T,Mesh>::create_source_term(conf, mesh, on_device);
  } else if (st_type == "infiltration") {
    return InfiltrationSourceTerm<TT,T,Mesh>::create_source_term(conf, mesh, on_device);
  } else {
    std::cerr << "ERROR: Unknown source term type: " << std::quoted(st_type)
	      << std::endl;
    throw std::runtime_error("Unknown source term type.");
  }
}
