/***********************************************************************
 * mfcm sycl.hpp
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

#ifndef mfcm_sycl_hpp
#define mfcm_sycl_hpp

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-conversion"
#pragma GCC diagnostic ignored "-Wignored-attributes"
#include <CL/sycl.hpp>
namespace sycl = cl::sycl;
#pragma GCC diagnostic pop

std::shared_ptr<sycl::queue> get_sycl_queue(void);

#endif
