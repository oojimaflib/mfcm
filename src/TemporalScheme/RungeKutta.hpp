/***********************************************************************
 * mfcm TemporalScheme/RungeKutta.hpp
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

#ifndef mfcm_TemporalScheme_RungeKutta_hpp
#define mfcm_TemporalScheme_RungeKutta_hpp

#include "TemporalScheme.hpp"

#include "Field.hpp"
#include "../Output/CheckFile.hpp"
#include "../Output/OutputFile.hpp"

/**
   Specialisation of the TimeParameters object with additional data
   for the Runge Kutta scheme. (No additional data is currently used.)
*/
template<typename T>
class RungeKuttaTimeParameters : public TimeParameters<T>
{
public:
  RungeKuttaTimeParameters(const Config& conf)
    : TimeParameters<T>(conf)
  {}
};

/**
   Coefficients for the Runge Kutta scheme.

   This class stores the Runge Kutta coefficients that form the
   Butcher Tableau for the Runge Kutta scheme. The coefficients in the
   tableau are represented as follows:

   \f[
   \begin{array}{c|cccc}
   c_1 \\
   c_2 | a_{21} \\
   c_3 | a_{31} | a_{32} \\
   c_4 | a_{41} | a_{42} | a_{43} \\
   \hline
   {} | b_1 | b_2 | b_3 | b_4 \\
   \end{array}
   \f]
*/
class RungeKuttaCoefficientSet
{
private:
  
  std::vector<std::vector<double>> a_;

public:

  /**
     Default constructor. This does not produce a valid coefficient set.
  */
  RungeKuttaCoefficientSet(void)
    : a_()
  {}

  /**
     Constructor. Initialises the coefficient set from a nested vector
     of coefficients.
  */
  RungeKuttaCoefficientSet(const std::vector<std::vector<double>>& a);

  /**
     Constructor. Initialise the coefficient set given the desired
     order and alpha parameter.
  */
  RungeKuttaCoefficientSet(const size_t& order, const double& alpha);

  /**
     Writes the Butcher Tableau to the supplied output stream.
  */
  std::ostream& butcher_tableau(std::ostream& os);

  /**
     Number of steps (order) of the Runge Kutta scheme.
  */
  size_t nsteps(void) const
  {
    return a_.size() - 1;
  }

  /**
     Returns the coefficient \f$a_{ij}\f$.
  */
  const double& a(const size_t& i, const size_t& j) const
  {
    return a_[i - 1][j];
  }

  /**
     Returns the coefficient \f$b_i\f$.
  */
  const double& b(const size_t& i) const
  {
    return a_[a_.size() - 1][i];
  }

  /**
     Returns the coefficient \f$c_i\f$.
  */
  const double& c(const size_t& i) const
  {
    return a_[i - 1][0];
  }
    
};

extern const std::map<std::string,
	       RungeKuttaCoefficientSet> runge_kutta_named_coefficient_sets_;

/**
   Explicit Runge Kutta temporal integration scheme.

   The method takes the form:
   \f{eqnarray*}{
   y_{n+1} &=& y_n + {\Delta t} \sum_{i=1}^{N} b_i k_i, \\
   y_{ni} &=& y_n + {\Delta t} \sum_{j=1}^{i-1} a_{ij} k_j, \\
   k_i &=& f(t_n + c_i {\Delta t}, y_{ni}). \\
   \f}

   @tparam T Type used to represent times.
*/
template<typename T>
class RungeKuttaTemporalScheme : public TypedTemporalScheme<T>
{
public:

  using TimeType = T;

protected:

  /**
     Coefficients for the scheme.
  */
  RungeKuttaCoefficientSet coeffs_;

  /**
     Accessor for the coefficients.
  */
  const RungeKuttaCoefficientSet& coeffs(void) const { return coeffs_; }

  /**
     Calculate \f$k_i\f$.

     @param[in] step The sub-step (value of \f$i\f$).
     @param[in] substep_time The local time at the start of the
     substep. i.e. \f$t_n + {\Delta t} c_i\f$.
     @param[in] timestep The current timestep, \f$\Delta t\f$.
   */
  virtual void update_k(const size_t& step,
			const TimeType& substep_time,
			const TimeType& timestep) = 0;

  /**
     Calculate \f$y_{ni}\f$.
     
     @param[in] step The sub-step (value of \f$i\f$).
     @param[in] timestep The current timestep, \f$\Delta t\f$.     
   */
  virtual void update_y(const size_t& step,
			const TimeType& timestep) = 0;

public:

