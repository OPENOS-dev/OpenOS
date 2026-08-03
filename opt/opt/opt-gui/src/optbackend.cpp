#include "optbackend.h"
#include <QDebug>
#include <dlfcn.h>
#include <cstring>

// ── C FFI declarations (libopt exports) ──
extern "C" {
    struct OptLibResult {
        int    success;
        char  *message;
    };

    OptLibResult  opt_validate(const char *path);
    OptLibResult  opt_install(const char *path, int yes, int no_deps);
    OptLibResult  opt_remove(const char *name, int purge, int yes);
    char         *opt_list_installed();
    char         *opt_info(const char *name);
    OptLibResult  opt_build(const char *path, const char *output, int force);
    void          opt_free_string(char *s);
}

// ── PackageInfo from JSON ──

PackageInfo PackageInfo::fromJson(const QJsonObject &obj)
{
    PackageInfo info;
    info.name          = obj["name"].toString();
    info.version       = obj["version"].toString();
    info.description   = obj["description"].toString();
    info.section       = obj["section"].toString();
    info.maintainer    = obj["maintainer"].toString();
    info.architecture  = obj["architecture"].toString();
    info.license       = obj["license"].toString();
    info.desktopFile   = obj["desktop_file"].toString();
    info.appType       = obj["app_type"].toString();
    info.runtime       = obj["runtime"].toString();
    info.packageSize   = static_cast<qint64>(obj["package_size"].toDouble());
    info.installedSize = static_cast<qint64>(obj["installed_size"].toDouble());

    // Arrays
    for (const auto &v : obj["depends"].toArray())
        info.depends.append(v.toString());
    for (const auto &v : obj["provides"].toArray())
        info.provides.append(v.toString());
    for (const auto &v : obj["permissions"].toArray())
        info.permissions.append(v.toString());
    for (const auto &v : obj["categories"].toArray())
        info.categories.append(v.toString());
    for (const auto &v : obj["chromium_features"].toArray())
        info.chromiumFeatures.append(v.toString());

    return info;
}

// ── OptBackend ──

OptBackend::OptBackend()
{
}

OptBackend::~OptBackend()
{
    if (m_handle) {
        dlclose(m_handle);
    }
}

bool OptBackend::initialize()
{
    // Try loading libopt as a dynamic library first
    // (more flexible for development)
    m_handle = dlopen("liblibopt.dylib", RTLD_NOW | RTLD_GLOBAL);
    if (!m_handle) {
        m_handle = dlopen("libopt.so", RTLD_NOW | RTLD_GLOBAL);
    }
    if (!m_handle) {
        // Fallback: statically linked — symbols are already available
        qDebug() << "libopt: using static linkage";
    }

    m_loaded = true;
    return true;
}

OptResult OptBackend::validate(const QString &path)
{
    QByteArray pathBytes = path.toUtf8();
    auto cres = opt_validate(pathBytes.constData());
    return toResult(cres);
}

OptResult OptBackend::install(const QString &path, bool yes)
{
    QByteArray pathBytes = path.toUtf8();
    auto cres = opt_install(pathBytes.constData(), yes ? 1 : 0, 0);
    return toResult(cres);
}

OptResult OptBackend::remove(const QString &name, bool purge)
{
    QByteArray nameBytes = name.toUtf8();
    auto cres = opt_remove(nameBytes.constData(), purge ? 1 : 0, 1);
    return toResult(cres);
}

QVector<PackageInfo> OptBackend::listInstalled()
{
    QVector<PackageInfo> packages;

    char *jsonStr = opt_list_installed();
    if (!jsonStr) return packages;

    QJsonDocument doc = QJsonDocument::fromJson(QByteArray(jsonStr));

    // Check for error
    if (doc.isObject() && doc.object().contains("error")) {
        qWarning() << "listInstalled error:" << doc.object()["error"].toString();
        opt_free_string(jsonStr);
        return packages;
    }

    if (doc.isArray()) {
        for (const auto &val : doc.array()) {
            packages.append(PackageInfo::fromJson(val.toObject()));
        }
    }

    opt_free_string(jsonStr);
    return packages;
}

OptResult OptBackend::info(const QString &name)
{
    QByteArray nameBytes = name.toUtf8();
    char *jsonStr = opt_info(nameBytes.constData());
    if (!jsonStr) {
        return {false, "null response from libopt"};
    }

    QJsonDocument doc = QJsonDocument::fromJson(QByteArray(jsonStr));

    if (doc.isObject() && doc.object().contains("error")) {
        OptResult r;
        r.success = false;
        r.message = doc.object()["error"].toString();
        opt_free_string(jsonStr);
        return r;
    }

    OptResult r;
    r.success = true;
    r.message = QString::fromUtf8(jsonStr);
    opt_free_string(jsonStr);
    return r;
}

OptResult OptBackend::build(const QString &path, const QString &output)
{
    QByteArray pathBytes = path.toUtf8();
    QByteArray outBytes  = output.toUtf8();
    const char *outPtr   = output.isEmpty() ? nullptr : outBytes.constData();

    auto cres = opt_build(pathBytes.constData(), outPtr, 0);
    return toResult(cres);
}

OptResult OptBackend::toResult(const OptLibResult &cres)
{
    OptResult r;
    r.success = (cres.success == 0);
    if (cres.message) {
        r.message = QString::fromUtf8(cres.message);
        opt_free_string(cres.message);
    }
    return r;
}
