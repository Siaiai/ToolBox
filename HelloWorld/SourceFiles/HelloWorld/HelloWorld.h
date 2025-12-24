#pragma once

#include <QObject>
#include <QtWidgets/QWidget>
#include "common/PluginInterface.h"
#include "ui_Helloworld.h"

// 保留原UI类
class HelloWorld : public QWidget
{
    Q_OBJECT

public:
    HelloWorld(QWidget* parent = nullptr);
    ~HelloWorld();

private:
    Ui::HelloWorldClass ui;
};

// 插件核心类（修改Q_PLUGIN_METADATA，去掉FILE参数）
class HelloWorldPlugin : public QObject, public PluginInterface
{
    Q_OBJECT
        // 仅保留IID，去掉plugin_meta.json关联
        Q_PLUGIN_METADATA(IID PluginInterface_iid)
        Q_INTERFACES(PluginInterface)

public:
    QString pluginName() const override;
    QString pluginDesc() const override;
    QWidget* createPluginWidget(QWidget* parent = nullptr) override;
};