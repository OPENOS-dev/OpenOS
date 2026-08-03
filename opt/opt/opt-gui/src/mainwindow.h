#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include "packagelistmodel.h"
#include "optbackend.h"

class QLabel;

/// Main window for opt-gui.
///
/// Layout:
///   ┌──────────────────────────────────────────────┐
///   │ Toolbar: [Install] [Remove] [Refresh] [Build] │
///   ├──────────────────────┬───────────────────────┤
///   │ Package List         │ Detail Panel          │
///   │ (QTableView)         │ (Labels & buttons)    │
///   │                      │                       │
///   ├──────────────────────┴───────────────────────┤
///   │ Status Bar                                   │
///   └──────────────────────────────────────────────┘
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onInstall();
    void onRemove();
    void onRefresh();
    void onBuild();
    void onPackageSelected(const QModelIndex &current, const QModelIndex &previous);
    void onValidate();

private:
    void setupUi();
    void setupToolbar();
    void setupStatusBar();
    void showDetail(const PackageInfo *pkg);
    void clearDetail();
    void setStatus(const QString &text, int timeout = 5000);

    // UI components
    QSplitter      *m_splitter        = nullptr;
    QTableView     *m_packageTable    = nullptr;
    QWidget        *m_detailPanel     = nullptr;

    // Detail panel widgets
    QLabel         *m_detailName      = nullptr;
    QLabel         *m_detailVersion   = nullptr;
    QLabel         *m_detailDesc      = nullptr;
    QLabel         *m_detailInfo      = nullptr;
    QPushButton    *m_detailRemoveBtn = nullptr;

    // Data
    PackageListModel *m_model         = nullptr;
    OptBackend       *m_backend       = nullptr;
    QVector<PackageInfo> m_packages;
};

#endif // MAINWINDOW_H
