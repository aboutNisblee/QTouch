/**
 * \file utils.hpp
 *
 * \date 30.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

namespace qtouch
{

template<typename T>
inline T value(const T& v) {
    return v;
}

template<typename T>
inline T& value(const std::shared_ptr<T>& v) {
    return *v;
}

template<typename T>
inline T& value(const std::unique_ptr<T>& v) {
    return *v;
}

} /* namespace qtouch */

#endif /* UTILS_HPP_ */
