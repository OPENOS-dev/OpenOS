#ifndef PACKAGELISTMODEL_H
#define PACKAGELISTMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "optbackend.h"

/// Table model for the installed packages list.
///
/// Columns:
///   0 — Package name
///   1 — Version
///   2 — Architecture
///   3 — Section
///   4 — App type
///   5 — Size
class PackageListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColName = 0,
        ColVersion,
        ColArch,
        ColSection,
        ColType,
        ColSize,
        ColumnCount
    };

    explicit PackageListModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /// Reload package list from backend
    void refresh(const QVector<PackageInfo> &packages);

    /// Get package info at row
    const PackageInfo *packageAt(int row) const;

private:
    QVector<PackageInfo> m_packages;
};

#endif // PACKAGELISTMODEL_H
