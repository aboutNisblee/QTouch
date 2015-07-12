/**
 * \file profile.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef PROFILE_HPP_
#define PROFILE_HPP_

#include <QString>
#include <QSharedPointer>

namespace qtouch
{

class Profile;
typedef QSharedPointer<Profile> ProfilePtr;

class Profile
{
public:
	Profile();
	virtual ~Profile();

	const QString& getName() const;
	void setName(const QString& name);
	quint8 getSkillLevel() const;
	void setSkillLevel(quint8 skillLevel);

private:
	QString mName;
	quint8 mSkillLevel;
};

} /* namespace qtouch */

#endif /* PROFILE_HPP_ */
