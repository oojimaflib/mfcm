/***********************************************************************
 * mfcm TemporalScheme/TemporalScheme.hpp
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

#ifndef mfcm_TemporalScheme_TemporalScheme_hpp
#define mfcm_TemporalScheme_TemporalScheme_hpp

#include <memory>
#include "TimeParameters.hpp"

/**
   Base class for temporal schemes---solution-independent time
   integrators.
*/
class TemporalScheme
{
private:

public:

  /**
     Constructor.
  */
  TemporalScheme(void)
  {}

  /**
     Destructor.
  */
  virtual ~TemporalScheme(void) {}

  /**
     Function to compute the solution.
  */
  virtual void solve(void) {}

};

/**
   Base class for temporal schemes.

   @tparam T Floating-point type used for time calculations.
*/
template<typename T>
class TypedTemporalScheme : public TemporalScheme
{
public:

  /**
     Floating-point type used for time calculations.
  */
  using TimeType = T;
  
private:

  /**
     Time parameters.
  */
  std::shared_ptr<TimeParameters<T>> time_params_;

  /**
     Current timestep.
  */
  TimeType dt_;

protected:
  
  /**
     Helper struct to return from the inner loop.
  */
  struct step_result
  {
    /**
       Number of timesteps that it took to complete the inner loop.
    */
    size_t num_timesteps;
    /**
       Number of timesteps that had to be repeated during this inner
       loop.
    */
    size_t num_repeated_timesteps;
  };

  /**
     Compute a computational step. 

     A step is of fixed duration and consists of a number of
     timesteps. The timesteps vary in duration with the duration of
     the next timestep being one of the outputs of the previous
     timestep. Note that the step has no knowledge of the global
     time. All the times used here and below are with reference to the
     start of the step.

     @returns a step_result object containing the number of timesteps
     required to complete the step and the number of timesteps that
     were repeated.
  */
  step_result step(void)
  {
    step_result result { 0, 0 };
    
    TimeType t_local = 0.0;
    TimeType step_duration = time_params_->step_duration();

    while (t_local < step_duration) {
      // TimeType t_now = start_time + t_local;

      auto [ new_dt, repeat_timestep ] = this->do_timestep(t_local, dt_);

      if (repeat_timestep) {
	std::cout << "Repeating timestep at local time " << t_local << std::endl;
	result.num_repeated_timesteps++;
      } else {
	result.num_timesteps++;
	t_local += dt_;
	this->accept_timestep();
      }
      dt_ = new_dt;
      
      if (t_local >= step_duration) {
	return result;
      } else if (t_local + dt_ > step_duration) {
	// Will reach the end of step in one timestep. Reduce the
	// timestep to hit the end of step exactly.
	dt_ = step_duration - t_local;
      } else if (t_local + 2.0 * dt_ > step_duration) {
	// Will reach end of step in 2 timesteps. Reduce the timestep
	// to split the difference 40/60 to hopefully guarantee that
	// we don't end up with a very short timestep.
	dt_ = (step_duration - t_local) * 0.4;
      }
    }

    throw std::runtime_error("Should not be possible to reach here.");
  }

protected:

  /**
     Helper struct to return from the do_timestep method.
  */
  struct timestep_result
  {
    /**
       Suggested duration for the next timestep.
    */
    TimeType new_dt;
    /**
       Boolean indicating whether this timestep should be repeated.
    */
    bool repeat_timestep;
  };

  /**
     Calculate the solution from one timestep.

     @param[in] local_time The simulation time relative to the start
     of the step.
     @param[in] dt The duration of the timestep.
  */
  virtual timestep_result do_timestep(const TimeType& local_time,
				      const TimeType& timestep) = 0;

  /**
     Mark the solution of the last timestep as valid (so that future
     timesteps will start from there). If this is not called after
     each timestep, the step will be repeated.
  */
  virtual void accept_timestep(void) = 0;

  /**
     Update "measure" outputs (if any) for the current time

     @param[in] time_now The (global) simulation time.
  */
  virtual void update_measures(const TimeType& time_now) = 0;
  
  /**
     Write user-requested outputs (if any) relating to the current
     time.

     @param[in] time_now The (global) simulation time.
  */
  virtual void do_outputs(const TimeType& time_now) = 0;

  /**
     Prepare the solver to start a new step. This method is used to
     update values in boundary conditions, source terms and the like.

     @param[in] time_now The global simulation time.
     @param[in] step_duration The length of the step.
   */
  virtual void start_new_step(const TimeType& time_now) = 0;

public:

  /**
     Constructor.

     @param[in] tparams Pointer to the time parameters object.
  */
  TypedTemporalScheme(const std::shared_ptr<TimeParameters<T>>& tparams)
    : TemporalScheme(),
      time_params_(tparams),
      dt_(time_params_->initial_timestep())
  {}

  virtual ~TypedTemporalScheme(void)
  {}

  /**
     Get the time parameters.
  */
  virtual std::shared_ptr<TimeParameters<TimeType>> time_parameters(void) const
  {
    return time_params_;
  }
  
  /**
     Compute the outer loop.

     The solution process operates as follows: The total solution time
     is subdivided into steps of constant length. Each step is handled
     by a call to the (private) inner_loop method. At the start of
     each step any model outputs are performed and the boundary
     conditions (and other source terms) are updated before the
     solution for that step is undertaken. It is assumed that boundary
     conditions, in particular, will vary linearly within a step.
  */
  virtual void solve(void)
  {
    // Get the simulation times and number of steps from the time
    // paramters object.
    TimeType start_time = time_params_->start_time();
    size_t num_steps = time_params_->num_steps();
    TimeType step_duration = time_params_->step_duration();
    dt_ = time_params_->initial_timestep();

    for (size_t i = 0; i < num_steps; ++i) {
      // Calculate the current global time.
      TimeType time_now = start_time + i * step_duration;
      std::cout << "Starting step " << i
		<< " at time " << time_now << std::endl;

      this->update_measures(time_now);
      
      // Write any outputs
      this->do_outputs(time_now);

      // Perform step set-up (the solver can do anything here, but we
      // expect it to do things like update boundary conditions and
      // other slow-moving things).
      this->start_new_step(time_now);

      // Perform the step solution.
      auto [num_timesteps, num_repeated_timesteps] { this->step() };

      // TODO: do something with the results from the inner loop.
    }
    
    this->do_outputs(start_time + num_steps * step_duration);
  }
  
};

#endif
