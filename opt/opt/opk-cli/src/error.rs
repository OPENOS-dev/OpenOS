// OPK Error types
//
// Centralized error handling using thiserror for all OPK operations.

use thiserror::Error;

#[derive(Error, Debug)]
pub enum OpkError {
    #[error("Package not found: {0}")]
    PackageNotFound(String),

    #[error("No packages specified")]
    NoPackagesSpecified,

    #[error("Repository not found: {0}")]
    RepositoryNotFound(String),

    #[error("Repository unreachable: {0}")]
    RepositoryUnreachable(String),

    #[error("Network error: {0}")]
    NetworkError(String),

    #[error("Download failed for {0}: {1}")]
    DownloadFailed(String, String),

    #[error("Checksum mismatch for {0}")]
    ChecksumMismatch(String),

    #[error("Archive extraction failed: {0}")]
    ExtractionError(String),

    #[error("Dependency conflict: {0} requires {1} but {2} is installed")]
    DependencyConflict(String, String, String),

    #[error("Unmet dependency: {0} requires {1}")]
    UnmetDependency(String, String),

    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),

    #[error("Serialization error: {0}")]
    Serialization(String),

    #[error("Configuration error: {0}")]
    Config(String),

    #[error("Lock error: Another OPK process is running")]
    LockError,

    #[error("{0}")]
    Other(String),
}

impl From<serde_json::Error> for OpkError {
    fn from(err: serde_json::Error) -> Self {
        OpkError::Serialization(err.to_string())
    }
}

impl From<toml::de::Error> for OpkError {
    fn from(err: toml::de::Error) -> Self {
        OpkError::Serialization(err.to_string())
    }
}

impl From<toml::ser::Error> for OpkError {
    fn from(err: toml::ser::Error) -> Self {
        OpkError::Serialization(err.to_string())
    }
}

impl From<ureq::Error> for OpkError {
    fn from(err: ureq::Error) -> Self {
        match err {
            ureq::Error::Status(code, _) => {
                OpkError::NetworkError(format!("HTTP {}", code))
            }
            ureq::Error::Transport(t) => {
                OpkError::NetworkError(t.to_string())
            }
        }
    }
}

impl From<std::io::Error> for OpkError {
    fn from(err: std::io::Error) -> Self {
        OpkError::Io(err)
    }
}
