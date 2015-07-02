/**
 * \file profile.cpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "profile.hpp"

namespace qtouch
{

Profile::Profile() :
		mSkillLevel(0)
{
	// TODO Auto-generated constructor stub

}

Profile::~Profile()
{
	// TODO Auto-generated destructor stub
}

const QString& Profile::getName() const
{
	return mName;
}

void Profile::setName(const QString& name)
{
	mName = name;
}

quint8 Profile::getSkillLevel() const
{
	return mSkillLevel;
}

void Profile::setSkillLevel(quint8 skillLevel)
{
	mSkillLevel = skillLevel;
}

} /* namespace qtouch */
