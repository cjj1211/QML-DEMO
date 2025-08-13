#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "CG3Controller.h"
#include <QQmlContext>  // ���ͷ�ļ��������
int main(int argc, char *argv[])
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); // STA ģ��
    if (FAILED(hr)) {
        qFatal("Failed to initialize COM library");
    }
    QGuiApplication app(argc, argv);

    G3Controller* controller = new G3Controller();
    controller->setBleMacAdd(0xe5eb7bdecf50);  // ��������MAC��ַ
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