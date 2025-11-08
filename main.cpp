
#include "transcoder.h"
#include "configmanager.h"

#include <QApplication>
#include <QTextCodec>
#include <QFile>
#include <QStyleFactory>
#include <QGraphicsDropShadowEffect>
#include <QMenuBar>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 初始化配置管理器
    ConfigManager *config = ConfigManager::instance();

    a.setStyle(QStyleFactory::create("Fusion"));

    // 根据配置加载主题
    QString themeName = config->getSystemSettings().theme;
    QString themeFile = (themeName == "modern") ? ":/styles/modern.qss" : ":/styles/dark.qss";
    QFile qss(themeFile);
    if (qss.open(QFile::ReadOnly | QFile::Text))
    {
        a.setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }
    Transcoder w;

    w.resize(QSize(257, 679));
    w.show();

    auto menus = w.menuBar()->findChildren<QMenu *>();
    for (auto *menu : menus)
    {
        menu->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
        menu->setAttribute(Qt::WA_TranslucentBackground);
    }

    return a.exec();
}
