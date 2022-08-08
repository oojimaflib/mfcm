/***********************************************************************
 * mfcm DataArray/DataArray.hpp
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

#ifndef mfcm_DataArray_DataArray_hpp
#define mfcm_DataArray_DataArray_hpp

#include <vector>
#include <memory>

#include "sycl.hpp"

/**
   @brief Class representing an array on either the host or GPU device.

   The array can be of any type, T, that can be used in
   sycl::buffer<T,1>. 
 */
template<typename T>
class DataArray
{
protected:

  /** 
      Pointer to the SYCL queue through which this data may be
      accessed.
  */
  std::shared_ptr< sycl::queue > queue_;

  /** 
      Pointer to the host data vector.
   */
  std::shared_ptr< std::vector<T> > host_data_;

  /** 
      Pointer to the SYCL buffer object representing this data on the
      device.
  */
  std::shared_ptr< sycl::buffer<T,1> > device_data_;

public:

  using AccessMode = sycl::access::mode;
  using AccessTarget = sycl::access::target;
  using AccessPlaceholder = sycl::access::placeholder;

  template<AccessMode Mode,
	   AccessTarget Target = AccessTarget::global_buffer,
	   AccessPlaceholder IsPlaceholder = AccessPlaceholder::false_t>
  using Accessor = sycl::accessor<T, 1, Mode, Target, IsPlaceholder>;

  /**
     Return a (non-placeholder) SYCL accessor to the data in the array
     with the given mode and target.
  */
  template<AccessMode Mode, AccessTarget Target>
  Accessor<Mode, Target> get_accessor(sycl::handler& cgh) const
  {
    return device_data_->template get_access<Mode, Target>(cgh);
  }

  /**
     Return a (non-placeholder) SYCL read accessor to the data in the
     array with the global device buffer target.
  */
  Accessor<sycl::access::mode::read>
  get_read_accessor(sycl::handler& cgh) const
  {
    return device_data_->template get_access<sycl::access::mode::read>(cgh);
  }
  
  /**
     Return a (non-placeholder) SYCL write accessor to the data in the
     array with the global device buffer target.
  */
  Accessor<sycl::access::mode::write>
  get_write_accessor(sycl::handler& cgh) const
  {
    return device_data_->template get_access<sycl::access::mode::write>(cgh);
  }
  
  /**
     Return a (non-placeholder) SYCL "discard" write accessor to the
     data in the array with the global device buffer target.
  */
  Accessor<sycl::access::mode::discard_write>
  get_discard_write_accessor(sycl::handler& cgh) const
  {
    return device_data_->template get_access<sycl::access::mode::discard_write>(cgh);
  }
  
  /**
     Return a (non-placeholder) SYCL read/write accessor to the data
     in the array with the global device buffer target.
  */
  Accessor<sycl::access::mode::read_write>
  get_read_write_accessor(sycl::handler& cgh) const
  {
    return device_data_->template get_access<sycl::access::mode::read_write>(cgh);
  }
  
  /**
     Return a placeholder SYCL accessor to the data in the array with
     the given mode and target.
  */
  template<AccessMode Mode, AccessTarget Target>
  Accessor<Mode, Target, AccessPlaceholder::true_t> get_placeholder_accessor(void) const
  {
    assert((bool)device_data_);
    return Accessor<Mode, Target, AccessPlaceholder::true_t>(*device_data_);
  }

public:

  /**
     Construct from a vector of data on the host.
   */
  DataArray(const std::shared_ptr<sycl::queue>& queue,
	    const std::vector<T>& data,
	    bool on_device = false);

  /**
     Construct with a given size and default value.

     If on_device is true, immediately copy the data to the GPU
     device.
   */
  DataArray(const std::shared_ptr<sycl::queue>& queue,
	    const size_t& size,
	    const T& value = T(),
	    bool on_device = false);

  /**
     Copy constructor.
   */
  DataArray(const DataArray<T>& da);

  /**
     Move constructor
  */
  DataArray(DataArray<T>&& da) = default;

  /**
     Destructor.

     If the object is destroyed while the data is still on the device,
     the data is un-linked from the host array so it is not copied
     back to the host.
  */
  ~DataArray(void);

  /**
     Returns the number of elements in the array.
  */
  size_t size(void) const
  {
    if (host_data_) {
      return host_data_->size();
    } else if (device_data_) {
      return device_data_->get_count();
    } else {
      throw std::logic_error("Data array has neither host nor device data.");
    }
  }

  /**
     Returns the pointer to the queue associated with this array.
  */
  const std::shared_ptr<sycl::queue>& queue_ptr(void) const
  {
    return queue_;
  }

  /**
     Returns a reference to the queue associated with this array.
  */
  sycl::queue& queue(void) const
  {
    return *queue_;
  }

  /**
     Returns a const reference to the associated vector on the
     host. Only valid if is_on_device() is false.
   */
  const std::vector<T>& host_vector(void) const
  {
    assert(host_data_);
    return *host_data_;
  }

  /**
     Returns a non-const reference to the associated vector on the
     host. Only valid if is_on_device() is false.
   */
  std::vector<T>& host_vector(void)
  {
    assert(host_data_);
    return *host_data_;
  }
 
  /**
     Copies the data in this array from the host to the device. If the
     data is already on the device this method does nothing.
   */
  void move_to_device(void);

  /**
     Copies the data in this array from the device to the host,
     removing it from the device.
   */
  void move_to_host(void);

  /**
     Returns whether the array data is on the device (true) or on the
     host (false)
   */
  bool is_on_device(void) const
  {
    return (bool) device_data_;
  }

  /**
     Returns a reference to the underlying SYCL buffer object
     representing the data on the device. Only valid if is_on_device()
     is true.
   */
  sycl::buffer<T,1>& get_buffer(void)
  {
    return *device_data_;
  }
  
  /**
     Returns a reference to the underlying SYCL buffer object
     representing the data on the device. Only valid if is_on_device()
     is true.
   */
  const sycl::buffer<T,1>& get_buffer(void) const
  {
    return *device_data_;
  }
  
};

#endif
