/***********************************************************************
 * mf_parser.hpp
 *
 * Parser to read "mf" format files into a boost::property_tree
 *
 * Copyright (C) Edenvale Young Associates 2020.
 *
 * Sections lifted extensively from boost/property_tree which is
 * Copyright (C) 2009 Sebastian Redl
 *
 * Distributed under the Boost Software License, Version 1.0. 
 * (See accompanying file LICENSE_1_0.txt or copy at 
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * For more information, see www.boost.org
 ***********************************************************************/

#include "mf_parser.hpp"

namespace boost {
  namespace property_tree {
    namespace mf_parser {

      template<class Ptree, class Ch>
      void read_mf(std::basic_istream<Ch>& stream, Ptree& pt)
      {
	Ptree local;
	read_mf_internal(stream, local, std::string(), 0);
	pt.swap(local);
      }

      template<class Ptree, class Ch>
      void read_mf(std::basic_istream<Ch>& stream, Ptree& pt,
                   const Ptree &default_ptree)
      {
	try {
	  read_mf(stream, pt);
	} catch (file_parser_error&) {
	  pt = default_ptree;
	}
      }

      template<class Ptree>
      void read_mf(const std::string &filename, Ptree &pt,
		   const std::locale &loc)
      {
        std::basic_ifstream<typename Ptree::key_type::value_type>
	  stream(filename.c_str());
        if (!stream) {
	  BOOST_PROPERTY_TREE_THROW(mf_parser_error(
						    "cannot open file for reading", filename, 0));
        }
        stream.imbue(loc);
        Ptree local;
        read_mf_internal(stream, local, filename, 0);
        pt.swap(local);
      }

      template<class Ptree>
      void read_mf(const std::string &filename,
		   Ptree &pt,
		   const Ptree &default_ptree,
		   const std::locale &loc)
      {
	try {
	  read_mf(filename, pt, loc);
	} catch(file_parser_error &) {
	  pt = default_ptree;
	}
      }

      template<class Ptree, class Ch>
      void write_mf(std::basic_ostream<Ch> &stream,
		    const Ptree &pt,
		    const mf_writer_settings<Ch> &settings)
      {
        write_mf_internal(stream, pt, std::string(), settings);
      }

      template<class Ptree>
      void write_mf(const std::string &filename,
                    const Ptree &pt,
                    const std::locale &loc,
                    const mf_writer_settings<
		    typename Ptree::key_type::value_type
                    > &settings)
      {
        std::basic_ofstream<typename Ptree::key_type::value_type>
	  stream(filename.c_str());
        if (!stream) {
	  BOOST_PROPERTY_TREE_THROW(mf_parser_error(
						    "cannot open file for writing", filename, 0));
        }
        stream.imbue(loc);
        write_mf_internal(stream, pt, filename, settings);
      }

    }
  }
}
