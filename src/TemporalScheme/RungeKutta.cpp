/***********************************************************************
 * mfcm TemporalScheme/RungeKutta.cpp
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

#include "RungeKutta.hpp"

RungeKuttaCoefficientSet::
RungeKuttaCoefficientSet(const std::vector<std::vector<double>>& a)
  : a_(a)
{
  // Check relative lengths of vectors
  size_t nsteps = a_.size() - 1;
  for (size_t row = 0; row < nsteps; ++row) {
    assert(a_[row].size() == row + 1);
  }
}

RungeKuttaCoefficientSet::
RungeKuttaCoefficientSet(const size_t& order,
			 const double& alpha)
{
  a_.push_back({0.0,});
  a_.push_back({alpha, alpha});
  if (order == 2) {
    a_.push_back({0.0, 1.0 - 1.0/(2.0*alpha), 1.0/(2.0*alpha)});
  } else if (order == 3) {
    double r3param = (1.0 - alpha) / (alpha * (3.0 * alpha - 2.0));
    a_.push_back({1.0, 1.0 + r3param, -r3param});
    a_.push_back({0.0,
	0.5 - 1.0 / (6.0 * alpha),
	1.0 / (6.0 * alpha * (1.0 - alpha)),
	(2.0 - 3.0 * alpha) / (6.0 * (1.0 - alpha))});
  } else {
    std::cerr << "No generic Runge Kutta parameters for order "
	      << order << std::endl;
    throw std::runtime_error("No generic Runge Kutta parameters available.");
  }
}

const std::map<std::string, RungeKuttaCoefficientSet>
runge_kutta_named_coefficient_sets_
  {
    {"euler",
     RungeKuttaCoefficientSet({{0.0,},
			       {0.0, 1.0}})},
    {"midpoint",
     RungeKuttaCoefficientSet({{0.0,},
			       {0.5, 0.5},
			       {0.0, 0.0, 1.0}})},
    {"heun2",
     RungeKuttaCoefficientSet({{0.0,},
			       {1.0, 1.0},
			       {0.0, 0.5, 0.5}})},
    {"ralston2",
     RungeKuttaCoefficientSet({{0.0,},
			       {2.0/3.0, 2.0/3.0},
			       {0.0, 0.25, 0.75}})},
    {"kutta3",
     RungeKuttaCoefficientSet({{0.0,},
			       {0.5, 0.5},
			       {1.0, -1.0, 2.0},
			       {0.0, 1.0/6.0, 2.0/3.0, 1.0/6.0}})},
    {"heun3",
     RungeKuttaCoefficientSet({{0.0,},
			       {1.0/3.0, 1.0/3.0},
			       {2.0/3.0, 0.0, 2.0/3.0},
			       {0.0, 0.25, 0.0, 0.75}})},
    {"wray3",
     RungeKuttaCoefficientSet({{0.0,},
			       {8.0/15.0, 8.0/15.0},
			       {2.0/3.0, 0.25, 5.0/12.0},
			       {0.0, 0.25, 0.0, 0.75}})},
    {"ralston3",
     RungeKuttaCoefficientSet({{0.0,},
			       {0.5, 0.5},
			       {0.75, 0.0, 0.75},
			       {0.0, 2.0/9.0, 1.0/3.0, 4.0/9.0}})},
    {"ssprk3",
     RungeKuttaCoefficientSet({{0.0,},
			       {1.0, 1.0},
			       {0.5, 0.25, 0.25},
			       {0.0, 1.0/6.0, 1.0/6.0, 2.0/3.0}})},
    {"classic",
     RungeKuttaCoefficientSet({{0.0,},
			       {0.5, 0.5},
			       {0.5, 0.0, 0.5},
			       {1.0, 0.0, 0.0, 1.0},
			       {0.0, (1.0/6.0), (1.0/3.0), (1.0/3.0), (1.0/6.0)}})},
    {"ralston4",
     RungeKuttaCoefficientSet({{0.0,},
			       {0.4, 0.4},
			       {0.45573725, 0.29697761, 0.15875964},
			       {1.0, 0.21810040, -3.05096516, 3.83286476},
			       {0.0, 0.17476028, -0.55148066, 1.20553560, 0.17118478}})},
    {"3/8rule",
     RungeKuttaCoefficientSet({{0.0,},
			       {1.0/3.0, 1.0/3.0},
			       {2.0/3.0, -1.0/3.0, 1.0},
			       {1.0, 1.0, -1.0, 1.0},
			       {0.0, 1.0/8.0, 3.0/8.0, 3.0/8.0, 1.0/8.0}})},
  };

std::ostream& RungeKuttaCoefficientSet::butcher_tableau(std::ostream& os)
{
  // Print the Butcher tableau
  os << "Butcher tableau for Runge Kutta scheme is:" << std::endl;
  for (size_t row = 0; row < nsteps() - 1; ++row) {
    os << std::to_string(a_[row][0]) << " │ ";
    for (size_t col = 1; col < a_[row].size(); ++col) {
      os << std::to_string(a_[row][col]) << "   ";
    }
    os << std::endl;
  }
  os << "────────" << "─┼─";
  for (size_t col = 1; col < a_.size(); ++col) {
    os << "────────";
  }
  os << std::endl;
  os << "        " << " │ ";
  for (size_t col = 1; col < a_[nsteps()].size(); ++col) {
    os << std::to_string(a_[nsteps()][col]) << "  ";
  }
  os << std::endl;
  return os;
}

template<typename T>
typename RungeKuttaTemporalScheme<T>::timestep_result
RungeKuttaTemporalScheme<T>::do_timestep(const TimeType& local_time,
					 const TimeType& dt)
{
  //  std::cout << "Starting timestep at " << step_start_time << " with timestep " << timestep << std::endl;
  for (size_t substep = 1; substep <= coeffs_.nsteps(); ++substep) {
    // std::cout << "Sub-step " << substep << std::endl;
    TimeType substep_time = local_time + coeffs_.c(substep) * dt;
    // CHECK: should update_k and update_y get dt or dt*some coeff?
    this->update_k(substep, substep_time, dt);
    this->update_y(substep, dt);
  }
  double cn_target = 1.0;
  double cn = this->get_latest_control_number(dt);
  // std::cout << "Step control number: " << cn << std::endl;
  if (cn > cn_target) {
    if (cn > 5.0 * cn_target) {
      return { TimeType(dt / 5.0), true };
    } else {
      return { TimeType(dt / (cn * 1.1)), true };
    }
  } else if (cn > 0.9 * cn_target) {
    return { TimeType(dt), false };
  } else {
    return { TimeType(dt / cn), false };
  }
}
