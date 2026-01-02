#include "ToolboxBase.h"
#include <QDebug>
#include <QStandardItem>
#include <QFileDialog>
#include <QPluginLoader>

ToolboxBase::ToolboxBase(QWidget* parent)
    : QMainWindow(parent)
    , m_pluginModel(new QStandardItemModel(this))
{
    ui.setupUi(this);

    // ========== 新增：路径输入框设置为只读（不可修改） ==========
    ui.lineEdit_PluginDir->setReadOnly(true);
    // 可选：设置样式，提示用户不可编辑
    ui.lineEdit_PluginDir->setStyleSheet("QLineEdit:read-only { background-color: #F0F0F0; }");

    // 初始化TreeView模型
    m_pluginModel->setHorizontalHeaderLabels({ u8"插件列表" });
    ui.treeViewl_Plugins->setModel(m_pluginModel);
    // 隐藏TreeView的根节点（只显示插件项）
    ui.treeViewl_Plugins->setRootIsDecorated(false);

    // ========== 新增：TabWidget开启关闭按钮 ==========
    ui.tabWidget_PluginUI->setTabsClosable(true); // 显示关闭叉号
    // 绑定Tab关闭信号到自定义槽函数
    connect(ui.tabWidget_PluginUI, &QTabWidget::tabCloseRequested, this, &ToolboxBase::onTabCloseRequested);

    // 绑定按钮点击事件（选择路径）
    connect(ui.pushButton_PluginDir, &QPushButton::clicked, this, &ToolboxBase::onSelectPathClicked);
    // 绑定TreeView点击事件（加载插件）
    connect(ui.treeViewl_Plugins, &QTreeView::doubleClicked, this, &ToolboxBase::onPluginItemClicked);

    // 初始化LineEdit（默认路径为空）
    ui.lineEdit_PluginDir->setPlaceholderText(u8"请选择插件所在路径");
}

ToolboxBase::~ToolboxBase()
{
    // 释放所有插件加载器（卸载DLL）
    for (auto loader : m_pluginLoaders.values()) {
        loader->unload();
        delete loader;
    }

    // ========== 新增：释放Tab中所有插件UI资源 ==========
    while (ui.tabWidget_PluginUI->count() > 0) {
        QWidget* widget = ui.tabWidget_PluginUI->widget(0);
        ui.tabWidget_PluginUI->removeTab(0);
        delete widget;
    }
}

// 路径选择按钮点击事件
void ToolboxBase::onSelectPathClicked()
{
    // 弹出文件夹选择对话框
    QString selectPath = QFileDialog::getExistingDirectory(
        this,
        "选择目录",
        ui.lineEdit_PluginDir->text().isEmpty() ? QDir::currentPath() : ui.lineEdit_PluginDir->text()
    );

    if (selectPath.isEmpty()) {
        return;
    }

    // 更新LineEdit显示选中的路径
    ui.lineEdit_PluginDir->setText(selectPath);
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
    // 只筛选DLL文件
    QStringList filter;
    filter << "*.dll";
    QFileInfoList fileList = pluginDir.entryInfoList(filter, QDir::Files);

    for (const QFileInfo& fileInfo : fileList) {
        QString pluginPath = fileInfo.absoluteFilePath();
        // 直接加载插件，无需检查json文件
        PluginInterface* plugin = loadPlugin(pluginPath);
        if (plugin) {
            // 插件加载成功，通过接口获取名称
            QStandardItem* item = new QStandardItem(plugin->pluginName());
            // ========== 新增：插件名称不可编辑 ==========
            item->setEditable(false); // 禁止修改插件名称
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
        // ========== 新增：空提示项也不可编辑 ==========
        emptyItem->setEditable(false);
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
    QWidget* pluginWidget = plugin->createPluginWidget(ui.tabWidget_PluginUI);
    if (!pluginWidget) {
        return;
    }

    // 在TabWidget中新建标签页，展示插件UI
    int tabIndex = ui.tabWidget_PluginUI->addTab(pluginWidget, plugin->pluginName());
    ui.tabWidget_PluginUI->setCurrentIndex(tabIndex);
}

// ========== 新增：Tab关闭事件处理函数 ==========
void ToolboxBase::onTabCloseRequested(int index)
{
    // 1. 获取要关闭的Tab对应的Widget
    QWidget* widget = ui.tabWidget_PluginUI->widget(index);
    if (widget) {
        // 2. 移除Tab（先移除再删除，避免UI异常）
        ui.tabWidget_PluginUI->removeTab(index);
        // 3. 释放Widget资源（插件UI）
        delete widget;
        qDebug() << "关闭插件标签页，索引：" << index;
    }

    // 注意：插件DLL不卸载（保留加载器缓存，方便再次打开插件）
    // 如果需要关闭Tab时卸载DLL，可添加逻辑：根据插件名称找到对应的loader，调用unload()
}