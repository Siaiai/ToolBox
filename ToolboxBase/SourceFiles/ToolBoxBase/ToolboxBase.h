#pragma once

#include <QtWidgets/QMainWindow>
#include <QStandardItemModel>
#include <QPluginLoader>
#include <QDir>
#include <QFileDialog>
#include <QMap>
#include "ui_ToolboxBase.h"
#include "common/PluginInterface.h"

class ToolboxBase : public QMainWindow
{
    Q_OBJECT

public:
    ToolboxBase(QWidget* parent = nullptr);
    ~ToolboxBase();

private slots:
    // 路径选择按钮点击事件
    void onSelectPathClicked();
    // TreeView点击插件项事件
    void onPluginItemClicked(const QModelIndex& index);

private:
    Ui::ToolboxBaseClass ui;
    // 插件列表模型（用于TreeView展示）
    QStandardItemModel* m_pluginModel;
    // 缓存插件路径和加载器（避免重复加载DLL）
    QMap<QString, QPluginLoader*> m_pluginLoaders;

    // 扫描指定路径下的所有有效插件
    void scanPlugins(const QString& path);
    // 加载单个插件（返回修改后的插件接口实例）
    PluginInterface* loadPlugin(const QString& pluginPath);
};