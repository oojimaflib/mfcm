/***********************************************************************
 * mfcm SaintVenant/Solver.hpp
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

#ifndef mfcm_SaintVenant_Solver_hpp
#define mfcm_SaintVenant_Solver_hpp

#include "Constants.hpp"
#include "State.hpp"
#include "Fluxes.hpp"

#include "SourceTerm.hpp"
#include "Measure.hpp"
#include "TemporalDerivativeKernel.hpp"

template<typename TT,
	 typename T,
	 typename Mesh>
class SaintVenantSolver
{
public:

  using TimeType = TT;
  using ValueType = T;
  using MeshType = Mesh;

  using Constants = SaintVenantConstants<ValueType,MeshType>;
  using State = SaintVenantState<ValueType,MeshType>;
  using Fluxes = SaintVenantFluxes<ValueType,MeshType>;

  using SourceTerm = SaintVenantSourceTerm<TimeType,ValueType,MeshType>;
  using MeasureType = SaintVenantMeasure<TimeType,ValueType,MeshType>;
  
private:

  std::shared_ptr<TimeParameters<TimeType>> time_params_;
  std::shared_ptr<MeshType> mesh_;

  std::shared_ptr<Constants> constants_;
  std::vector<std::shared_ptr<State>> U_;
  std::vector<std::shared_ptr<State>> dUdt_;

  std::shared_ptr<State> dUdx_;
  std::shared_ptr<State> dUdy_;
  std::shared_ptr<Fluxes> fluxes_;

  std::vector<std::shared_ptr<SourceTerm>> source_terms_;
  std::shared_ptr<SourceTerm> q_boundary_;
  std::shared_ptr<SourceTerm> h_boundary_;

  std::vector<std::shared_ptr<MeasureType>> measures_;

  CellField<ValueType, MeshType> stage_;
  
public:

  /**
     Constructor. Create a solver for the Saint Venant equations.

     @param queue Pointer to SYCL queue object to be used for the
     solution.
     @param no_of_states Number of intermediate states that the
     program must store.
     @param step_length Model duration of the inner loop in seconds.
  */
  SaintVenantSolver(const std::shared_ptr<sycl::queue>& queue,
		    size_t no_of_states,
		    const std::shared_ptr<TimeParameters<TimeType>>& tparams);

  const std::shared_ptr<sycl::queue>& queue(void)
  {
    return mesh_->queue_ptr();
  }

  void end_of_step(const TimeType& time_now)
  {
    // Update stage field
    stage_ = constants_->z_bed() + U_.at(0)->h();
    // Update measures
    for (auto&& measure : measures_) {
      measure->update(time_now, *(U_.at(0)));
    }
  }
  
  void update_dUdt(const size_t& state_no,
		   const TimeType& time_now,
		   const TimeType& timestep);

  void start_new_step(const TimeType& time_now,
		      const std::shared_ptr<TimeParameters<TimeType>>& tp_ptr);
  
  State& state(const size_t& i = 0)
  {
    return *(U_.at(i));
  }

  State& dUdt(const size_t& i)
  {
    return *(dUdt_.at(i));
  }

  ValueType control_number(const size_t& state_no,
			   const TimeType& timestep)
  {
    // std::cout << "Calculating control number for state " << state_no << std::endl;
    return U_.at(state_no)->max_control_number(timestep);
  }

  template<typename OutputFieldType>
  OutputFieldType* get_solver_output_field_ptr(const std::string& name)
  {
    return nullptr;
  }

  template<>
  CellField<ValueType,MeshType>* get_solver_output_field_ptr(const std::string& name)
  {
    if (name == "stage") {
      return &stage_;
    }
    return nullptr;
  }
  
  template<MeshComponent C>
  Field<ValueType,MeshType,C>* get_output_field_ptr(const std::string& name)
  {
    using OutputFieldType = Field<ValueType,MeshType,C>;

    OutputFieldType* ptr = nullptr;

    ptr = this->template get_solver_output_field_ptr<OutputFieldType>(name);
    if (ptr) return ptr;
    
    ptr = U_.at(0)->template get_output_field_ptr<OutputFieldType>(name);
    if (ptr) return ptr;

    for (auto&& source_term : source_terms_) {
      ptr = source_term->template get_output_field_ptr<C>(name);
      if (ptr) return ptr;
    }

    ptr = fluxes_->template get_output_field_ptr<OutputFieldType>(name);
    if (ptr) return ptr;

    return nullptr;
  }
  
  std::vector<typename State::FieldType*> state_output_vector(void)
  {
    return {
	&(U_.at(0)->h()),
	&(U_.at(0)->u()),
	&(U_.at(0)->v())
    };
  }
  
};

#endif

