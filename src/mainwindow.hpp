#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QQuickWindow>

#include <QPointer>

class QQmlEngine;
class QQmlComponent;

class HomeScreenController;
class LessonModel;

class MainWindow : public QQuickWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QQmlEngine* engine = 0);
    ~MainWindow();

signals:

public slots:

protected:
    void resizeEvent(QResizeEvent* e);

private:
    QPointer<QQmlEngine> mwEngine;
    QQmlComponent* component;
    QPointer<QQuickItem> rootItem;

//    HomeScreenController* hsController;

    LessonModel* lessonModel;

    void init();
    bool componentError(QQmlComponent*);

    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_HPP
