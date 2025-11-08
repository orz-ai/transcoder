
#include "transcoder.h"

#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QStyleFactory>
#include <QGraphicsDropShadowEffect>
#include <QMenuBar>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("Fusion"));
    QFile qss(":/styles/modern.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text))
    {
        a.setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }
    Transcoder w;

    w.resize(QSize(257, 679));
    w.show();


    auto menus = w.menuBar()->findChildren<QMenu*>();
    for (auto *menu : menus)
    {
        menu-> setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        menu->setAttribute(Qt::WA_TranslucentBackground);
    }

    return a.exec();
}
