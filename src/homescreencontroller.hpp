#ifndef HOMESCREENCONTROLLER_HPP
#define HOMESCREENCONTROLLER_HPP

#include <QObject>
#include <QString>

namespace qtouch
{

class HomeScreenController: public QObject
{
Q_OBJECT

Q_PROPERTY(bool visible READ isVisible NOTIFY visibilityChanged)

public:
	explicit HomeScreenController(QObject* parent = 0);
	virtual ~HomeScreenController();

	bool isVisible()
	{
		return mVisible;
	}

signals:
	void visibilityChanged();

public slots:
	void onLoaded();

private:
	bool mVisible;
};

} /* namespace qtouch */

#endif // HOMESCREENCONTROLLER_HPP
