#include "packagelistmodel.h"

PackageListModel::PackageListModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PackageListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_packages.size();
}

int PackageListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColumnCount;
}

QVariant PackageListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_packages.size())
        return {};

    const auto &pkg = m_packages[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColName:    return pkg.name;
        case ColVersion: return pkg.version;
        case ColArch:    return pkg.architecture;
        case ColSection: return pkg.section;
        case ColType:    return pkg.appType;
        case ColSize: {
            if (pkg.installedSize < 1024)
                return QString("%1 B").arg(pkg.installedSize);
            else if (pkg.installedSize < 1024 * 1024)
                return QString("%1 KB").arg(pkg.installedSize / 1024);
            else
                return QString("%1 MB").arg(pkg.installedSize / (1024 * 1024));
        }
        }
    }

    if (role == Qt::ToolTipRole) {
        return pkg.description;
    }

    // Store the index so detail widget can retrieve it
    if (role == Qt::UserRole) {
        return index.row();
    }

    return {};
}

QVariant PackageListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case ColName:    return tr("Package");
    case ColVersion: return tr("Version");
    case ColArch:    return tr("Arch");
    case ColSection: return tr("Section");
    case ColType:    return tr("Type");
    case ColSize:    return tr("Size");
    }
    return {};
}

void PackageListModel::refresh(const QVector<PackageInfo> &packages)
{
    beginResetModel();
    m_packages = packages;
    endResetModel();
}

const PackageInfo *PackageListModel::packageAt(int row) const
{
    if (row < 0 || row >= m_packages.size())
        return nullptr;
    return &m_packages[row];
}
