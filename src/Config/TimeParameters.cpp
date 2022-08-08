/***********************************************************************
 * mfcm Config/TimeParameters.cpp
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

#include "TimeParameters.hpp"

template<typename TimeType>
TimeParameters<TimeType>::
TimeParameters(const Config& conf)
  : parser_(conf)
{
  start_time_ = parser_.parse(conf.get<std::string>("start time", "0"));
  end_time_ = parser_.parse(conf.get<std::string>("end time"));
    
  step_duration_ = parser_.parse(conf.get<std::string>("step duration", "0"));
  if (step_duration_ == 0.0) {
    step_duration_ = conf.get<TimeType>("step duration seconds", 60.0);
  }

  initial_timestep_ = conf.get<TimeType>("initial timestep seconds", 1.0);
  maximum_timestep_ = conf.get<TimeType>("maximum timestep seconds",
					 step_duration_);
  minimum_timestep_ = conf.get<TimeType>("minimum timestep seconds", 0.001);
}

template<typename TimeType>
size_t
TimeParameters<TimeType>::
num_steps(void) const
{
  double eps = 1e-6;
  double sim_duration = simulation_duration();
  double fractional_steps = sim_duration / step_duration_;
  size_t nsteps = (size_t) fractional_steps;
  if ((step_duration_ * nsteps) >= sim_duration - eps and
      (step_duration_ * nsteps) <= sim_duration + eps) {
    // Simulation duration is close enough to an even number of
    // steps
    return nsteps;
  } else {
    // Simulation duration is not an even number of steps
    return nsteps + 1;
  }
}
