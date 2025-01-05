
#include "transcoder.h"

#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Transcoder w;

    // resize to specific
    w.resize(QSize(257,679));
    w.show();
    return a.exec();
}
