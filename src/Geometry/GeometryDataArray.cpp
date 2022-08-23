/***********************************************************************
 * mfcm Geometry/GeometryDataArray.cpp
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

#include "GeometryDataArray.hpp"

template<sycl::access::mode Mode,
	 sycl::access::target Target>
PointDataArrayAccessor<Mode, Target>::
PointDataArrayAccessor(const PointDataArray& pda)
  : has_z_(pda.has_z()),
    has_m_(pda.has_m()),
    data_acc_(pda.template get_placeholder_accessor<Mode, Target>())
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
PointDataArrayAccessor<Mode, Target>::
PointDataArrayAccessor(const PointDataArray& pda,
		       sycl::handler& cgh)
  : has_z_(pda.has_z()),
    has_m_(pda.has_m()),
    data_acc_(pda.template get_accessor<Mode, Target>(cgh))
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
void PointDataArrayAccessor<Mode, Target>::bind(sycl::handler& cgh)
{
  cgh.require(data_acc_);
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
MultiPointDataArrayAccessor<Mode, Target>::
MultiPointDataArrayAccessor(const MultiPointDataArray& mpda)
  : has_z_(mpda.has_z()),
    has_m_(mpda.has_m()),
    data_acc_(mpda.template get_placeholder_accessor<Mode, Target>())
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
MultiPointDataArrayAccessor<Mode, Target>::
MultiPointDataArrayAccessor(const MultiPointDataArray& mpda,
			    sycl::handler& cgh)
  : has_z_(mpda.has_z()),
    has_m_(mpda.has_m()),
    data_acc_(mpda.template get_accessor<Mode, Target>(cgh))
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
void MultiPointDataArrayAccessor<Mode, Target>::bind(sycl::handler& cgh)
{
  cgh.require(data_acc_);
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
LineStringDataArrayAccessor<Mode, Target>::
LineStringDataArrayAccessor(const LineStringDataArray& lsda)
  : has_z_(lsda.has_z()),
    has_m_(lsda.has_m()),
    data_acc_(lsda.template get_placeholder_accessor<Mode, Target>())
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
LineStringDataArrayAccessor<Mode, Target>::
LineStringDataArrayAccessor(const LineStringDataArray& lsda,
			    sycl::handler& cgh)
  : has_z_(lsda.has_z()),
    has_m_(lsda.has_m()),
    data_acc_(lsda.template get_accessor<Mode, Target>(cgh))
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
void LineStringDataArrayAccessor<Mode, Target>::bind(sycl::handler& cgh)
{
  cgh.require(data_acc_);
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
MultiLineStringDataArrayAccessor<Mode, Target>::
MultiLineStringDataArrayAccessor(const MultiLineStringDataArray& mlsda)
  : has_z_(mlsda.has_z()),
    has_m_(mlsda.has_m()),
    data_acc_(mlsda.template get_placeholder_accessor<Mode, Target>())
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
MultiLineStringDataArrayAccessor<Mode, Target>::
MultiLineStringDataArrayAccessor(const MultiLineStringDataArray& mlsda,
				 sycl::handler& cgh)
  : has_z_(mlsda.has_z()),
    has_m_(mlsda.has_m()),
    data_acc_(mlsda.template get_accessor<Mode, Target>(cgh))
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
void MultiLineStringDataArrayAccessor<Mode, Target>::bind(sycl::handler& cgh)
{
  cgh.require(data_acc_);
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
PolygonDataArrayAccessor<Mode, Target>::
PolygonDataArrayAccessor(const PolygonDataArray& pgnda)
  : has_z_(pgnda.has_z()),
    has_m_(pgnda.has_m()),
    data_acc_(pgnda.template get_placeholder_accessor<Mode, Target>())
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
PolygonDataArrayAccessor<Mode, Target>::
PolygonDataArrayAccessor(const PolygonDataArray& pgnda,
			 sycl::handler& cgh)
  : has_z_(pgnda.has_z()),
    has_m_(pgnda.has_m()),
    data_acc_(pgnda.template get_accessor<Mode, Target>(cgh))
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
void PolygonDataArrayAccessor<Mode, Target>::bind(sycl::handler& cgh)
{
  cgh.require(data_acc_);
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
MultiPolygonDataArrayAccessor<Mode, Target>::
MultiPolygonDataArrayAccessor(const MultiPolygonDataArray& mpgnda)
  : has_z_(mpgnda.has_z()),
    has_m_(mpgnda.has_m()),
    data_acc_(mpgnda.template get_placeholder_accessor<Mode, Target>())
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
MultiPolygonDataArrayAccessor<Mode, Target>::
MultiPolygonDataArrayAccessor(const MultiPolygonDataArray& mpgnda,
			      sycl::handler& cgh)
  : has_z_(mpgnda.has_z()),
    has_m_(mpgnda.has_m()),
    data_acc_(mpgnda.template get_accessor<Mode, Target>(cgh))
{
}

template<sycl::access::mode Mode,
	 sycl::access::target Target>
void MultiPolygonDataArrayAccessor<Mode, Target>::bind(sycl::handler& cgh)
{
  cgh.require(data_acc_);
}

