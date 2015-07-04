#include "homescreencontroller.hpp"

#include <QDebug>
#include <QtQml>

namespace qtouch
{

HomeScreenController::HomeScreenController(QObject* parent) :
		QObject(parent)
{
	mVisible = false;
}

HomeScreenController::~HomeScreenController()
{
}

void HomeScreenController::onLoaded()
{
	mVisible = true;
	emit visibilityChanged();
}

} /* namespace qtouch */
