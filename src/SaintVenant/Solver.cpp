/***********************************************************************
 * mfcm SaintVenant/Solver.cpp
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

#include "Solver.hpp"

#include "SourceTerms/ManningRoughnessSourceTerm.hpp"
#include "SourceTerms/InfiltrationSourceTerm.hpp"

#include "Boundaries/DischargeBoundarySourceTerm.hpp"
#include "Boundaries/HeadBoundarySourceTerm.hpp"
#include "Boundaries/StageBoundarySourceTerm.hpp"

#include "Measure.hpp"

#include "Output/OutputFile.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
SaintVenantSolver<TT,T,Mesh>::
SaintVenantSolver(const std::shared_ptr<sycl::queue>& queue,
		  size_t no_of_states,
		  const std::shared_ptr<TimeParameters<TT>>& tparams)
  : time_params_(tparams),
    mesh_(std::make_shared<MeshType>(queue, true)),
    constants_(std::make_shared<Constants>(mesh_, true)),
    stage_(mesh_->queue_ptr(), "stage", mesh_, 0.0f, true)
{
  U_.push_back(std::make_shared<State>(mesh_));
  dUdt_.push_back(std::make_shared<State>(0.0, mesh_, "d", "⁄dt"));
  for (size_t i = 1; i < no_of_states; ++i) {
    const State& U0 = *(U_.at(0));
    U_.push_back(std::make_shared<State>(0.0, mesh_, "", std::to_string(i)));
    dUdt_.push_back(std::make_shared<State>(0.0, mesh_, "", std::to_string(i)));
  }

  /*
    CheckFile<Field<ValueType,MeshType,MeshComponent::Cell>> cf("state");
    cf.output({ &(U_[0]->h()), &(U_[0]->u()), &(U_[0]->v()) });
  */
  
  dUdx_ = std::make_shared<State>(0.0, mesh_, "d", "⁄dx");
  dUdy_ = std::make_shared<State>(0.0, mesh_, "d", "⁄dy");
  fluxes_ = std::make_shared<Fluxes>(mesh_, "", "flux");

  for (auto&& st_conf : GlobalConfig::instance().source_term_configurations()) {
    source_terms_.push_back(SaintVenantSourceTerm<TT,T,Mesh>::create_source_term(st_conf, mesh_));
  }

  for (auto&& b_conf : GlobalConfig::instance().boundary_configurations()) {
    boundaries_.push_back(BoundarySourceTerm<TT,T,Mesh>::create_boundary(b_conf, mesh_));
  }

  // Create the measures
  SaintVenantHPointMeasure<TT,T,Mesh>::create_measures(queue, time_params_, mesh_, measures_);
}

template<typename TT,
	 typename T,
	 typename Mesh>
void
SaintVenantSolver<TT,T,Mesh>::update_dUdt(const size_t& state_no,
					  const TT& time_now,
					  const TT& timestep)
{
  // Update the spatial derivatives
  U_.at(state_no)->calculate_spatial_derivatives(*dUdx_, *dUdy_);

  // Calculate the flux at each face
  fluxes_->update(*(U_.at(state_no)), *constants_, *dUdx_, *dUdy_);

  // Calculate the temporal derivative
  using TDKernel = SaintVenantTemporalDerivativeKernel<ValueType,MeshType>;
  size_t ncells = mesh_->template object_count<MeshComponent::Cell>();
  mesh_->queue_ptr()->submit([&] (sycl::handler& cgh) {
    auto kernel = TDKernel(cgh, *(U_.at(state_no)), *constants_,
			   *dUdx_, *dUdy_, *fluxes_,
			   *(dUdt_.at(state_no)), time_now, timestep);
    cgh.parallel_for(sycl::range<1>(ncells), kernel);
  });

  // Apply source terms
  for (auto&& st : source_terms_) {
    st->apply(*(U_.at(state_no)),
	      *(constants_),
		*(dUdx_), *(dUdy_),
	      *(dUdt_.at(state_no)),
	      timestep, time_now, time_params_);
  }

  // Apply boundary condition terms
  for (auto&& bdy : boundaries_) {
    bdy->apply(*(U_.at(state_no)),
	       *(constants_),
		*(dUdx_), *(dUdy_),
	       *(dUdt_.at(state_no)),
	       timestep, time_now, time_params_);
  }
}

template<typename TT,
	 typename T,
	 typename Mesh>
void SaintVenantSolver<TT,T,Mesh>::
start_new_step(const TT& time_now,
	       const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr)
{
  for (auto&& st : source_terms_) {
    st->start_new_step(*(constants_), time_now, tp_ptr);
  }
  for (auto&& bdy : boundaries_) {
    bdy->start_new_step(*(constants_), time_now, tp_ptr);
  }
}
