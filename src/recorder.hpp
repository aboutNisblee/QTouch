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
 * \file recorder.hpp
 *
 * \date 09.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef RECORDER_HPP_
#define RECORDER_HPP_

#include <QObject>
#include <QDateTime>

class QTimer;

namespace qtouch
{

class Recorder: public QObject
{
	Q_OBJECT

	Q_PROPERTY(QDateTime start READ getStart NOTIFY startChanged)
	Q_PROPERTY(int elapsed READ getElapsed NOTIFY elapsedChanged)
	Q_PROPERTY(int hits READ getHits NOTIFY hitsChanged)
	Q_PROPERTY(int misses READ getMisses NOTIFY missesChanged)

public:
	Recorder(QObject* parent = nullptr);
	virtual ~Recorder();

	QDateTime getStart() const { return mStart; }
	int getElapsed() const { return mElapsed; }
	int getHits() const { return mHits; }
	int getMisses() const { return mMisses; }

signals:
	void startChanged();
	void elapsedChanged();
	void hitsChanged();
	void missesChanged();

	void onShowHint();
	void onHideHint();

public slots:
	void reset();
	void pause();
	void hit();
	void unhit();
	void miss();
	void unmiss();

private:
	void timeout();
	void resume();

	QTimer* mStatsTimer;
	QTimer* mPauseTimer;
	QTimer* mHintTimer;

	QDateTime mStart;
	int mElapsed = 0;
	int mHits = 0;
	int mMisses = 0;
};

} /* namespace qtouch */

#endif /* RECORDER_HPP_ */
