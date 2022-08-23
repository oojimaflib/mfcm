/***********************************************************************
 * mfcm Geometry/Geometry_impl.cpp
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

#include "Geometry.cpp"
#include "GeometryDataArray.cpp"

template class PointDataArrayAccessor<sycl::access::mode::read,
				      sycl::access::target::global_buffer>;
template class MultiPointDataArrayAccessor<sycl::access::mode::read,
					   sycl::access::target::global_buffer>;
template class LineStringDataArrayAccessor<sycl::access::mode::read,
					   sycl::access::target::global_buffer>;
template class MultiLineStringDataArrayAccessor<sycl::access::mode::read,
						sycl::access::target::global_buffer>;
template class PolygonDataArrayAccessor<sycl::access::mode::read,
					sycl::access::target::global_buffer>;
template class MultiPolygonDataArrayAccessor<sycl::access::mode::read,
					     sycl::access::target::global_buffer>;

