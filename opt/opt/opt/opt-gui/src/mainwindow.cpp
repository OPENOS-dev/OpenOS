#include "mainwindow.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QGroupBox>
#include <QFont>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("opt-gui — OPENOS Desktop Package Manager"));
    resize(960, 640);

    m_backend = new OptBackend();
    m_backend->initialize();

    setupUi();

    // Load package list on startup
    onRefresh();
}

MainWindow::~MainWindow()
{
    delete m_backend;
}

// ── UI Setup ──

void MainWindow::setupUi()
{
    setupToolbar();
    setupStatusBar();

    // ── Central widget with splitter ──
    m_splitter = new QSplitter(Qt::Horizontal, this);

    // Left: Package table
    m_packageTable = new QTableView();
    m_model = new PackageListModel(this);
    m_packageTable->setModel(m_model);
    m_packageTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_packageTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_packageTable->setSortingEnabled(true);
    m_packageTable->setAlternatingRowColors(true);
    m_packageTable->verticalHeader()->hide();
    m_packageTable->horizontalHeader()->setStretchLastSection(true);
    m_packageTable->setShowGrid(false);
    m_packageTable->setColumnWidth(PackageListModel::ColName, 180);
    m_packageTable->setColumnWidth(PackageListModel::ColVersion, 100);
    m_packageTable->setColumnWidth(PackageListModel::ColArch, 70);
    m_packageTable->setColumnWidth(PackageListModel::ColSection, 100);
    m_packageTable->setColumnWidth(PackageListModel::ColType, 70);

    connect(m_packageTable->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &MainWindow::onPackageSelected);

    // Right: Detail panel
    m_detailPanel = new QWidget();
    auto *detailLayout = new QVBoxLayout(m_detailPanel);
    detailLayout->setContentsMargins(16, 16, 16, 16);

    // Package name
    m_detailName = new QLabel(tr("Select a package"));
    QFont titleFont = m_detailName->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    m_detailName->setFont(titleFont);
    detailLayout->addWidget(m_detailName);

    // Version + arch
    m_detailVersion = new QLabel();
    detailLayout->addWidget(m_detailVersion);

    // Description
    m_detailDesc = new QLabel();
    m_detailDesc->setWordWrap(true);
    detailLayout->addWidget(m_detailDesc);

    detailLayout->addSpacing(8);

    // Info group
    auto *infoGroup = new QGroupBox(tr("Details"));
    auto *infoLayout = new QVBoxLayout(infoGroup);
    m_detailInfo = new QLabel();
    m_detailInfo->setTextFormat(Qt::PlainText);
    infoLayout->addWidget(m_detailInfo);
    detailLayout->addWidget(infoGroup);

    // Remove button
    m_detailRemoveBtn = new QPushButton(tr("Remove Package"));
    m_detailRemoveBtn->setEnabled(false);
    m_detailRemoveBtn->setStyleSheet(
        "QPushButton { background-color: #d9534f; color: white; padding: 8px; }"
        "QPushButton:hover { background-color: #c9302c; }"
        "QPushButton:disabled { background-color: #ccc; }"
    );
    connect(m_detailRemoveBtn, &QPushButton::clicked, this, &MainWindow::onRemove);
    detailLayout->addWidget(m_detailRemoveBtn);

    detailLayout->addStretch();

    m_splitter->addWidget(m_packageTable);
    m_splitter->addWidget(m_detailPanel);
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 2);

    setCentralWidget(m_splitter);
}

void MainWindow::setupToolbar()
{
    auto *toolbar = addToolBar(tr("Actions"));
    toolbar->setMovable(false);

    auto *installAction = toolbar->addAction(tr("📦 Install"));
    installAction->setToolTip(tr("Install a .opt package"));
    connect(installAction, &QAction::triggered, this, &MainWindow::onInstall);

    auto *removeAction = toolbar->addAction(tr("🗑 Remove"));
    removeAction->setToolTip(tr("Remove selected package"));
    connect(removeAction, &QAction::triggered, this, &MainWindow::onRemove);

    auto *refreshAction = toolbar->addAction(tr("🔄 Refresh"));
    refreshAction->setToolTip(tr("Refresh package list"));
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefresh);

    toolbar->addSeparator();

    auto *buildAction = toolbar->addAction(tr("🔨 Build .opt"));
    buildAction->setToolTip(tr("Build a .opt package from an app directory"));
    connect(buildAction, &QAction::triggered, this, &MainWindow::onBuild);

    auto *validateAction = toolbar->addAction(tr("✅ Validate"));
    validateAction->setToolTip(tr("Validate a .opt package file"));
    connect(validateAction, &QAction::triggered, this, &MainWindow::onValidate);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

// ── Slots ──

void MainWindow::onInstall()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select .opt Package"),
        QString(),
        tr("OPT Packages (*.opt);;All Files (*)")
    );

    if (path.isEmpty()) return;

    setStatus(tr("Installing %1...").arg(path));

    OptResult result = m_backend->install(path, false);
    if (result.success) {
        setStatus(tr("✅ Installed successfully"), 5000);
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Install Failed"), result.message);
        setStatus(tr("❌ Install failed: %1").arg(result.message), 8000);
    }
}

