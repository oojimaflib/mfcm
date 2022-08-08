/***********************************************************************
 * mfcm Config/TimeParameters.hpp
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

#ifndef mfcm_Config_TimeParameters_hpp
#define mfcm_Config_TimeParameters_hpp

#include <map>

#include "Config.hpp"
#include "TimeParser.hpp"

/**
   Store for time parameters for the simulation. Handles reading these
   parameters and converting between user units and seconds (used
   internally).

   @tparam TimeType Type used for representing time values.
*/
template<typename TimeType>
class TimeParameters
{
private:

  TimeParser<TimeType> parser_;
  
  TimeType start_time_;
  TimeType end_time_;
  TimeType step_duration_;
  TimeType initial_timestep_;

  TimeType minimum_timestep_;
  TimeType maximum_timestep_;
  
public:

  /**
     Constructor.
  */
  TimeParameters(const Config& conf);

  /**
     Destructor.
  */
  virtual ~TimeParameters(void) {}

  /**
     Return the time parser object
   */
  const TimeParser<TimeType>& parser(void) const
  {
    return parser_;
  }
  
  /**
     Return the simulation start time in seconds.
  */
  const TimeType& start_time(void) const { return start_time_; }
  
  /**
     Return the simulation end time in seconds.
  */
  const TimeType& end_time(void) const { return end_time_; }

  /**
     Return the duration of each step in seconds.
  */
  const TimeType& step_duration(void) const { return step_duration_; }

  /**
     Return the duration the initial timestep.
  */
  const TimeType& initial_timestep(void) const { return initial_timestep_; }

  /**
     Return the minimum permissible timestep
  */
  const TimeType& minimum_timestep(void) const { return minimum_timestep_; }

  /**
     Return the maximum permissible timestep
  */
  const TimeType& maximum_timestep(void) const { return maximum_timestep_; }

  /**
     Return the duration of the simulation in seconds.
  */
  TimeType simulation_duration(void) const
  {
    return end_time_ - start_time_;
  }

  /**
     Return the number of steps in the simulation.
  */
  size_t num_steps(void) const;

};

#endif
