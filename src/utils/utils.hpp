/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
 *
 * This file is part of QTouch.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file utils.hpp
 *
 * \date 30.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <memory>
#include <type_traits>

namespace qtouch
{

/* Anonymous namespace is a bit useless since the client must include the file */
namespace
{
template <typename>
struct is_unique_ptr_helper : public std::false_type {};
template <typename Tp>
struct is_unique_ptr_helper<std::unique_ptr<Tp>> : public std::true_type {};
template <typename>
struct is_shared_ptr_helper : public std::false_type {};
template <typename Tp>
struct is_shared_ptr_helper<std::shared_ptr<Tp>> : public std::true_type {};
} /* namespace anonymous*/

/** If T is of type std::unique_ptr this type derives from std::true_type
 * else from std::false_type. */
template <typename T>
struct is_unique_ptr : public
		is_unique_ptr_helper<typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type> {};

/** If T is of type std::shared_ptr this type derives from std::true_type
 * else from std::false_type. */
template <typename T>
struct is_shared_ptr : public
		is_shared_ptr_helper<typename std::remove_cv<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::type> {};

/** If T is of type std::unique_ptr orstd::shared_ptr this type derives
 * from std::true_type else from std::false_type. */
template <typename T>
struct is_smart_ptr : public std::conditional < (is_unique_ptr<T>::value
        || is_shared_ptr<T>::value), std::true_type, std::false_type >::type {};

template<typename T>
inline T&& value(T&& v) { return std::forward<T>(v); }

template<typename T>
inline T& value(T* v) { return *v; }

template<typename Tp>
inline Tp& value(std::shared_ptr<Tp>& v) { return *v; }

template<typename Tp>
inline Tp& value(const std::shared_ptr<Tp>& v) { return *v; }

template<typename Tp>
inline Tp& value(std::unique_ptr<Tp>& v) { return *v; }

template<typename Tp>
inline Tp& value(const std::unique_ptr<Tp>& v) { return *v; }

/**
 * This simple tool stores a reference to a boolean variable,
 * flips its state on creation and back on destruction.
 * @note The boolean variable must outlive the object of this class.
 * Simply don't put this on the heap, it doesn't make sense anyway.
 */
class ScopedFlag
{
public:
	explicit ScopedFlag(bool& flag) :	f(flag) { f = !f; }
	ScopedFlag(bool& flag, bool set_to) :	f(flag) { f = set_to; }
	~ScopedFlag()	{ f = !f; }
private:
	ScopedFlag(const ScopedFlag&) = delete;
	ScopedFlag& operator=(const ScopedFlag&) = delete;
	bool& f;
};

} /* namespace qtouch */

#endif /* UTILS_HPP_ */
