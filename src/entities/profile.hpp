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
 * \file profile.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef PROFILE_HPP_
#define PROFILE_HPP_

#include <memory>
#include <vector>
#include <QString>

#include "stats.hpp"

namespace qtouch
{

class Profile
{
public:
	typedef std::vector<std::shared_ptr<const Stats>>::const_iterator const_iterator;

	/** SkillLevel */
	enum SkillLevel
	{
		Beginner,//!< Beginner
		Advanced //!< Advanced
	};

	explicit Profile(const QString& name, SkillLevel skill = Beginner) : mName(name), mSkillLevel(skill) {}
	virtual ~Profile() {}

	inline const QString& getName() const { return mName; }

	inline SkillLevel getSkillLevel() const { return mSkillLevel; }
	inline void setSkillLevel(SkillLevel skillLevel) { mSkillLevel = skillLevel; }

	void push_back(const Stats& stats);
	inline void clear() { mStats.clear(); }
	inline void replace(std::vector<std::shared_ptr<const Stats>> stats) { mStats.swap(stats); }
	inline int size() const { return mStats.size(); }

	inline const_iterator begin() const { return mStats.begin(); }
	inline const_iterator end() const { return mStats.end(); }

private:
	QString mName;
	SkillLevel mSkillLevel;

	// Note: No deep copy needed. Manipulation impossible.
	std::vector<std::shared_ptr<const Stats>> mStats;
};

} /* namespace qtouch */

#endif /* PROFILE_HPP_ */