void MainWindow::onRemove()
{
    auto current = m_packageTable->currentIndex();
    if (!current.isValid()) {
        QMessageBox::information(this, tr("Remove"), tr("Select a package first."));
        return;
    }

    const PackageInfo *pkg = m_model->packageAt(current.row());
    if (!pkg) return;

    auto reply = QMessageBox::question(
        this,
        tr("Remove Package"),
        tr("Remove %1 v%2?").arg(pkg->name, pkg->version),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) return;

    setStatus(tr("Removing %1...").arg(pkg->name));
    OptResult result = m_backend->remove(pkg->name, false);
    if (result.success) {
        setStatus(tr("✅ Removed %1").arg(pkg->name), 5000);
        clearDetail();
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Remove Failed"), result.message);
        setStatus(tr("❌ Remove failed: %1").arg(result.message), 8000);
    }
}

void MainWindow::onRefresh()
{
    setStatus(tr("Refreshing package list..."));
    QApplication::processEvents();

    m_packages = m_backend->listInstalled();
    m_model->refresh(m_packages);

    statusBar()->showMessage(
        tr("Loaded %1 package(s)").arg(m_packages.size())
    );
}

void MainWindow::onBuild()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select App Directory"),
        QString(),
        QFileDialog::ShowDirsOnly
    );

    if (dir.isEmpty()) return;

    QString outputDir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Output Directory"),
        dir
    );

    setStatus(tr("Building .opt from %1...").arg(dir));

    OptResult result = m_backend->build(dir, outputDir);
    if (result.success) {
        setStatus(tr("✅ Built: %1").arg(result.message), 10000);
        QMessageBox::information(this, tr("Build Complete"),
            tr("Package built successfully:\n%1").arg(result.message));
        onRefresh();
    } else {
        QMessageBox::warning(this, tr("Build Failed"), result.message);
        setStatus(tr("❌ Build failed: %1").arg(result.message), 8000);
    }
}

void MainWindow::onValidate()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Select .opt Package to Validate"),
        QString(),
        tr("OPT Packages (*.opt);;All Files (*)")
    );

    if (path.isEmpty()) return;

    OptResult result = m_backend->validate(path);
    if (result.success) {
        // Parse the returned JSON manifest
        QJsonDocument doc = QJsonDocument::fromJson(result.message.toUtf8());
        if (doc.isObject()) {
            PackageInfo info = PackageInfo::fromJson(doc.object());
            QMessageBox::information(this, tr("✅ Valid Package"),
                tr("Package: %1\nVersion: %2\nArch: %3\nType: %4\n\nThis package is valid.")
                    .arg(info.name, info.version, info.architecture, info.appType));
        }
        setStatus(tr("✅ Package is valid"), 5000);
    } else {
        QMessageBox::warning(this, tr("❌ Invalid Package"), result.message);
        setStatus(tr("❌ Package validation failed: %1").arg(result.message), 8000);
    }
}

void MainWindow::onPackageSelected(const QModelIndex &current, const QModelIndex & /*previous*/)
{
    if (!current.isValid()) {
        clearDetail();
        return;
    }

    const PackageInfo *pkg = m_model->packageAt(current.row());
    showDetail(pkg);
}

void MainWindow::showDetail(const PackageInfo *pkg)
{
    if (!pkg) {
        clearDetail();
        return;
    }

    m_detailName->setText(pkg->name);
    m_detailVersion->setText(
        tr("%1  ·  %2  ·  %3")
            .arg(pkg->version, pkg->architecture, pkg->runtime)
    );
    m_detailDesc->setText(pkg->description);

    QString info;
    info += tr("Section:       %1\n").arg(pkg->section);
    info += tr("License:       %1\n").arg(pkg->license);
    info += tr("App Type:      %1\n").arg(pkg->appType);
    info += tr("Desktop File:  %1\n").arg(pkg->desktopFile);
    info += tr("Maintainer:    %1\n").arg(pkg->maintainer);

    if (!pkg->categories.isEmpty())
        info += tr("Categories:    %1\n").arg(pkg->categories.join(", "));
    if (!pkg->depends.isEmpty())
        info += tr("Depends:       %1\n").arg(pkg->depends.join(", "));
    if (!pkg->permissions.isEmpty())
        info += tr("Permissions:   %1\n").arg(pkg->permissions.join(", "));
    if (!pkg->chromiumFeatures.isEmpty())
        info += tr("Features:      %1\n").arg(pkg->chromiumFeatures.join(", "));

    info += tr("\nPackage Size:  %1\n").arg(pkg->packageSize);
    info += tr("Installed Size: %1").arg(pkg->installedSize);

    m_detailInfo->setText(info);

    m_detailRemoveBtn->setEnabled(true);
    m_detailRemoveBtn->setText(tr("🗑 Remove %1").arg(pkg->name));
}

void MainWindow::clearDetail()
{
    m_detailName->setText(tr("Select a package"));
    m_detailVersion->setText({});
    m_detailDesc->setText({});
    m_detailInfo->setText({});
    m_detailRemoveBtn->setEnabled(false);
    m_detailRemoveBtn->setText(tr("Remove Package"));
}

void MainWindow::setStatus(const QString &text, int timeout)
{
    statusBar()->showMessage(text, timeout);
}
