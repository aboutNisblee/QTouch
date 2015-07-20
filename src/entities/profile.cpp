/**
 * \file profile.cpp
 *
 * \date 24.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "profile.hpp"

namespace qtouch
{

void Profile::push_back(const Stats& stats)
{
	auto ptr = std::make_shared<const Stats>(stats);
	mStats.push_back(ptr);
}

} /* namespace qtouch */
