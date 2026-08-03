# Copyright 2012 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"
CROS_WORKON_PROJECT="chromiumos/third_party/autotest"
CROS_WORKON_LOCALNAME="third_party/autotest/files"

inherit cros-workon autotest

DESCRIPTION="Autotest server tests"
HOMEPAGE="https://chromium.googlesource.com/chromiumos/third_party/autotest/"
SRC_URI=""
LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~*"

# Enable autotest by default.
IUSE="
	android-container-rvc
	android-vm-tm
	+autotest
	biod
	+cellular
	-chromeless_tests
	-chromeless_tty
	debugd
	dlc
	minios
	-moblab
	+power_management
	+readahead
	+tpm
	tpm2
	"

DEPEND="${RDEPEND}
	!<chromeos-base/autotest-0.0.2
"

SERVER_IUSE_TESTS="
	+tests_android_ACTS
	+tests_android_EasySetup
	+tests_audio_AudioAfterReboot
	+tests_audio_AudioAfterSuspend
	+tests_audio_AudioArtifacts
	+tests_audio_AudioBasicAssistant
	+tests_audio_AudioBasicBluetoothPlayback
	+tests_audio_AudioBasicBluetoothPlaybackRecord
	+tests_audio_AudioBasicBluetoothRecord
	+tests_audio_AudioBasicExternalMicrophone
	+tests_audio_AudioBasicHDMI
	+tests_audio_AudioBasicHeadphone
	+tests_audio_AudioBasicHotwording
	+tests_audio_AudioBasicInternalMicrophone
	+tests_audio_AudioBasicInternalSpeaker
	+tests_audio_AudioBasicUSBPlayback
	+tests_audio_AudioBasicUSBPlaybackRecord
	+tests_audio_AudioBasicUSBRecord
	+tests_audio_AudioBluetoothConnectionStability
	+tests_audio_AudioNodeSwitch
	+tests_audio_AudioNoiseCancellation
	+tests_audio_AudioPinnedStream
	+tests_audio_AudioQualityAfterSuspend
	+tests_audio_AudioTestAssumptionCheck
	+tests_audio_AudioVolume
	+tests_audio_AudioWebRTCLoopback
	+tests_audio_InternalCardNodes
	+tests_audio_LeftRightInternalSpeaker
	+tests_audio_MediaBasicVerification
	+tests_audio_RebootChameleon
	+tests_autoupdate_Basic
	+tests_autoupdate_CatchBadSignatures
	+tests_autoupdate_Cellular
	+tests_autoupdate_ConsecutiveUpdatesBeforeReboot
	+tests_autoupdate_DataPreserved
	+tests_autoupdate_DeferredUpdate
	+tests_autoupdate_EndToEndTest
	+tests_autoupdate_ForcedOOBEUpdate
	+tests_autoupdate_FromUI
	+tests_autoupdate_Interruptions
	+tests_autoupdate_InvalidateUpdateBeforeReboot
	+tests_autoupdate_Lacros
	+tests_autoupdate_Metered
	+tests_autoupdate_MiniOS
	+tests_autoupdate_NonBlockingOOBEUpdate
	+tests_autoupdate_OmahaResponse
	+tests_autoupdate_P2P
	+tests_autoupdate_Periodic
	+tests_autoupdate_RejectDuplicateUpdate
	+tests_autoupdate_Rollback
	+tests_autoupdate_WithDLC
	+tests_autoupdate_WithFirmware
	+tests_cellular_StaleModemReboot
	+tests_cheets_CTS_R
	+tests_cheets_CTS_T
	+tests_cellular_Callbox_AssertCellularData
	+tests_cellular_Callbox_AssertSMS
	+tests_display_EdidStress
	+tests_display_HotPlugAtBoot
	+tests_display_HotPlugAtSuspend
	+tests_display_LidCloseOpen
	+tests_display_Resolution
	+tests_display_ResolutionList
	+tests_display_ServerChameleonConnection
	+tests_display_SwitchMode
	+tests_factory_Basic
	+tests_firmware_ClearTPMOwnerAndReset
	+tests_firmware_ConsecutiveBoot
	+tests_firmware_CorruptBothFwBodyAB
	+tests_firmware_CorruptBothKernelAB
	+tests_firmware_CorruptMinios
	+tests_firmware_Cr50APROTrigger
	+tests_firmware_Cr50BID
	+tests_firmware_Cr50CCDServoCap
	+tests_firmware_Cr50CCDUartStress
	+tests_firmware_Cr50CheckCap
	+tests_firmware_Cr50ConsoleCommands
	+tests_firmware_Cr50DeepSleepStress
	+tests_firmware_Cr50DeferredECReset
	+tests_firmware_Cr50DeviceState
	+tests_firmware_Cr50ECReset
	+tests_firmware_Cr50FactoryResetVC
	+tests_firmware_Cr50FIPSDS
	+tests_firmware_Cr50GetName
	+tests_firmware_Cr50InvalidateRW
	+tests_firmware_Cr50Keygen
	+tests_firmware_Cr50Open
	+tests_firmware_Cr50OpenTPMRstDebounce
	+tests_firmware_Cr50OpenWhileAPOff
	+tests_firmware_Cr50PartialBoardId
	+tests_firmware_Cr50Password
	+tests_firmware_Cr50PinWeaverServer
	+tests_firmware_Cr50RddG3
	+tests_firmware_Cr50RejectUpdate
	+tests_firmware_Cr50RMAOpen
	+tests_firmware_Cr50SetBoardId
	+tests_firmware_Cr50ShortECC
	+tests_firmware_Cr50Testlab
	+tests_firmware_Cr50U2fCommands
	+tests_firmware_Cr50Unlock
	+tests_firmware_Cr50Update
	+tests_firmware_Cr50UpdateScriptStress
	+tests_firmware_Cr50USB
	+tests_firmware_Cr50VerifyEK
	+tests_firmware_Cr50WilcoEcrst
	+tests_firmware_Cr50WilcoRmaFactoryMode
	+tests_firmware_Cr50WPG3
	+tests_firmware_EmmcWriteLoad
	+tests_firmware_FAFTPrepare
	+tests_firmware_FAFTSetup
	+tests_firmware_Fingerprint
	+tests_firmware_FingerprintCrosConfig
	+tests_firmware_FingerprintSigner
	+tests_firmware_FWMPDisableCCD
	+tests_firmware_FWupdate
	+tests_firmware_FWupdateWP
	+tests_firmware_GSCDSUpdate
	+tests_firmware_GSCEncStateful
	+tests_firmware_GSCFWMPAttrs
	+tests_firmware_GSCPinweaverUpdate
	+tests_firmware_GSCSetAPROV1
	+tests_firmware_GSCStraps
	+tests_firmware_GSCUpdatePCR
	+tests_firmware_RecoveryCacheBootKeys
	+tests_firmware_SetSerialNumber
	+tests_firmware_StandbyPowerConsumption
	+tests_firmware_UpdateKernelDataKeyVersion
	+tests_firmware_WilcoDiagnosticsMode
	+tests_firmware_WriteProtect
	+tests_firmware_WriteProtectFunc
	+tests_fleet_FirmwareUpdate
	+tests_hardware_DiskFirmwareUpgrade
	+tests_hardware_StorageQual
	+tests_hardware_StorageQualBase
	+tests_hardware_StorageQualCheckSetup
	+tests_hardware_StorageQualSuspendStress
	+tests_hardware_StorageQualTrimStress
	+tests_hardware_StorageQualV2
	+tests_hardware_StorageQualV3
	+tests_hardware_StorageStress
	+tests_infra_MultiDutsWithAndroid
	+tests_infra_TLSExecDUTCommand
	+tests_kernel_EmptyLines
	+tests_nbr_EndToEndTest
	+tests_nbr_NetworkInterruptions
	+tests_p2p_EndToEndTest
	+tests_platform_BootDevice
	+tests_platform_BootLockboxServer
	+tests_platform_CorruptRootfs
	+tests_platform_CrashStateful
	+tests_platform_ExternalUsbPeripherals
	+tests_platform_FetchCloudConfig
	+tests_platform_InitLoginPerfServer
	+tests_platform_MTBF
	+tests_platform_SPRITE
	+tests_platform_PowerStatusStress
	+tests_power_WakeSources
	+tests_platform_SyncCrash
	+tests_policy_DeviceServer
	+tests_power_BatteryChargeControl
	+tests_power_BatteryDrainControl
	+tests_power_BrightnessResetAfterReboot
	+tests_power_LW
	+tests_power_ServoChargeStress
	+tests_power_ServodWrapper
	+tests_provision_CheetsUpdate
	+tests_provision_Cr50TOT
	+tests_provision_Cr50Update
	+tests_pvs_Sequence
	+tests_rlz_CheckPing
	+tests_sequences
	+tests_servo_LabControlVerification
	+tests_servo_LabstationVerification
	+tests_servo_USBMuxVerification
	+tests_servo_LogGrab
	+tests_servo_Verification
	+tests_servohost_Reboot
	+tests_stress_EnrollmentRetainment
	+tests_stub_FailServer
	+tests_stub_PassServer
	+tests_stub_ServerToClientPass
	+tests_stub_SynchronousOffloadServer
"

IUSE_TESTS="${IUSE_TESTS}
	${SERVER_IUSE_TESTS}
"

IUSE="${IUSE} ${IUSE_TESTS}"

AUTOTEST_FILE_MASK="*.a *.tar.bz2 *.tbz2 *.tgz *.tar.gz"

INIT_FILE="__init__.py"

src_install() {
	# Make sure we install all |SERVER_IUSE_TESTS| first.
	autotest_src_install
	# Autotest depends on a few strategically placed INIT_FILEs to allow
	# importing python code. In particular we want to allow importing
	# server.site_tests.tast to be able to launch tast local tests.
	# This INIT_FILE exists in git, but needs to be installed and finally
	# packaged via chromite/lib/autotest_util.py into
	# autotest_server_package.tar.bz2 to be served by devservers.
	insinto "${AUTOTEST_BASE}/${AUTOTEST_SERVER_SITE_TESTS}"
	doins "${AUTOTEST_SERVER_SITE_TESTS}/${INIT_FILE}"
}
