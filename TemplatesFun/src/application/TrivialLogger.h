#ifndef SEVERALGH_LOGGER_H_
#define SEVERALGH_LOGGER_H_

#include <boost/log/trivial.hpp>

#define LOG_TRC_TRIV BOOST_LOG_TRIVIAL(trace)
#define LOG_DBG_TRIV BOOST_LOG_TRIVIAL(debug)
#define LOG_INF_TRIV BOOST_LOG_TRIVIAL(info)
#define LOG_WRN_TRIV BOOST_LOG_TRIVIAL(warning)
#define LOG_ERR_TRIV BOOST_LOG_TRIVIAL(error)
#define LOG_FTL_TRIV BOOST_LOG_TRIVIAL(fatal)

#endif
