#ifndef LOGS_H
#define LOGS_H

#include <cassert>
#include <iostream>
#include <fstream>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

enum severity_level
{
    normal,
    warning,
    error
};

template< typename CharT, typename TraitsT >
inline std::basic_ostream<CharT, TraitsT>& operator<< (std::basic_ostream< CharT, TraitsT >& strm, severity_level lvl)
{
    static const char* const str[] =
    {
        "normal",
        "warning",
        "error"
    };
    if(static_cast<std::size_t>(lvl) < (sizeof(str) / sizeof(*str)))
        strm << str[lvl];
    else
        strm << static_cast<int>(lvl);
    return strm;
}

#endif // LOGS_H
