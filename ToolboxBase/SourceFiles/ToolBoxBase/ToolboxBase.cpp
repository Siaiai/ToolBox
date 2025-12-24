#include "ToolboxBase.h"
#include <QDebug>
#include <QStandardItem>

ToolboxBase::ToolboxBase(QWidget* parent)
    : QMainWindow(parent)
    , m_pluginModel(new QStandardItemModel(this))
{
    ui.setupUi(this);

    // 初始化TreeView模型
    m_pluginModel->setHorizontalHeaderLabels({ u8"插件列表" });
    ui.treeView->setModel(m_pluginModel);
    // 隐藏TreeView的根节点（只显示插件项）
    ui.treeView->setRootIsDecorated(false);

    // 绑定按钮点击事件（选择路径）
    connect(ui.pushButton, &QPushButton::clicked, this, &ToolboxBase::onSelectPathClicked);
    // 绑定TreeView点击事件（加载插件）
    connect(ui.treeView, &QTreeView::clicked, this, &ToolboxBase::onPluginItemClicked);

    // 初始化LineEdit（默认路径为空）
    ui.lineEdit->setPlaceholderText(u8"请选择插件所在路径");
}

ToolboxBase::~ToolboxBase()
{
    // 释放所有插件加载器（卸载DLL）
    for (auto loader : m_pluginLoaders.values()) {
        loader->unload();
        delete loader;
    }
}

// 路径选择按钮点击事件
void ToolboxBase::onSelectPathClicked()
{
    // 弹出文件夹选择对话框
    QString selectPath = QFileDialog::getExistingDirectory(
        this,
        u8"选择插件路径",
        ui.lineEdit->text().isEmpty() ? QDir::currentPath() : ui.lineEdit->text()
    );

    if (selectPath.isEmpty()) {
        return;
    }

    // 更新LineEdit显示选中的路径
    ui.lineEdit->setText(selectPath);
    // 扫描该路径下的插件
    scanPlugins(selectPath);
}

// 扫描指定路径下的插件
void ToolboxBase::scanPlugins(const QString& path)
{
    // 清空原有插件列表
    m_pluginModel->clear();
    m_pluginModel->setHorizontalHeaderLabels({ u8"插件列表" });

    QDir pluginDir(path);
    // 只筛选DLL文件（先实现DLL插件，EXE后续扩展）
    QStringList filter;
    filter << "*.dll";
    QFileInfoList fileList = pluginDir.entryInfoList(filter, QDir::Files);

    for (const QFileInfo& fileInfo : fileList) {
        QString pluginPath = fileInfo.absoluteFilePath();
        // 尝试加载插件
        PluginInterface* plugin = loadPlugin(pluginPath);
        if (plugin) {
            // 插件加载成功，添加到TreeView
            QStandardItem* item = new QStandardItem(plugin->pluginName());
            // 存储插件路径到Item的Data中，方便后续加载
            item->setData(pluginPath, Qt::UserRole);
            m_pluginModel->appendRow(item);
            qDebug() << "加载插件成功：" << plugin->pluginName();
        }
        else {
            qDebug() << "加载插件失败：" << pluginPath;
        }
    }

    if (m_pluginModel->rowCount() == 0) {
        QStandardItem* emptyItem = new QStandardItem(u8"当前路径无有效插件");
        emptyItem->setEnabled(false);
        m_pluginModel->appendRow(emptyItem);
    }
}

// 加载单个插件
PluginInterface* ToolboxBase::loadPlugin(const QString& pluginPath)
{
    // 如果已加载过该插件，直接返回实例
    if (m_pluginLoaders.contains(pluginPath)) {
        QPluginLoader* loader = m_pluginLoaders[pluginPath];
        return qobject_cast<PluginInterface*>(loader->instance());
    }

    // 新建插件加载器
    QPluginLoader* loader = new QPluginLoader(pluginPath, this);
    QObject* pluginObj = loader->instance();
    if (!pluginObj) {
        qDebug() << "插件加载失败原因：" << loader->errorString();
        delete loader;
        return nullptr;
    }

    // 转换为修改后的自定义插件接口
    PluginInterface* plugin = qobject_cast<PluginInterface*>(pluginObj);
    if (!plugin) {
        qDebug() << "插件未实现PluginInterface接口";
        loader->unload();
        delete loader;
        return nullptr;
    }

    // 缓存加载器
    m_pluginLoaders[pluginPath] = loader;
    return plugin;
}

// 点击TreeView中的插件项
void ToolboxBase::onPluginItemClicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    // 获取插件路径
    QString pluginPath = index.data(Qt::UserRole).toString();
    if (pluginPath.isEmpty()) {
        return;
    }

    // 加载插件
    PluginInterface* plugin = loadPlugin(pluginPath);
    if (!plugin) {
        return;
    }

    // 创建插件UI组件
    QWidget* pluginWidget = plugin->createPluginWidget(ui.tabWidget);
    if (!pluginWidget) {
        return;
    }

    // 在TabWidget中新建标签页，展示插件UI
    int tabIndex = ui.tabWidget->addTab(pluginWidget, plugin->pluginName());
    ui.tabWidget->setCurrentIndex(tabIndex);
}