
#include "transcoder.h"

#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle(QStyleFactory::create("Fusion"));
    QFile qss(":/styles/dark.qss");
    if (qss.open(QFile::ReadOnly | QFile::Text)) {
        a.setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }
    Transcoder w;

    // resize to specific
    w.resize(QSize(257,679));
    w.show();
    return a.exec();
}
