/***********************************************************************
 * mfcm DataArray/DataArray.cpp
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

#include "DataArray.hpp"

template<typename T>
DataArray<T>::DataArray(const std::shared_ptr<sycl::queue>& queue,
			const std::vector<T>& data,
			bool on_device)
  : queue_(queue),
    host_data_(std::make_shared< std::vector<T> >(data)),
    device_data_()
{
  if (on_device) {
    move_to_device();
  }
}

template<typename T>
DataArray<T>::DataArray(const std::shared_ptr<sycl::queue>& queue,
			const size_t& size,
			const T& value,
			bool on_device)
  : queue_(queue),
    host_data_(),
    device_data_()
{
  if (on_device) {
    device_data_ = std::make_shared<sycl::buffer<T,1>>(sycl::range<1>(size));
    queue_->submit([&](sycl::handler& cgh)
    {
      cgh.fill(this->get_discard_write_accessor(cgh), value);
    });
  } else {
    host_data_ = std::make_shared<std::vector<T>>(size,value);
  }
}

template<typename T>
DataArray<T>::DataArray(const DataArray<T>& da)
  : queue_(da.queue_),
    host_data_(),
    device_data_()
{
  if (da.host_data_) {
    host_data_ = std::make_shared< std::vector<T> >(*da.host_data_);
  }
  if (da.is_on_device()) {
    if (host_data_) {
      move_to_device();
    } else {
      device_data_ = std::make_shared<sycl::buffer<T,1>>(sycl::range<1>(da.size()));
    }
    da.queue_->submit([&](sycl::handler& cgh)
    {
      cgh.copy(da.get_read_accessor(cgh),
	       this->get_discard_write_accessor(cgh));
    });
  }
}

template<typename T>
DataArray<T>::~DataArray(void)
{
  if (device_data_) device_data_->set_final_data();
}

template<typename T>
void DataArray<T>::move_to_device(void)
{
  if (device_data_) {
    // Data is already on the device. Do nothing.
    return;
  }

  if (host_data_ && host_data_->size() > 0) {
    // Create the SYCL buffer object
    device_data_ =
      std::make_shared<sycl::buffer<T,1>>(host_data_->data(),
					  sycl::range<1>(host_data_->size()));
  } else {
    // No actual data to copy, but we need there to be a thing on
    // the device
    host_data_ = std::make_shared<std::vector<T>>(1);
    device_data_ =
      std::make_shared<sycl::buffer<T,1>>(host_data_->data(),
					  sycl::range<1>(host_data_->size()));

    host_data_->pop_back();
  }
}

template<typename T>
void DataArray<T>::move_to_host(void)
{
  if (not host_data_) {
    host_data_ = std::make_shared<std::vector<T>>(device_data_->get_count());
    device_data_->set_final_data(host_data_->begin());
  }
  device_data_.reset();
}
