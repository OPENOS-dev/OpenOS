#ifndef OPTBACKEND_H
#define OPTBACKEND_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QVector>
#include <functional>

/// Package info structure (mirrors PackageManifest in Rust)
struct PackageInfo {
    QString name;
    QString version;
    QString description;
    QString section;
    QString maintainer;
    QString architecture;
    QString license;
    QString desktopFile;
    QString appType;
    QString runtime;
    qint64  packageSize = 0;
    qint64  installedSize = 0;
    QStringList depends;
    QStringList provides;
    QStringList permissions;
    QStringList categories;
    QStringList chromiumFeatures;

    /// Parse from JSON returned by libopt
    static PackageInfo fromJson(const QJsonObject &obj);
};

/// Result returned by blocking libopt calls
struct OptResult {
    bool    success = false;
    QString message;
};

/// C wrapper structure matching libopt's OptResultC
#pragma pack(push, 1)
struct OptResultC {
    int         success;
    char       *message;
};
#pragma pack(pop)

/// C FFI function pointers (loaded from libopt)
struct OptFFI {
    std::function<OptResultC(const char*)>         validate;
    std::function<OptResultC(const char*, int, int)> install;
    std::function<OptResultC(const char*, int, int)> remove;
    std::function<char*()>                         listInstalled;
    std::function<char*(const char*)>              info;
    std::function<OptResultC(const char*, const char*, int)> build;
    std::function<void(char*)>                     freeString;
};

/// Thin C++ wrapper around libopt's C FFI.
///
/// Handles library loading (cdylib) or static linking,
/// marshals C types to Qt types.
class OptBackend
{
public:
    OptBackend();
    ~OptBackend();

    /// Initialize the backend (load libopt).
    /// Returns false if libopt cannot be loaded.
    bool initialize();

    /// Validate a .opt file. Returns package info on success.
    OptResult validate(const QString &path);

    /// Install a .opt package.
    OptResult install(const QString &path, bool yes);

    /// Remove an installed package.
    OptResult remove(const QString &name, bool purge);

    /// List all installed packages.
    QVector<PackageInfo> listInstalled();

    /// Get detailed info about an installed package.
    OptResult info(const QString &name);

    /// Build a .opt package from an app directory.
    OptResult build(const QString &path, const QString &output);

private:
    void *m_handle = nullptr;
    OptFFI m_ffi;
    bool m_loaded = false;

    /// Load function pointers from shared library.
    template<typename Func>
    Func loadFunc(const char *name);

    /// Convert C result to Qt result and free the C string.
    OptResult toResult(const OptResultC &cres);
};

#endif // OPTBACKEND_H
