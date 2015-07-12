/**
 * \file typo.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef TYPO_HPP
#define TYPO_HPP

#include <QDataStream>

namespace qtouch
{

/**
 * @brief Container that holds a single typo.
 * This class is used to count typos of a single character.
 * The character can come from nearly any language because it is
 * stored as a 16-bit Unicode. The typo count can be incremented
 * and reset in either case a corresponding signal is emitted.
 */
class Typo
{
public:
	Typo();
	explicit Typo(QChar const&);
	Typo(QChar const&, quint16);
	virtual ~Typo();

	QChar const& getChar() const;
	quint16 getCount() const;

	void incrementCount();
	void resetCount();

private:
	QChar mChar;
	quint16 mCount;
};

QDataStream& operator<<(QDataStream& out, Typo const& typo);
QDataStream& operator>>(QDataStream& in, Typo& typo);

} /* namespace qtouch */

#endif // TYPO_HPP
