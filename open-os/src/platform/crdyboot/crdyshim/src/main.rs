// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#![deny(clippy::arithmetic_side_effects)]
#![deny(clippy::indexing_slicing)]
#![deny(clippy::pedantic)]
#![cfg_attr(target_os = "uefi", no_main)]
#![cfg_attr(target_os = "uefi", no_std)]

extern crate alloc;

mod fs;

use alloc::borrow::ToOwned;
use alloc::boxed::Box;
use libcrdy::arch::{Arch, PeFileForCurrentArch};
use libcrdy::entry_point::get_primary_entry_point;
use libcrdy::fs::{FileLoader, FileLoaderImpl, FsError};
use libcrdy::launch::{LaunchError, NextStage};
use libcrdy::logging::initialize_logging;
use libcrdy::nx::{self, NxError};
use libcrdy::page_alloc::{PageAllocationError, ScopedPageAllocation};
use libcrdy::relocation::{relocate_pe_into, RelocationError};
use libcrdy::sbat_revocation::{self, RevocationError};
use libcrdy::tpm::extend_pcr_and_log;
use libcrdy::uefi::{Uefi, UefiImpl};
use libcrdy::util::mib_to_bytes;
use libcrdy::{embed_section, fail_with_fatal_error};
use log::info;
use p256::ecdsa::signature::Verifier;
use sbat::{RevocationSbat, RevocationSbatOwned};
use uefi::boot::{AllocateType, MemoryType};
use uefi::prelude::*;
use uefi::proto::tcg::PcrIndex;
use uefi::runtime::VariableVendor;
use uefi::{cstr16, CStr16, CString16};

/// TPM PCR to measure into.
///
/// This is the same PCR shim uses.
///
/// See also the Linux TPM PCR Registry:
/// <https://uapi-group.org/specifications/specs/linux_tpm_pcr_registry/>
const PCR_INDEX: PcrIndex = PcrIndex(4);

/// Amount of memory to allocate for both the raw executable and the
/// relocated version (each).
///
/// This value does not need to be particularly precise, other than
/// being a multiple of the UEFI page size.
///
/// Choose 2 MiB, which is a small enough amount of memory that we
/// don't need to worry about it, but still much larger than what our
/// next stage actually needs.
const NEXT_STAGE_ALLOCATION_SIZE_IN_BYTES: usize = mib_to_bytes(2);

