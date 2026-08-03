// libopt — OPENOS Desktop Package Manager Core Library
//
// This crate provides the core logic for .opt package management:
// - Package format parsing & validation
// - .opt package building from app directories
// - Package installation & removal on ChromiumOS
// - Repository index management
//
// It is consumed by:
//   - opt-cli (Rust CLI, thin wrapper)
//   - opt-gui (C++ Qt, via C FFI in c_api module)

pub mod format;
pub mod error;
pub mod builder;
pub mod installer;
pub mod repo;
pub mod importer;
pub mod c_api;

pub use format::PackageManifest;
pub use format::RepositoryIndex;
pub use format::Checksums;
pub use format::AppType;
pub use format::Runtime;
pub use error::OptError;
pub use error::OptResult;