  /**
     Construct the scheme from a time parameters object and a
     coefficient set.
  */
  RungeKuttaTemporalScheme(const std::shared_ptr<TimeParameters<T>>& tparams,
			   const RungeKuttaCoefficientSet& coeffs)
    : TypedTemporalScheme<TimeType>(tparams),
      coeffs_(coeffs)
  {}

  /**
     Construct the scheme from a time parameters object and a set of
     generic Runge Kutta parameters calculated from a desired order
     and alpha factor.
  */
  RungeKuttaTemporalScheme(const std::shared_ptr<TimeParameters<T>>& tparams,
			   const size_t& order, const double& alpha)
    : TypedTemporalScheme<TimeType>(tparams),
      coeffs_(order, alpha)
  {}
  
  /**
     Construct the scheme from a time parameters object and a named
     set of coefficients that can be looked up in
     runge_kutta_named_coefficient_sets_
  */
  RungeKuttaTemporalScheme(const std::shared_ptr<TimeParameters<T>>& tparams,
			   const std::string& named_coeffs)
    : TypedTemporalScheme<TimeType>(tparams),
      coeffs_()
  {
    if (runge_kutta_named_coefficient_sets_.count(named_coeffs) > 0) {
      coeffs_ = runge_kutta_named_coefficient_sets_.at(named_coeffs);
    } else {
      std::cerr << "Unknown Runge Kutta scheme name: "
		<< std::quoted(named_coeffs) << std::endl;
      throw std::runtime_error("Unknown name for Runge Kutta scheme.");
    }
  }

  virtual ~RungeKuttaTemporalScheme(void) {}

protected:

  /**
     Local alias for the timestep_result helper class.
  */
  using timestep_result = typename TypedTemporalScheme<T>::timestep_result;

  /**
     Do the timestep.
  */
  virtual timestep_result do_timestep(const TimeType& local_time,
				      const TimeType& dt);

  /**
     Get the control number from the result of the last timestep.

     @param[in] dt The duration of the last timestep.

     @returns the control number indicating whether the solution was
     stable or, if not, the extent to which the timestep duration
     should be adjusted.
  */
  virtual double get_latest_control_number(const TimeType& dt) = 0;

  /**
     Mark the timestep as successful. This copies the result of the
     last timestep from \f$y_nN\f$ to \f$y_n\f$ so it is used as the
     basis of the next timestep.
  */
  virtual void accept_timestep(void) = 0;

  /**
     Write any user-defined outputs.
  */
  virtual void do_outputs(const TimeType& time_now) = 0;

};

/**
   Glue class linking the RungeKuttaTemporalScheme to a solver object.

   @tparam T The floating point used to represent time.
   @tparam Solver The type of solver object.
*/
template<typename Solver>
class RungeKuttaSolver : public RungeKuttaTemporalScheme<typename Solver::TimeType>
{
public:

  using TimeType = typename Solver::TimeType;
  using SolverType = Solver;
  using OutputFileType = TimedOutputFile<TimeType>;
  
protected:
  
  std::shared_ptr<SolverType> solver_;

  std::vector<std::shared_ptr<OutputFileType>> outputs_;
  
  bool step_debugging_;

public:

  /**
     Constructor from an arbitrary set of coefficients.

     @param[in] tparams Pointer to the time parameters object.
     @param[in] coeffs Runge Kutta coefficients
     @param[in] queue Pointer to the SYCL queue object
     @param[in] step_debugging Boolean to turn on super-verbose
     outputs for debugging the numerical scheme.
  */
  RungeKuttaSolver(const std::shared_ptr<TimeParameters<TimeType>>& tparams,
		   const RungeKuttaCoefficientSet& coeffs,
		   const std::shared_ptr<sycl::queue>& queue,
		   bool step_debugging = false)
    : RungeKuttaTemporalScheme<TimeType>(tparams, coeffs),
      solver_(std::make_shared<SolverType>(queue, coeffs.nsteps() + 1,
					   tparams)),
      step_debugging_(step_debugging)
  {
    for (auto&& name : GlobalConfig::instance().output_files_list()) {
      outputs_.push_back(make_field_output_file<SolverType>(name, tparams, solver_));
    }
  }

  /**
     Constructor from a set of named coefficients

     @param[in] tparams Pointer to the time parameters object.
     @param[in] coeffs Runge Kutta coefficients
     @param[in] queue Pointer to the SYCL queue object
     @param[in] step_debugging Boolean to turn on super-verbose
     outputs for debugging the numerical scheme.
  */
  RungeKuttaSolver(const std::shared_ptr<TimeParameters<TimeType>>& tparams,
		   const std::string& named_coeffs,
		   const std::shared_ptr<sycl::queue>& queue,
		   bool step_debugging = false)
    : RungeKuttaTemporalScheme<TimeType>(tparams, named_coeffs),
      solver_(std::make_shared<SolverType>(queue, runge_kutta_named_coefficient_sets_.at(named_coeffs).nsteps() + 1,
					   tparams)),
      step_debugging_(step_debugging)
  {
    for (auto&& name : GlobalConfig::instance().output_files_list()) {
      outputs_.push_back(make_field_output_file<SolverType>(name, tparams, solver_));
    }
  }

protected:
  
