/***********************************************************************
 * mfcm SaintVenant/SourceTerm.hpp
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

#ifndef mfcm_SaintVenant_SourceTerm_hpp
#define mfcm_SaintVenant_SourceTerm_hpp

#include "TimeParameters.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
class SaintVenantSourceTerm
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;

  using State = SaintVenantState<ValueType,MeshType>;
  using Constants = SaintVenantConstants<ValueType,MeshType>;
  using Fluxes = SaintVenantFluxes<ValueType,MeshType>;
  
  SaintVenantSourceTerm(void)
  {}

  virtual ~SaintVenantSourceTerm(void)
  {}

  virtual void apply(State& U, Constants& constants, State& dUdt,
		     const TimeType& timestep, const TimeType& time_now,
		     const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr) = 0;

  virtual void start_new_step(Constants& constants,
			      const TimeType& time_now,
			      const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
  {
    // Do nothing by default
  }
  
};

#endif
