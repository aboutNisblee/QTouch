#include "homescreencontroller.hpp"

#include <QDebug>
#include <QtQml>

HomeScreenController::HomeScreenController(QObject* parent): QObject(parent)
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
