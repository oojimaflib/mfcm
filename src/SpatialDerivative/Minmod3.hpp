/***********************************************************************
 * mfcm SpatialDerivative/Minmod3.hpp
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

#ifndef mfcm_SpatialDerivative_Minmod3_hpp
#define mfcm_SpatialDerivative_Minmod3_hpp

template<typename T>
class Minmod3
{
private:

  static constexpr const T theta_ = 2.0;
  
public:

  using ValueType = T;

  //  __host__ __device__
  inline ValueType operator()(const ValueType& U_l,
			      const double& dx_l,
			      const ValueType& U_c,
			      const double& dx_r,
			      const ValueType& U_r)
  {
    // LHS and RHS differences: if dx is close to zero keep them at
    // NaN.
    T lhs_difference = std::numeric_limits<T>::quiet_NaN();
    if (dx_l > 0.01 * dx_r) {
      lhs_difference = theta_ * (U_c - U_l) / T(dx_l);
    }
    T rhs_difference = std::numeric_limits<T>::quiet_NaN();
    if (dx_r > 0.01 * dx_l) {
      rhs_difference = theta_ * (U_r - U_c) / T(dx_r);
    }
    // Central difference. If dx is close to zero, set this to zero
    // (as we expect the lhs and rhs differences to be NaN).
    T central_difference = 0.0;
    T total_dx = dx_l + dx_r;
    if (total_dx > 1e-6) {
      central_difference = (U_r - U_l) / T(dx_l + dx_r);
    }
    // Calculat the minmod of the outer (lhs and rhs) differences. If
    // either is NaN, use the other one. If both are NaN,
    // outer_difference will also be NaN.
    T outer_difference = std::numeric_limits<T>::quiet_NaN();
    if (std::isnan(lhs_difference)) {
      outer_difference = rhs_difference;
    } else if (std::isnan(rhs_difference)) {
      outer_difference = lhs_difference;
    } else {
      outer_difference = T(0.5) * (sycl::sign(lhs_difference) +
				   sycl::sign(rhs_difference)) *
	sycl::min(sycl::fabs(lhs_difference),
		  sycl::fabs(rhs_difference));
    }
    // Calculate the minmod of the outer and central differences. If
    // the outer difference is NaN, then just return the central
    // difference (which is zero for zero dx).
    if (std::isnan(outer_difference)) {
      return central_difference;
    } else {
      return T(0.5) * (sycl::sign(outer_difference) +
		       sycl::sign(central_difference)) *
	sycl::min(sycl::fabs(outer_difference),
		  sycl::fabs(central_difference));
    }
  }
  
};

#endif
