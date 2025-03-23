#include <QtWidgets>

#include "MainWindow.h"

int main(int argc, char * argv[])
{
    QCoreApplication::setOrganizationName("BeyondXin");
    QCoreApplication::setApplicationName("TextAnimationHelper");

    QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, 1);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, 1);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication app(argc, argv);
    app.setStyle("Fusion");
    QThread::currentThread()->setObjectName("Main");

    MainWindow w;
    w.show();

    int res = app.exec();
    return res;
}
