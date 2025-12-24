#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QObject>
#include <QWidget>
#include <QString>

class PluginInterface
{
public:
    virtual ~PluginInterface() = default;

    // 获取插件名称
    virtual QString pluginName() const = 0;

    // 获取插件描述
    virtual QString pluginDesc() const = 0;

    // 创建插件的UI组件
    virtual QWidget* createPluginWidget(QWidget* parent = nullptr) = 0;
};

// 修改接口IID标识
#define PluginInterface_iid "com.toolbox.PluginInterface/1.0"
// 声明接口（QT插件机制必需）
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H