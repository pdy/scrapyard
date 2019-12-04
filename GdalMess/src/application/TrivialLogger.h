#ifndef PDY_LOGGER_H_
#define PDY_LOGGER_H_

#include <boost/log/trivial.hpp>

#define LOG_TRC BOOST_LOG_TRIVIAL(trace)
#define LOG_DBG BOOST_LOG_TRIVIAL(debug)
#define LOG_INF BOOST_LOG_TRIVIAL(info)
#define LOG_WRN BOOST_LOG_TRIVIAL(warning)
#define LOG_ERR BOOST_LOG_TRIVIAL(error)
#define LOG_FTL BOOST_LOG_TRIVIAL(fatal)

#endif
