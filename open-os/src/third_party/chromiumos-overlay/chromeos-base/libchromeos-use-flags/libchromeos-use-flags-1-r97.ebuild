# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

# This ebuild only cares about its own FILESDIR and ebuild file, so it tracks
# the canonical empty project.
CROS_WORKON_COMMIT="d2d95e8af89939f893b1443135497c1f5572aebc"
CROS_WORKON_TREE="776139a53bc86333de8672a51ed7879e75909ac9"
CROS_WORKON_PROJECT="chromiumos/infra/build/empty-project"
CROS_WORKON_LOCALNAME="platform/empty-project"
CROS_WORKON_OUTOFTREE_BUILD=1

inherit cros-workon

DESCRIPTION="Text file listing USE flags for chromeos-base/libchromeos"

LICENSE="BSD-Google"
# Nothing depends on this package for build info.  All the files are used at
# runtime only by design.
SLOT="0/0"
KEYWORDS="*"

# NB: Flags listed here are off by default unless prefixed with a '+'.
# This list is lengthy since it determines the USE flags that will be written to
# the /etc/ui_use_flags.txt file that's used to generate Chrome's command line.
IUSE="
	allow_consumer_kiosk
	ambient_eq_default_off
	arc
	arc_adb_sideloading
	arc_container_app_killer
	arc_disable_cros_video_decoder
	arc_enable_attestation
	arc_erofs
	arc_force_2x_scaling
	arc_transition_m_to_n
	arc_ureadahead
	arcpp
	arcvm
	arcvm_data_migration
	arcvm_dlc
	arcvm_gki
	arcvm_virtio_blk_data
	arcvm_virtio_blk_multiple_workers
	asan
	avoid_duplicate_begin_frames
	background_blur
	biod
	block_keymint
	borealis_host
	broken_24hours_wake
	camera_feature_effects
	camera_feature_super_res
	cfm_enabled_device
	cheets
	clear_fast_ink_buffer
	compupdates
	coral
	cuttlefish
	default_color_space_rec709
	diagnostics
	disable_cros_video_decoder
	disable_explicit_dma_fences
	disable_native_gpu_memory_buffers
	disable_instant_tethering
	disable_spectre_variant2_mitigation
	disable_use_multiple_overlays
	disable_video_decode_batching
	disable_webrtc_hw_decoding
	drm_atomic
	edge_touch_filtering
	enable_dsp_hotword
	enable_heuristic_palm_detection_filter
	enable_neural_palm_detection_filter
	ferrochrome
	floss
	gpu_sandbox_allow_sysv_shm
	gpu_sandbox_failures_not_fatal
	gpu_sandbox_start_early
	houdini
	houdini64
	houdini_dlc
	ipcz
	kvm_guest
	kvm_host
	legacy_keyboard
	legacy_power_button
	lvm_application_containers
	mantis
	ml_service
	moblab
	mojo_service_manager
	native_gpu_memory_buffers
	natural_scroll_default
	ndk_translation
	ndk_translation64
	neon
	odml_xs_base_model
	ondevice_document_scanner
	ondevice_document_scanner_dlc
	ondevice_grammar
	ondevice_handwriting
	ondevice_handwriting_dlc
	ondevice_image_content_annotation
	ondevice_speech
	oobe_skip_postlogin
	oobe_skip_to_login
	opengles
	os_install_service
	passive_event_listeners
	pita
	pita-camera
	pita-microphone
	rauru_max_display_limit
	reduce_hardware_video_decoder_buffers
	reven_branding
	rialto
	scheduler_configuration_performance
	screenshare_sw_codec
	set_hw_overlay_strategy_none
	shelf-hotseat
	smartdim
	squid
	tablet_form_factor
	touch_centric_device
	touchscreen_wakeup
	touchview
	tpm_dynamic
	v4l2_flat_stateful_video_decoder
	variable_refresh_rate_default_enabled
	video_capture_use_gpu_memory_buffer
	vulkan_chrome
	wilco
"

src_install() {
	# Install a file containing a list of currently-set USE flags that
	# ChromiumCommandBuilder reads at runtime while constructing Chrome's
	# command line.
	local path="${WORKDIR}/ui_use_flags.txt"
	cat <<EOF >"${path}"
# This file is just for libchromeos's ChromiumCommandBuilder class.
# Don't use it for anything else. Your code will break.
EOF

	# If you need to use a new flag, add it to $IUSE at the top of the file.
	local flags=( ${IUSE} )
	local flag
	for flag in "${flags[@]/#[-+]}" ; do
		usev "${flag}"
	done | sort -u >>"${path}"

	insinto /etc
	doins "${path}"
}
