#include "HelloWorld.h"

HelloWorld::HelloWorld(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
}

HelloWorld::~HelloWorld()
{
}


// ---------------------- 插件接口实现 ----------------------
// 返回插件名称
QString HelloWorldPlugin::pluginName() const
{
    return "HelloWorld插件";
}

// 返回插件描述
QString HelloWorldPlugin::pluginDesc() const
{
    return "基于QWidget的极简HelloWorld插件，支持即插即用";
}

// 创建插件UI实例（每次调用新建，支持多开）
QWidget* HelloWorldPlugin::createPluginWidget(QWidget* parent)
{
    // 返回原Helloworld类的实例（主程序加载到Tab页）
    return new HelloWorld(parent);
}