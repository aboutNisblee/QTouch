/**
 * \file typecontroller.cpp
 *
 * \date 09.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QTimer>
#include <QDebug>
#include "recorder.hpp"

namespace qtouch
{

Recorder::Recorder(QObject* parent) :
	QObject(parent)
{
	mStatsTimer = new QTimer(this);
	mPauseTimer = new QTimer(this);
	mHintTimer = new QTimer(this);

	mStatsTimer->setInterval(500);
	mStatsTimer->setSingleShot(false);
	connect(mStatsTimer, &QTimer::timeout, this, &Recorder::timeout);

	// TODO: Make it configurable
	mPauseTimer->setInterval(3000);
	mPauseTimer->setSingleShot(true);
	connect(mPauseTimer, &QTimer::timeout, this, &Recorder::pause);

	// TODO: Make it configurable
	mHintTimer->setInterval(2000);
	mHintTimer->setSingleShot(true);
	connect(mHintTimer, &QTimer::timeout, this, &Recorder::onHideHint);
}

Recorder::~Recorder()
{
}

void Recorder::reset()
{
	mStatsTimer->stop();
	mHintTimer->stop();

	mStart = QDateTime();
	emit startChanged();

	mElapsed = 0;
	emit elapsedChanged();

	mHits = 0;
	emit hitsChanged();

	mMisses = 0;
	emit missesChanged();
}

void Recorder::pause()
{
	qDebug() << "Pause";
	mStatsTimer->stop();
	mHintTimer->stop();
}

void Recorder::hit()
{
	resume();
	++mHits;
	emit hitsChanged();
}

void Recorder::unhit()
{
	resume();
	if (mHits > 0)
	{
		--mHits;
		emit hitsChanged();
	}
}

void Recorder::miss()
{
	resume();
	++mMisses;
	emit missesChanged();
}


void Recorder::unmiss()
{
	resume();
}

void Recorder::timeout()
{
	mElapsed += mStatsTimer->interval();
	emit elapsedChanged();
}

void Recorder::resume()
{
	if (!mStatsTimer->isActive())
		mStatsTimer->start();

	if (!mStart.isValid())
	{
		qDebug() << "Start";
		mStart = QDateTime::currentDateTime();
		emit startChanged();
	}
	else if (!mPauseTimer->isActive())
	{
		qDebug() << "Resume";
	}

	mPauseTimer->start();
}

} /* namespace qtouch */

