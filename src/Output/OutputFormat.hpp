/***********************************************************************
 * mfcm Output/OutputFormat.hpp
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

#ifndef mfcm_Output_OutputFormat_hpp
#define mfcm_Output_OutputFormat_hpp

#include "OutputFunction.hpp"

class FormattedOutput
{
protected:

  std::shared_ptr<OutputFunction> of_;

public:

  FormattedOutput(const std::shared_ptr<OutputFunction>& of = std::shared_ptr<OutputFunction>())
    : of_(of)
  {}

  virtual ~FormattedOutput(void) {}

  virtual void set_output_function(const std::shared_ptr<OutputFunction>& of)
  {
    of_ = of;
  }
  
  virtual void output(const stdfs::path& filename) = 0;
  
};

class RawBinaryOutput : public FormattedOutput
{
public:

  RawBinaryOutput(const std::shared_ptr<OutputFunction>& of)
    : FormattedOutput(of)
  {
  }

  void output(const stdfs::path& filename)
  {
    uint64_t nrows = of_->nrows();
    uint64_t ncols = of_->ncols();
    uint64_t bvs = of_->binary_value_size();
    
    std::ofstream ofs(filename, std::ios::binary);
    ofs.write((char*)(&nrows), sizeof(size_t));
    ofs.write((char*)(&ncols), sizeof(size_t));
    ofs.write((char*)(&bvs), sizeof(size_t));
    char buf[bvs * ncols];
    for (size_t row = 0; row < nrows; ++row) {
      for (size_t col = 0; col < ncols; ++col) {
	of_->set_binary_value_ptr(col, row, buf + col * bvs);
      }
      ofs.write(buf, bvs*ncols);
    }
    ofs.close();
  }
  
};

class CSVOutput : public FormattedOutput
{
private:

  std::string delimiter_;
  
public:
  
  CSVOutput(const std::shared_ptr<OutputFunction>& of,
	    const std::string delimiter = ", ")
    : FormattedOutput(of), delimiter_(delimiter)
  {}

  void output(const stdfs::path& filename)
  {
    size_t nrows = of_->nrows();
    size_t ncols = of_->ncols();

    std::ofstream ofs(filename);

    if (of_->rows_have_location()) {
      ofs << "X" << delimiter_
	  << "Y" << delimiter_;
    }
    for (size_t col = 0; col < ncols; ++col) {
      ofs << std::quoted(of_->column_name(col)) << delimiter_;
    }
    ofs << std::endl;
    
    for (size_t row = 0; row < nrows; ++row) {
      if (of_->rows_have_location()) {
	for (auto&& coord : of_->location(row)) {
	  ofs << coord << delimiter_;
	}
      }
      for (size_t col = 0; col < ncols; ++col) {
	ofs << of_->at_str(col, row) << delimiter_;
      }
      ofs << std::endl;
    }

    ofs.close();
  }
  
};

#endif
