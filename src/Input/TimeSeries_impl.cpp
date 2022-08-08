/***********************************************************************
 * mfcm Input/TimeSeries_impl.cpp
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

#if __INCLUDE_LEVEL__
#error "This file should not be included."
#endif

#include "TimeSeries.cpp"

template<>
TimeSeriesDatabase<float,float>* TimeSeriesDatabase<float,float>::tsdb_ = nullptr;
template<>
TimeSeriesDatabase<double,float>* TimeSeriesDatabase<double,float>::tsdb_ = nullptr;
template<>
TimeSeriesDatabase<float,double>* TimeSeriesDatabase<float,double>::tsdb_ = nullptr;
template<>
TimeSeriesDatabase<double,double>* TimeSeriesDatabase<double,double>::tsdb_ = nullptr;

template class TimeSeries<float,float>;
template class TimeSeries<double,float>;
template class TimeSeries<float,double>;
template class TimeSeries<double,double>;

template class TimeSeriesAccessor<float,float>;
template class TimeSeriesAccessor<double,float>;
template class TimeSeriesAccessor<float,double>;
template class TimeSeriesAccessor<double,double>;

template class TimeSeriesDatabase<float,float>;
template class TimeSeriesDatabase<double,float>;
template class TimeSeriesDatabase<float,double>;
template class TimeSeriesDatabase<double,double>;

