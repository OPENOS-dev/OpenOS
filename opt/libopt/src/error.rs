use thiserror::Error;

#[derive(Error, Debug)]
pub enum OptError {
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),

    #[error("ZIP error: {0}")]
    Zip(#[from] zip::result::ZipError),

    #[error("JSON error: {0}")]
    Json(#[from] serde_json::Error),

    #[error("WalkDir error: {0}")]
    WalkDir(#[from] walkdir::Error),

    #[error("Invalid package: {0}")]
    InvalidPackage(String),

    #[error("Missing field: {0}")]
    MissingField(String),

    #[error("Validation error: {0}")]
    Validation(String),

    #[error("Checksum mismatch for {file}: expected {expected}, got {actual}")]
    ChecksumMismatch {
        file: String,
        expected: String,
        actual: String,
    },

    #[error("Dependency not satisfied: {0}")]
    UnsatisfiedDependency(String),

    #[error("Package conflict: {0}")]
    PackageConflict(String),

    #[error("Kernel version insufficient: need >= {need}, have {have}")]
    KernelVersion { need: String, have: String },

    #[error("ChromiumOS feature missing: {0}")]
    MissingFeature(String),

    #[error("Install script failed (exit code {code}): {script}")]
    ScriptFailed { script: String, code: i32 },

    #[error("Package already installed: {name}-{version}")]
    AlreadyInstalled { name: String, version: String },

    #[error("Package not found: {0}")]
    NotFound(String),

    #[error("Permission denied: {0}")]
    PermissionDenied(String),

    #[error("Network error: {0}")]
    Network(String),

    #[error("{0}")]
    General(String),
}

pub type OptResult<T> = Result<T, OptError>;