#[derive(Debug, Eq, PartialEq, thiserror::Error)]
pub enum CrdyshimError {
    /// Failed to get the revocation data.
    #[error("revocation data error")]
    RevocationDataError(#[source] RevocationError),

    /// The current executable is revoked.
    #[error("current image is revoked")]
    SelfRevoked(#[source] RevocationError),

    /// The next stage is revoked.
    #[error("next stage is revoked")]
    NextStageRevoked(#[source] RevocationError),

    /// Failed to allocate memory.
    #[error("failed to allocate memory")]
    Allocation(#[source] PageAllocationError),

    /// Failed to open the boot file system.
    #[error("failed to open the boot file system")]
    BootFileSystemError(#[source] FsError),

    /// Failed to read the next stage executable file.
    #[error("failed to read the next stage executable")]
    ExecutableReadFailed(#[source] FsError),

    /// The signature file name was not successfully created.
    #[error("failed to create the signature file name")]
    InvalidSignatureName,

    /// Failed to read the signature file.
    #[error(
        "failed to read the next stage signature (signature is required with secure boot enabled)"
    )]
    SignatureReadFailed(#[source] FsError),

    /// The signature is not a valid DER-encoded ECSDA signature.
    #[error("signature file is malformed")]
    MalformedSignature,

    /// The next stage did not pass signature validation.
    #[error("signature verification failed")]
    SignatureVerificationFailed,

    /// Failed to relocate a PE executable.
    #[error("failed to relocate the next stage executable")]
    Relocation(#[source] RelocationError),

    /// Failed to parse a PE executable.
    #[error("invalid PE: {0}")]
    InvalidPe(object::Error),

    /// Failed to update memory attributes.
    #[error("failed to set up memory protection")]
    MemoryProtection(#[source] NxError),

    /// Failed to launch the next stage.
    #[error("failed to launch next stage")]
    Launch(#[source] LaunchError),
}

/// Represents the high-level flow of the crdyshim application. Crdyshim
/// has a very linear flow, so control mostly goes through these methods
/// in order.
///
/// This is implemented as a trait to allow for mocking.
#[cfg_attr(test, mockall::automock)]
trait Crdyshim {
    fn update_and_get_revocations(&self) -> Result<RevocationSbatOwned, CrdyshimError>;

    fn self_revocation_check(&self, revocations: &RevocationSbat) -> Result<(), CrdyshimError>;

    fn allocate_pages(
        &self,
        memory_type: MemoryType,
        num_bytes: usize,
    ) -> Result<ScopedPageAllocation, CrdyshimError>;

    fn is_secure_boot_enabled(&self) -> bool;

    fn boot_file_loader(&self) -> Result<Box<dyn FileLoader>, CrdyshimError>;

    /// Get the ECDSA public key used to verify the next stage.
    ///
    /// This function is also where the raw public key data is
    /// embedded. Which key data is embedded depends on the `use_dev_pubkey`
    /// feature. If enabled, the key generated by xtask will be
    /// embedded. Otherwise, the official reven public key will be embedded.
    fn get_public_key(&self) -> p256::ecdsa::VerifyingKey;

    fn extend_pcr_and_log(&self, raw_exe: &[u8]);

    fn next_stage_revocation_check(
        &self,
        raw_exe: &[u8],
        revocations: &RevocationSbat,
    ) -> Result<(), CrdyshimError>;

    fn relocate_pe_into(&self, src: &[u8], dst: &mut [u8]) -> Result<(), CrdyshimError>;

    fn get_entry_point_offset(&self, relocated_exe: &[u8]) -> Result<u32, CrdyshimError>;

    fn update_mem_attrs(&self, relocated_exe: &[u8]) -> Result<(), CrdyshimError>;

    fn launch_next_stage(
        &self,
        relocated_exe: &[u8],
        entry_point_offset: u32,
    ) -> Result<(), CrdyshimError>;
}

/// The real implementation of the `Crdyshim` trait used at runtime.
struct CrdyshimImpl;

impl Crdyshim for CrdyshimImpl {
    fn update_and_get_revocations(&self) -> Result<RevocationSbatOwned, CrdyshimError> {
        sbat_revocation::update_and_get_revocations().map_err(CrdyshimError::RevocationDataError)
    }

    fn self_revocation_check(&self, revocations: &RevocationSbat) -> Result<(), CrdyshimError> {
        sbat_revocation::validate_image(&SBAT, revocations).map_err(CrdyshimError::SelfRevoked)
    }

    fn allocate_pages(
        &self,
        memory_type: MemoryType,
        num_bytes: usize,
    ) -> Result<ScopedPageAllocation, CrdyshimError> {
        ScopedPageAllocation::new(AllocateType::AnyPages, memory_type, num_bytes)
            .map_err(CrdyshimError::Allocation)
    }

    fn is_secure_boot_enabled(&self) -> bool {
        is_secure_boot_enabled(&UefiImpl)
    }

    fn boot_file_loader(&self) -> Result<Box<dyn FileLoader>, CrdyshimError> {
        Ok(Box::new(
            FileLoaderImpl::open_boot_file_system().map_err(CrdyshimError::BootFileSystemError)?,
        ))
    }

    fn get_public_key(&self) -> p256::ecdsa::VerifyingKey {
        // If the `use_dev_pubkey` feature is enabled, use the dev key from
        // vboot_reference/tests/devkeys/uefi/crdyshim-v2.pub.pem
        #[cfg(feature = "use_dev_pubkey")]
        const PUBLIC_KEY_SEC1: &[u8] = include_bytes!("pubkey_dev.sec1");

        // If the `use_dev_pubkey` feature is disabled (the default), use
        // the official reven public key.
        #[cfg(not(feature = "use_dev_pubkey"))]
        const PUBLIC_KEY_SEC1: &[u8] = include_bytes!("pubkey_official.sec1");

        #[cfg(feature = "use_dev_pubkey")]
        info!("embedded public key (dev): {:02x?}", PUBLIC_KEY_SEC1);

        #[cfg(not(feature = "use_dev_pubkey"))]
        info!("embedded public key (official): {:02x?}", PUBLIC_KEY_SEC1);

        // Parse the SEC1 key data as an ECDSA public key.
        //
        // OK to unwrap: the key data is hardcoded at build time.
        p256::ecdsa::VerifyingKey::from_sec1_bytes(PUBLIC_KEY_SEC1).unwrap()
    }

    fn extend_pcr_and_log(&self, raw_exe: &[u8]) {
        extend_pcr_and_log(PCR_INDEX, raw_exe);
    }

    fn next_stage_revocation_check(
        &self,
        raw_exe: &[u8],
        revocations: &RevocationSbat,
    ) -> Result<(), CrdyshimError> {
        let pe = PeFileForCurrentArch::parse(raw_exe).map_err(CrdyshimError::InvalidPe)?;
        sbat_revocation::validate_pe(&pe, revocations).map_err(CrdyshimError::NextStageRevoked)
    }

    fn relocate_pe_into(&self, src: &[u8], dst: &mut [u8]) -> Result<(), CrdyshimError> {
        let pe = PeFileForCurrentArch::parse(src).map_err(CrdyshimError::InvalidPe)?;
        relocate_pe_into(&pe, dst).map_err(CrdyshimError::Relocation)
    }

    fn get_entry_point_offset(&self, relocated_exe: &[u8]) -> Result<u32, CrdyshimError> {
        let pe = PeFileForCurrentArch::parse(relocated_exe).map_err(CrdyshimError::InvalidPe)?;
        Ok(get_primary_entry_point(&pe))
    }

    fn update_mem_attrs(&self, relocated_exe: &[u8]) -> Result<(), CrdyshimError> {
        let pe = PeFileForCurrentArch::parse(relocated_exe).map_err(CrdyshimError::InvalidPe)?;
        nx::update_mem_attrs(&pe).map_err(CrdyshimError::MemoryProtection)
    }

    fn launch_next_stage(
        &self,
        relocated_exe: &[u8],
        entry_point_offset: u32,
    ) -> Result<(), CrdyshimError> {
        let next_stage = NextStage {
            image_data: relocated_exe,
            load_options: &[],
            entry_point_offset,
            log_var_name: cstr16!("CrdyshimLog"),
        };
        unsafe { next_stage.launch() }.map_err(CrdyshimError::Launch)
    }
}

/// Check whether secure boot is enabled or not.
///
/// The firmware communicates secure boot status with a global
/// "SecureBoot" UEFI variable containing a `u8` value. If the value is
/// 0, secure boot is disabled. If the value is 1, secure boot is
/// enabled.
///
/// If the variable cannot be read, or if the value is anything other
/// than 0 or 1, log an error and treat it as secure boot being
/// disabled.
fn is_secure_boot_enabled(uefi: &dyn Uefi) -> bool {
    let mut buf: [u8; 1] = [0];
    match uefi.get_variable(
        cstr16!("SecureBoot"),
        &VariableVendor::GLOBAL_VARIABLE,
        &mut buf,
    ) {
        Ok((len, _)) => {
            if len == 1 {
                match buf[0] {
                    0 => false,
                    1 => true,
                    val => {
                        // Only the values 0 and 1 are valid per the
                        // spec. If the variable contains some other
                        // number, treat it as secure boot being
                        // disabled.
                        info!("unexpected SecureBoot value: {val:x?}");
                        false
                    }
                }
            } else {
                info!("unexpected SecureBoot length: {len}");
                false
            }
        }
        Err(err) => {
            // If the variable cannot be read, treat it as secure boot
            // being disabled.
            info!("failed to read SecureBoot variable: {}", err.status());
            false
        }
    }
}

/// Get the path of an executable in the boot directory for the given
/// name and arch.
fn get_executable_path(name: &CStr16, arch: Arch) -> CString16 {
    let mut path = cstr16!(r"\efi\boot\").to_owned();
    path.push_str(name);
    path.push_str(match arch {
        Arch::Aarch64 => cstr16!("aa64.efi"),
        Arch::Ia32 => cstr16!("ia32.efi"),
        Arch::X86_64 => cstr16!("x64.efi"),
    });
    path
}

// Read the signature file of the next stage and use it to verify that
// the next stage executable was signed by the expected key.
//
// Returns `Ok(())` if and only if verification is successful.
fn validate_next_stage(
    crdyshim: &dyn Crdyshim,
    file_loader: &mut dyn FileLoader,
    exe_path: &CStr16,
    raw_exe: &[u8],
) -> Result<(), CrdyshimError> {
    let public_key = crdyshim.get_public_key();

    // Read the next stage signature.
    let signature_path = fs::replace_final_extension(exe_path, cstr16!("sig"))
        .ok_or(CrdyshimError::InvalidSignatureName)?;
    // The DER-encoded signature doesn't have a fixed length, but it
    // will always fit into a buffer of this size.
    let mut raw_signature = [0; 100];
    let read_size = file_loader
        .read_file_into(&signature_path, &mut raw_signature)
        .map_err(CrdyshimError::SignatureReadFailed)?;
    // OK to unwrap: `read_file_into` would return an error if the file
    // size was bigger than the input buffer.
    let raw_signature = &raw_signature.get(..read_size).unwrap();
    info!("next-stage signature: {:02x?}", raw_signature);
    let signature = p256::ecdsa::Signature::from_der(raw_signature)
        .map_err(|_| CrdyshimError::MalformedSignature)?;

    // Verify the executable's signature.
    public_key
        .verify(raw_exe, &signature)
        .map_err(|_| CrdyshimError::SignatureVerificationFailed)
}

fn load_and_validate_next_stage(
    crdyshim: &dyn Crdyshim,
    next_stage_name: &CStr16,
) -> Result<ScopedPageAllocation, CrdyshimError> {
    let is_secure_boot_enabled = crdyshim.is_secure_boot_enabled();
    info!("secure boot enabled? {}", is_secure_boot_enabled);

    // Allocate space for the raw next stage executable.
    let mut raw_exe_alloc = crdyshim.allocate_pages(
        // Use `LOADER_DATA` because this buffer will not be used
        // for code execution. The executable will be relocated in a
        // separate buffer.
        MemoryType::LOADER_DATA,
        NEXT_STAGE_ALLOCATION_SIZE_IN_BYTES,
    )?;

    // Read the next stage executable.
    let mut file_loader = crdyshim.boot_file_loader()?;
    let exe_path = get_executable_path(next_stage_name, Arch::get_current_exe_arch());
    let exe_size = file_loader
        .read_file_into(&exe_path, &mut raw_exe_alloc)
        .map_err(CrdyshimError::ExecutableReadFailed)?;
    // OK to unwrap: if `read_file_into` succeeded, the size it
    // returns is less than or equal to the buffer length.
    let raw_exe = raw_exe_alloc.get(..exe_size).unwrap();

    // Validate the signature of the next stage. This is allowed to fail
    // if secure boot is off.
    if let Err(err) = validate_next_stage(crdyshim, &mut *file_loader, &exe_path, raw_exe) {
        if is_secure_boot_enabled {
            return Err(err);
        }

        info!("secure boot is not enabled, allowing signature error: {err:?}");
    } else {
        info!("signature successfully verified");
    }

    // Measure the raw executable into the TPM.
    //
    // This measurement must be done on the raw data rather than the
    // relocated version. Relocations depend on where the image is
    // loaded in memory, so the measurement would essentially be random.
    //
    // We measure at this point because we still have access to
    // `raw_exe`. The full `raw_exe_alloc` has extra padding at the end
    // filled with zeroes, which would make the measurement less useful.
    crdyshim.extend_pcr_and_log(raw_exe);

    Ok(raw_exe_alloc)
}

/// The main application.
///
/// The following operations are performed:
/// 1. Get revocations, updating if necessary.
/// 2. Perform the self-revocation check.
/// 3. Load, verify, and execute the next stage.
///
/// This is separated out from `efi_main` so that it can return a
/// `Result` and propagate errors with `?`.
fn run(crdyshim: &dyn Crdyshim) -> Result<(), CrdyshimError> {
    let revocations = crdyshim.update_and_get_revocations()?;

    // IMPORTANT: this self revocation check must happen as early in the
    // program as possible. If a security flaw is found that
    // necessitates a revocation of crdyshim, that revocation can only
    // occur via SBAT if the flaw is _after_ this point, so we want as
    // little code as possible prior to this point.
    crdyshim.self_revocation_check(&revocations)?;

    info!("crdyshim version: {}", env!("CARGO_PKG_VERSION"));

    // Base file name of the next stage. The actual file name will have
    // an arch suffix and extension, e.g. "crdybootx64.efi".
    let next_stage_name = cstr16!("crdyboot");

    // Allocate space for the relocated next stage executable. This is
    // the allocation the next stage will actually run from, so it is
    // allocated as type `LOADER_CODE`.
    let mut relocated_exe_alloc =
        crdyshim.allocate_pages(MemoryType::LOADER_CODE, NEXT_STAGE_ALLOCATION_SIZE_IN_BYTES)?;

    {
        // This loads the next stage executable from a hardcoded path. The
        // executable's signature is also loaded from a hardcoded path, and
        // used to verify that the executable data has been properly signed by
        // the expected key.
        let raw_exe_alloc = load_and_validate_next_stage(crdyshim, next_stage_name)?;

        // Check that the next-stage executable has not been revoked via SBAT.
        crdyshim.next_stage_revocation_check(&raw_exe_alloc, &revocations)?;

        // Relocate the validated raw executable into a new buffer to
        // move sections to the correct offset and apply relocations
        // from the .reloc section.
        crdyshim.relocate_pe_into(&raw_exe_alloc, &mut relocated_exe_alloc)?;
    }

    let entry_point_offset = crdyshim.get_entry_point_offset(&relocated_exe_alloc)?;

    crdyshim.update_mem_attrs(&relocated_exe_alloc)?;

    // Transfer control to the next-stage executable.
    crdyshim.launch_next_stage(&relocated_exe_alloc, entry_point_offset)
}

#[entry]
fn efi_main() -> Status {
    initialize_logging();

    match run(&CrdyshimImpl) {
        Ok(()) => unreachable!("next stage did not take control"),
        Err(err) => {
            fail_with_fatal_error!(err);
        }
    }
}

// Add `.sbat` section to the binary.
//
// See https://github.com/rhboot/shim/blob/main/SBAT.md for details of what
// this section is used for.
embed_section!(SBAT, ".sbat", "../sbat.csv");

#[cfg(test)]
mod tests {
    use super::*;
    use libcrdy::fs::MockFileLoader;
    use libcrdy::uefi::MockUefi;
    use uefi::runtime::VariableAttributes;
    use uefi::Error;

    fn create_mock_for_secure_boot(val: Option<Vec<u8>>) -> MockUefi {
        let mut uefi = MockUefi::new();
        uefi.expect_get_variable()
            .returning(move |name, vendor, buf| {
                assert_eq!(name, cstr16!("SecureBoot"));
                assert_eq!(*vendor, VariableVendor::GLOBAL_VARIABLE);
                assert_eq!(buf.len(), 1);

                let attrs =
                    VariableAttributes::BOOTSERVICE_ACCESS | VariableAttributes::RUNTIME_ACCESS;

                if let Some(val) = &val {
                    if val.is_empty() {
                        Ok((0, attrs))
                    } else if val.len() == 1 {
                        buf[0] = val[0];
                        Ok((1, attrs))
                    } else {
                        Err(Error::new(Status::BUFFER_TOO_SMALL.into(), Some(val.len())))
                    }
                } else {
                    Err(Error::new(Status::NOT_FOUND, None))
                }
            });
        uefi
    }

    /// Test that `get_executable_path` works for both arches.
    #[test]
    fn test_get_executable_path() {
        assert_eq!(
            get_executable_path(cstr16!("abc"), Arch::X86_64),
            cstr16!(r"\efi\boot\abcx64.efi")
        );
        assert_eq!(
            get_executable_path(cstr16!("abc"), Arch::Ia32),
            cstr16!(r"\efi\boot\abcia32.efi")
        );
    }

    /// Test that `is_secure_boot_enabled` returns true if secure boot
    /// is enabled.
    #[test]
    fn test_is_secure_boot_enabled_true() {
        log::set_max_level(log::LevelFilter::Info);
        let uefi = create_mock_for_secure_boot(Some(vec![1]));
        assert_eq!(is_secure_boot_enabled(&uefi), true);
    }

    /// Test that `is_secure_boot_enabled` returns false if secure boot
    /// is disabled.
    #[test]
    fn test_is_secure_boot_enabled_false() {
        log::set_max_level(log::LevelFilter::Info);
        let uefi = create_mock_for_secure_boot(Some(vec![0]));
        assert_eq!(is_secure_boot_enabled(&uefi), false);
    }

    /// Test that `is_secure_boot_enabled` returns false for an invalid
    /// value.
    #[test]
    fn test_is_secure_boot_enabled_invalid_val() {
        log::set_max_level(log::LevelFilter::Info);
        let uefi = create_mock_for_secure_boot(Some(vec![2]));
        assert_eq!(is_secure_boot_enabled(&uefi), false);
    }

    /// Test that `is_secure_boot_enabled` returns false for empty data.
    #[test]
    fn test_is_secure_boot_enabled_empty() {
        log::set_max_level(log::LevelFilter::Info);
        let uefi = create_mock_for_secure_boot(Some(vec![]));
        assert_eq!(is_secure_boot_enabled(&uefi), false);
    }

    /// Test that `is_secure_boot_enabled` returns false for the
    /// variable is too large.
    #[test]
    fn test_is_secure_boot_enabled_too_large() {
        log::set_max_level(log::LevelFilter::Info);
        let uefi = create_mock_for_secure_boot(Some(vec![1, 2]));
        assert_eq!(is_secure_boot_enabled(&uefi), false);
    }

    /// Test that `is_secure_boot_enabled` returns false if the secure
    /// boot variable is missing.
    #[test]
    fn test_is_secure_boot_enabled_not_set() {
        log::set_max_level(log::LevelFilter::Info);
        let uefi = create_mock_for_secure_boot(None);
        assert_eq!(is_secure_boot_enabled(&uefi), false);
    }

    /// Get dev ECDSA public key.
    ///
    /// This corresponds to vboot_reference/tests/devkeys/uefi/crdyshim-v2.pub.pem.
    fn dev_public_key() -> p256::ecdsa::VerifyingKey {
        let key = include_bytes!("pubkey_dev.sec1");
        p256::ecdsa::VerifyingKey::from_sec1_bytes(key).unwrap()
    }

    /// Arbitrary test data for the next stage executable.
    const TEST_EXE: &[u8] = &[b'c'; 1024];

    /// Arbitrary test data for the relocated executable.
    const TEST_EXE_RELOCATED: &[u8] = &[b'd'; 1024];

    /// Arbitrary entry offset for the next stage executable.
    const TEST_EXE_ENTRY_OFFSET: u32 = 192;

    /// ECDSA signature for test kernel data.
    ///
    /// This is a valid signature for `TEST_EXE` using the `dev_public_key` key.
    ///
    /// Generated with:
    /// openssl pkeyutl -sign -rawin -in input_file -inkey crdyshim.priv.pem | xxd -i
    const TEST_EXE_SIGNATURE: &[u8] = &[
        0x30, 0x46, 0x02, 0x21, 0x00, 0xf3, 0xa6, 0x5b, 0xa3, 0x1a, 0xba, 0xfc, 0x6e, 0xcb, 0x21,
        0x6d, 0xe5, 0xad, 0xa7, 0xe1, 0x9b, 0x04, 0x75, 0x57, 0xed, 0xc8, 0x9e, 0xd4, 0x66, 0xd0,
        0xf6, 0x96, 0x21, 0x64, 0x7d, 0x33, 0x50, 0x02, 0x21, 0x00, 0x9f, 0xe1, 0x20, 0xc9, 0xd7,
        0x79, 0x22, 0x7d, 0xc0, 0xfd, 0x74, 0x11, 0x11, 0x28, 0xaa, 0x45, 0xa7, 0xe6, 0xa8, 0xc6,
        0x56, 0xad, 0xfa, 0x36, 0xc7, 0xb5, 0x92, 0xbf, 0x31, 0xd6, 0xa3, 0xa0,
    ];

    /// ECDSA signature for data that _isn't_ `TEST_EXE`.
    ///
    /// This is a valid signature created using the dev crdyshim private
    /// key. See `TEST_EXE_SIGNATURE` for how it was generated; the only
    /// difference is the input file.
    const TEST_OTHER_SIGNATURE: &[u8] = &[
        0x30, 0x46, 0x02, 0x21, 0x00, 0xa5, 0xaa, 0x5b, 0x0c, 0xdd, 0x10, 0x2f, 0xf3, 0xc9, 0xab,
        0x1e, 0xe9, 0xf0, 0xf7, 0xd1, 0x92, 0x46, 0x0d, 0x7e, 0x7c, 0x1f, 0x6f, 0x34, 0x04, 0xe5,
        0xe5, 0x8e, 0x2b, 0x69, 0x32, 0x61, 0x75, 0x02, 0x21, 0x00, 0xd6, 0x20, 0x98, 0x56, 0x2f,
        0x82, 0xe1, 0x5b, 0x7c, 0xf9, 0x75, 0x3f, 0x50, 0xb7, 0xc9, 0x0f, 0xae, 0xf1, 0xac, 0xdd,
        0x5b, 0xc9, 0xcc, 0x34, 0x22, 0x33, 0x8e, 0x6c, 0x0f, 0x75, 0x37, 0x05,
    ];

    fn get_test_revocations() -> RevocationSbatOwned {
        let revocations = b"sbat,1,2023012900\nshim,2\ngrub,3\ngrub.debian,4";
        RevocationSbatOwned::parse(revocations).unwrap()
    }

    fn expect_read_file_exe(file_loader: &mut MockFileLoader) {
        file_loader
            .expect_read_file_into()
            .times(1)
            .withf(|path, _| path == cstr16!(r"\efi\boot\crdybootx64.efi"))
            .returning(|_, buf| {
                buf[..TEST_EXE.len()].copy_from_slice(TEST_EXE);
                Ok(TEST_EXE.len())
            });
    }

    fn expect_read_file_sig(file_loader: &mut MockFileLoader, sig: &[u8]) {
        let sig = sig.to_vec();
        file_loader
            .expect_read_file_into()
            .times(1)
            .withf(|path, _| path == cstr16!(r"\efi\boot\crdybootx64.sig"))
            .returning(move |_, buf| {
                buf[..sig.len()].copy_from_slice(&sig);
                Ok(sig.len())
            });
    }

    fn expect_update_and_get_revocations(
        crdyshim: &mut MockCrdyshim,
        result: Result<RevocationSbatOwned, RevocationError>,
    ) {
        crdyshim
            .expect_update_and_get_revocations()
            .times(1)
            .return_once(|| result.map_err(CrdyshimError::RevocationDataError));
    }

    fn expect_self_revocation_check(
        crdyshim: &mut MockCrdyshim,
        result: Result<(), RevocationError>,
    ) {
        crdyshim
            .expect_self_revocation_check()
            .times(1)
            .withf(|r| r == get_test_revocations())
            .return_once(|_| result.map_err(CrdyshimError::SelfRevoked));
    }

    fn expect_allocate_pages(crdyshim: &mut MockCrdyshim) {
        for expected_ty in [MemoryType::LOADER_CODE, MemoryType::LOADER_DATA] {
            crdyshim
                .expect_allocate_pages()
                .times(1)
                .withf(move |ty, size| {
                    (*ty, *size) == (expected_ty, NEXT_STAGE_ALLOCATION_SIZE_IN_BYTES)
                })
                .returning(|ty, size| {
                    Ok(ScopedPageAllocation::new(AllocateType::AnyPages, ty, size).unwrap())
                });
        }
    }

    fn expect_is_secure_boot_enabled(crdyshim: &mut MockCrdyshim, enabled: bool) {
        crdyshim
            .expect_is_secure_boot_enabled()
            .times(1)
            .return_const(enabled);
    }

    fn expect_boot_file_loader(crdyshim: &mut MockCrdyshim, sig: &[u8]) {
        let mut file_loader = MockFileLoader::new();
        expect_read_file_exe(&mut file_loader);
        expect_read_file_sig(&mut file_loader, sig);

        crdyshim
            .expect_boot_file_loader()
            .times(1)
            .return_once(|| Ok(Box::new(file_loader)));
    }

    fn expect_get_public_key(crdyshim: &mut MockCrdyshim) {
        crdyshim
            .expect_get_public_key()
            .times(1)
            .returning(|| dev_public_key());
    }

    fn expect_extend_pcr_and_log(crdyshim: &mut MockCrdyshim) {
        crdyshim
            .expect_extend_pcr_and_log()
            .times(1)
            .withf(|buf| buf == TEST_EXE)
            .return_const(());
    }

    fn expect_next_stage_revocation_check(
        crdyshim: &mut MockCrdyshim,
        result: Result<(), RevocationError>,
    ) {
        crdyshim
            .expect_next_stage_revocation_check()
            .times(1)
            .withf(|buf, revocations| {
                &buf[..TEST_EXE.len()] == TEST_EXE && revocations == get_test_revocations()
            })
            .return_once(|_, _| result.map_err(CrdyshimError::NextStageRevoked));
    }

    fn expect_relocate_pe_into(crdyshim: &mut MockCrdyshim) {
        crdyshim
            .expect_relocate_pe_into()
            .times(1)
            .withf(|src, _| &src[..TEST_EXE.len()] == TEST_EXE)
            .returning(|_, dst| {
                dst[..TEST_EXE_RELOCATED.len()].copy_from_slice(TEST_EXE_RELOCATED);
                Ok(())
            });
    }

    fn expect_get_entry_point_offset(crdyshim: &mut MockCrdyshim) {
        crdyshim
            .expect_get_entry_point_offset()
            .times(1)
            .withf(|buf| &buf[..TEST_EXE_RELOCATED.len()] == TEST_EXE_RELOCATED)
            .returning(|_| Ok(TEST_EXE_ENTRY_OFFSET));
    }

    fn expect_update_mem_attrs(crdyshim: &mut MockCrdyshim) {
        crdyshim
            .expect_update_mem_attrs()
            .times(1)
            .withf(|buf| &buf[..TEST_EXE_RELOCATED.len()] == TEST_EXE_RELOCATED)
            .returning(|_| Ok(()));
    }

    fn expect_launch_next_stage(crdyshim: &mut MockCrdyshim) {
        crdyshim
            .expect_launch_next_stage()
            .times(1)
            .withf(|buf, entry| {
                &buf[..TEST_EXE_RELOCATED.len()] == TEST_EXE_RELOCATED
                    && *entry == TEST_EXE_ENTRY_OFFSET
            })
            .returning(|_, _| Ok(()));
    }

    /// Test that the whole boot flow succeeds with valid data and
    /// secure boot enabled.
    #[test]
    #[cfg_attr(miri, ignore)]
    fn test_succesful_boot_with_secure_boot() {
        log::set_max_level(log::LevelFilter::Info);
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(&mut crdyshim, Ok(get_test_revocations()));
        expect_self_revocation_check(&mut crdyshim, Ok(()));
        expect_allocate_pages(&mut crdyshim);
        expect_is_secure_boot_enabled(&mut crdyshim, true);
        expect_boot_file_loader(&mut crdyshim, TEST_EXE_SIGNATURE);
        expect_get_public_key(&mut crdyshim);
        expect_extend_pcr_and_log(&mut crdyshim);
        expect_next_stage_revocation_check(&mut crdyshim, Ok(()));
        expect_relocate_pe_into(&mut crdyshim);
        expect_get_entry_point_offset(&mut crdyshim);
        expect_update_mem_attrs(&mut crdyshim);
        expect_launch_next_stage(&mut crdyshim);

        run(&crdyshim).unwrap();
    }

    /// Test that boot fails if the getting/updating revocations fails.
    #[test]
    fn test_get_revocation_error() {
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(
            &mut crdyshim,
            Err(RevocationError::UndatedEmbeddedRevocations),
        );

        assert_eq!(
            run(&crdyshim),
            Err(CrdyshimError::RevocationDataError(
                RevocationError::UndatedEmbeddedRevocations
            ))
        );
    }

    /// Test that boot fails if the self-revocation check fails.
    #[test]
    fn test_self_revoked() {
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(&mut crdyshim, Ok(get_test_revocations()));
        expect_self_revocation_check(&mut crdyshim, Err(RevocationError::Revoked));

        assert_eq!(
            run(&crdyshim),
            Err(CrdyshimError::SelfRevoked(RevocationError::Revoked))
        );
    }

    /// Test that signature verification fails with an incorrect signature.
    #[test]
    #[cfg_attr(miri, ignore)]
    fn test_signature_verification_error_with_secure_boot() {
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(&mut crdyshim, Ok(get_test_revocations()));
        expect_self_revocation_check(&mut crdyshim, Ok(()));
        expect_allocate_pages(&mut crdyshim);
        expect_is_secure_boot_enabled(&mut crdyshim, true);
        expect_boot_file_loader(&mut crdyshim, TEST_OTHER_SIGNATURE);
        expect_get_public_key(&mut crdyshim);

        assert_eq!(
            run(&crdyshim),
            Err(CrdyshimError::SignatureVerificationFailed)
        );
    }

    /// Test that signature verification fails with a malformed signature.
    #[test]
    #[cfg_attr(miri, ignore)]
    fn test_signature_malformed_error_with_secure_boot() {
        let invalid_signature = &[];
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(&mut crdyshim, Ok(get_test_revocations()));
        expect_self_revocation_check(&mut crdyshim, Ok(()));
        expect_allocate_pages(&mut crdyshim);
        expect_is_secure_boot_enabled(&mut crdyshim, true);
        expect_boot_file_loader(&mut crdyshim, invalid_signature);
        expect_get_public_key(&mut crdyshim);

        assert_eq!(run(&crdyshim), Err(CrdyshimError::MalformedSignature));
    }

    /// Test that boot fails if the next stage has been revoked.
    #[test]
    #[cfg_attr(miri, ignore)]
    fn test_next_stage_revoked() {
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(&mut crdyshim, Ok(get_test_revocations()));
        expect_self_revocation_check(&mut crdyshim, Ok(()));
        expect_allocate_pages(&mut crdyshim);
        expect_is_secure_boot_enabled(&mut crdyshim, true);
        expect_boot_file_loader(&mut crdyshim, TEST_EXE_SIGNATURE);
        expect_get_public_key(&mut crdyshim);
        expect_extend_pcr_and_log(&mut crdyshim);
        expect_next_stage_revocation_check(&mut crdyshim, Err(RevocationError::Revoked));

        assert_eq!(
            run(&crdyshim),
            Err(CrdyshimError::NextStageRevoked(RevocationError::Revoked))
        );
    }

    /// Test that the whole boot flow succeeds if the signature is
    /// incorrect but secure boot is disabled.
    #[test]
    #[cfg_attr(miri, ignore)]
    fn test_incorrect_signature_without_secure_boot() {
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(&mut crdyshim, Ok(get_test_revocations()));
        expect_self_revocation_check(&mut crdyshim, Ok(()));
        expect_allocate_pages(&mut crdyshim);
        expect_is_secure_boot_enabled(&mut crdyshim, false);
        expect_boot_file_loader(&mut crdyshim, TEST_OTHER_SIGNATURE);
        expect_get_public_key(&mut crdyshim);
        expect_extend_pcr_and_log(&mut crdyshim);
        expect_next_stage_revocation_check(&mut crdyshim, Ok(()));
        expect_relocate_pe_into(&mut crdyshim);
        expect_get_entry_point_offset(&mut crdyshim);
        expect_update_mem_attrs(&mut crdyshim);
        expect_launch_next_stage(&mut crdyshim);

        run(&crdyshim).unwrap();
    }

    /// Test that the whole boot flow succeeds if the signature is
    /// malformed but secure boot is disabled.
    #[test]
    #[cfg_attr(miri, ignore)]
    fn test_malformed_signature_without_secure_boot() {
        let invalid_signature = &[];
        let mut crdyshim = MockCrdyshim::new();

        expect_update_and_get_revocations(&mut crdyshim, Ok(get_test_revocations()));
        expect_self_revocation_check(&mut crdyshim, Ok(()));
        expect_allocate_pages(&mut crdyshim);
        expect_is_secure_boot_enabled(&mut crdyshim, false);
        expect_boot_file_loader(&mut crdyshim, invalid_signature);
        expect_get_public_key(&mut crdyshim);
        expect_extend_pcr_and_log(&mut crdyshim);
        expect_next_stage_revocation_check(&mut crdyshim, Ok(()));
        expect_relocate_pe_into(&mut crdyshim);
        expect_get_entry_point_offset(&mut crdyshim);
        expect_update_mem_attrs(&mut crdyshim);
        expect_launch_next_stage(&mut crdyshim);

        run(&crdyshim).unwrap();
    }
}