  /**
     Calculate \f$k_i\f$.

     @param[in] step The sub-step (value of \f$i\f$).
     @param[in] substep_time The local time at the start of the
     substep. i.e. \f$t_n + {\Delta t} c_i\f$.
     @param[in] timestep The current timestep, \f$\Delta t\f$.
   */
  virtual void update_k(const size_t& step,
			const TimeType& substep_time,
			const TimeType& timestep)
  {
    if (step_debugging_) {
      std::cout << "Updating k" << step - 1 << " at " << substep_time << " with timestep " << timestep << std::endl;
    }
    solver_->update_dUdt(step - 1, substep_time, timestep);

    if (step_debugging_) {
      using FieldType = Field<typename SolverType::ValueType,
			      typename SolverType::MeshType,MeshComponent::Cell>;
      FieldCheckFile<FieldType> cf("step debugging");
      cf.set_output_filename(GlobalConfig::instance().simulation_base_path() /
			     "check" /
			     (std::string("k") + std::to_string(step-1) + ".csv"));
      cf.output({ &(solver_->dUdt(step-1).h()),
	  &(solver_->dUdt(step-1).u()),
	  &(solver_->dUdt(step-1).v()) });
    }
  }

  /**
     Calculate \f$y_{ni}\f$.
     
     @param[in] step The sub-step (value of \f$i\f$).
     @param[in] timestep The current timestep, \f$\Delta t\f$.     
  */
  virtual void update_y(const size_t& step,
			const TimeType& timestep)
  {
    if (step_debugging_) {
      std::cout << "Updating y" << step << std::endl;
    }
    if (step > 0) {
      solver_->state(step) = solver_->state(0);
      if (step_debugging_) {
	std::cout << "y" << step << " = y0";
      }
      for (size_t col = 0; col < step; ++col) {
	solver_->state(step) +=
	  solver_->dUdt(col) * (this->coeffs().a(step+1, col+1) * timestep);
	if (step_debugging_) {
	  std::cout << " + k" << col << "×" << this->coeffs().a(step+1, col+1) << "×" << timestep;
	}
      }
      if (step_debugging_) {
	std::cout << std::endl;
      }
    }

    if (step_debugging_) {
      using FieldType = Field<typename SolverType::ValueType,
			      typename SolverType::MeshType,MeshComponent::Cell>;
      FieldCheckFile<FieldType> cf("step debugging");
      cf.set_output_filename(GlobalConfig::instance().simulation_base_path() /
			     "check" /
			     (std::string("y") + std::to_string(step) + ".csv"));
      cf.output({ &(solver_->state(step).h()),
	  &(solver_->state(step).u()),
	  &(solver_->state(step).v()) });
    }
  }

public:
  
  /**
     Get the control number from the result of the last timestep.

     @param[in] dt The duration of the last timestep.

     @returns the control number indicating whether the solution was
     stable or, if not, the extent to which the timestep duration
     should be adjusted.
  */
  virtual double get_latest_control_number(const TimeType& dt)
  {
    return solver_->control_number(this->coeffs().nsteps(), dt);
  }
  
  /**
     Mark the timestep as successful. This copies the result of the
     last timestep from \f$y_nN\f$ to \f$y_n\f$ so it is used as the
     basis of the next timestep.
  */
  virtual void accept_timestep(void)
  {
    solver_->state() = solver_->state(this->coeffs().nsteps());
  }

  /**
     Update any measures or output-only fields
  */
  virtual void end_of_step(const TimeType& time_now)
  {
    solver_->end_of_step(time_now);
  }
  
  /**
     Write any user-defined outputs.
  */
  virtual void do_outputs(const TimeType& time_now)
  {
    for (auto&& output : outputs_) {
      output->timed_output(time_now);
    }
  }

  /**
     Prepare the solver to start a new step. This method is used to
     update values in boundary conditions, source terms and the like.

     @param[in] time_now The global simulation time.
     @param[in] step_duration The length of the step.
  */
  virtual void start_new_step(const TimeType& time_now)
  {
    solver_->start_new_step(time_now, this->time_parameters());
  }
  
};

#endif
