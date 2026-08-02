/// C FFI layer for libopt
///
/// Exposes a stable C API for use by Qt C++ (opt-gui) and other
/// language bindings. All functions are #[no_mangle] extern "C".
///
/// Memory ownership:
///   - Strings returned via `opt_result_t.message` must be freed
///     with `opt_free_string()`.
///   - `opt_result_t` is returned by value (no heap allocation).

use std::ffi::{CStr, CString};
use std::os::raw::c_char;

use crate::builder;
use crate::installer;

// ──────────────────────────────────────────────
// C-compatible result type
// ──────────────────────────────────────────────

/// C-compatible result.
/// `success`: 0 = OK, 1 = error
/// `message`: owned C string (must be freed with opt_free_string)
#[repr(C)]
pub struct OptResultC {
    pub success: i32,
    pub message: *mut c_char,
}

impl OptResultC {
    fn ok() -> Self {
        let msg = CString::new("").unwrap();
        OptResultC {
            success: 0,
            message: msg.into_raw(),
        }
    }

    fn ok_with_msg(s: &str) -> Self {
        let msg = CString::new(s).unwrap_or_else(|_| CString::new("encoding error").unwrap());
        OptResultC {
            success: 0,
            message: msg.into_raw(),
        }
    }

    fn err(e: impl std::fmt::Display) -> Self {
        let err_str = format!("{}", e);
        let msg = CString::new(err_str).unwrap_or_else(|_| CString::new("unknown error").unwrap());
        OptResultC {
            success: 1,
            message: msg.into_raw(),
        }
    }
}

fn to_cstring(s: &str) -> *mut c_char {
    CString::new(s)
        .unwrap_or_else(|_| CString::new("encoding error").unwrap())
        .into_raw()
}

// ──────────────────────────────────────────────
// C API — Package validation
// ──────────────────────────────────────────────

/// Validate a .opt package file.
/// Returns manifest as JSON on success.
#[no_mangle]
pub extern "C" fn opt_validate(path: *const c_char) -> OptResultC {
    if path.is_null() {
        return OptResultC::err("null path");
    }
    let path_str = match unsafe { CStr::from_ptr(path) }.to_str() {
        Ok(s) => s,
        Err(e) => return OptResultC::err(e),
    };

    match installer::validate_package(path_str) {
        Ok(manifest) => match serde_json::to_string(&manifest) {
            Ok(json) => OptResultC::ok_with_msg(&json),
            Err(e) => OptResultC::err(e),
        },
        Err(e) => OptResultC::err(e),
    }
}

// ──────────────────────────────────────────────
// C API — Package installation
// ──────────────────────────────────────────────

/// Install a .opt package.
#[no_mangle]
pub extern "C" fn opt_install(
    path: *const c_char,
    yes: i32,
    no_deps: i32,
) -> OptResultC {
    if path.is_null() {
        return OptResultC::err("null path");
    }
    let path_str = match unsafe { CStr::from_ptr(path) }.to_str() {
        Ok(s) => s,
        Err(e) => return OptResultC::err(e),
    };

    match installer::install_package(path_str, yes != 0, no_deps != 0) {
        Ok(()) => OptResultC::ok(),
        Err(e) => OptResultC::err(e),
    }
}

// ──────────────────────────────────────────────
// C API — Package removal
// ──────────────────────────────────────────────

/// Remove a .opt package.
#[no_mangle]
pub extern "C" fn opt_remove(
    name: *const c_char,
    purge: i32,
    yes: i32,
) -> OptResultC {
    if name.is_null() {
        return OptResultC::err("null name");
    }
    let name_str = match unsafe { CStr::from_ptr(name) }.to_str() {
        Ok(s) => s,
        Err(e) => return OptResultC::err(e),
    };

    match installer::remove_package(name_str, purge != 0, yes != 0) {
        Ok(()) => OptResultC::ok(),
        Err(e) => OptResultC::err(e),
    }
}

// ──────────────────────────────────────────────
// C API — List installed packages
// ──────────────────────────────────────────────

/// List installed packages (returns JSON array).
/// Caller must free with `opt_free_string`.
#[no_mangle]
pub extern "C" fn opt_list_installed() -> *mut c_char {
    match installer::list_installed() {
        Ok(pkgs) => match serde_json::to_string(&pkgs) {
            Ok(json) => to_cstring(&json),
            Err(e) => to_cstring(&format!("{{\"error\":\"{e}\"}}")),
        },
        Err(e) => to_cstring(&format!("{{\"error\":\"{e}\"}}")),
    }
}

// ──────────────────────────────────────────────
// C API — Package info
// ──────────────────────────────────────────────

/// Get detailed info for an installed package (returns JSON).
/// Caller must free with `opt_free_string`.
#[no_mangle]
pub extern "C" fn opt_info(name: *const c_char) -> *mut c_char {
    if name.is_null() {
        return to_cstring("{\"error\":\"null name\"}");
    }
    let name_str = match unsafe { CStr::from_ptr(name) }.to_str() {
        Ok(s) => s,
        Err(e) => return to_cstring(&format!("{{\"error\":\"{e}\"}}")),
    };

    match installer::show_package(name_str) {
        Ok(manifest) => match serde_json::to_string(&manifest) {
            Ok(json) => to_cstring(&json),
            Err(e) => to_cstring(&format!("{{\"error\":\"{e}\"}}")),
        },
        Err(e) => to_cstring(&format!("{{\"error\":\"{e}\"}}")),
    }
}

// ──────────────────────────────────────────────
// C API — Build .opt package
// ──────────────────────────────────────────────

/// Build a .opt package from an app directory.
/// Returns path to the built .opt file on success.
#[no_mangle]
pub extern "C" fn opt_build(
    path: *const c_char,
    output: *const c_char,
    force: i32,
) -> OptResultC {
    if path.is_null() {
        return OptResultC::err("null path");
    }
    let path_str = match unsafe { CStr::from_ptr(path) }.to_str() {
        Ok(s) => s,
        Err(e) => return OptResultC::err(e),
    };
    let app_dir = std::path::Path::new(path_str);

    let out_dir = if output.is_null() {
        None
    } else {
        match unsafe { CStr::from_ptr(output) }.to_str() {
            Ok(s) => Some(std::path::Path::new(s)),
            Err(e) => return OptResultC::err(e),
        }
    };

    match builder::build_package(app_dir, out_dir, force != 0) {
        Ok(opt_path) => OptResultC::ok_with_msg(&opt_path.to_string_lossy()),
        Err(e) => OptResultC::err(e),
    }
}

// ──────────────────────────────────────────────
// C API — Free strings
// ──────────────────────────────────────────────

/// Free a string previously returned by libopt.
/// Must be called for every `*mut c_char` returned from
/// opt_list_installed() and opt_info().
#[no_mangle]
pub extern "C" fn opt_free_string(s: *mut c_char) {
    if !s.is_null() {
        unsafe {
            let _ = CString::from_raw(s);
        }
    }
}
