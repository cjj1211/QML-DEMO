#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "CG3Controller.h"
#include <QQmlContext>  // 这个头文件必须包含
int main(int argc, char *argv[])
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // STA 模型
    if (FAILED(hr)) {
        qFatal("Failed to initialize COM library");
    }
    QGuiApplication app(argc, argv);

    G3Controller* controller = new G3Controller();
    controller->setBleMacAdd(0xe5eb7bdecf50);  // 设置蓝牙MAC地址
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", controller);


    QString importPath = TaoQuickImportPath;
    importPath = importPath.replace("\\", "/");
    engine.addImportPath(importPath);
    engine.rootContext()->setContextProperty("taoQuickImportPath", importPath);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}