#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QQuickWindow>

#include <QPointer>

class QQmlEngine;
class QQmlComponent;

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

    void init();

    Q_DISABLE_COPY(MainWindow)
};

#endif // MAINWINDOW_HPP
