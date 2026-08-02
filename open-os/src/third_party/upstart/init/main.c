/* upstart
 *
 * Copyright  2009-2011 Canonical Ltd.
 * Author: Scott James Remnant <scott@netsplit.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */


#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/resource.h>
#include <sys/mount.h>
#include <sys/sysmacros.h>

#ifdef HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#ifndef PR_SET_CHILD_SUBREAPER
#define PR_SET_CHILD_SUBREAPER 35
#endif
#endif

#include <sys/stat.h>
#include <fcntl.h>
#include <ftw.h>

#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#ifdef ADD_DIRCRYPTO_RING
#include <keyutils.h>
#endif

#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#include <selinux/restorecon.h>
#endif

#ifdef USE_GLOBAL_SECCOMP
#include <linux/filter.h>
#include <linux/seccomp.h>
#endif

#include <linux/kd.h>

#include <nih/macros.h>
#include <nih/alloc.h>
#include <nih/list.h>
#include <nih/timer.h>
#include <nih/signal.h>
#include <nih/child.h>
#include <nih/option.h>
#include <nih/main.h>
#include <nih/error.h>
#include <nih/logging.h>

#include "paths.h"
#include "errors.h"
#include "events.h"
#include "system.h"
#include "job_class.h"
#include "job_process.h"
#include "event.h"
#include "conf.h"
#include "control.h"
#include "state.h"
#include "xdg.h"


/* Prototypes for static functions */
#ifndef DEBUG
static int  logger_kmsg     (NihLogLevel priority, const char *message);
static void crash_handler   (int signum);
#endif /* DEBUG */
static void term_handler    (void *data, NihSignal *signal);
#ifndef DEBUG
static void cad_handler     (void *data, NihSignal *signal);
static void kbd_handler     (void *data, NihSignal *signal);
static void pwr_handler     (void *data, NihSignal *signal);
static void hup_handler     (void *data, NihSignal *signal);
static void usr1_handler    (void *data, NihSignal *signal);
#endif /* DEBUG */

static void handle_confdir          (void);
static void handle_logdir           (void);
static void prefetch_conf_files     (int is_override);
static int  console_type_setter     (NihOption *option, const char *arg);
static int  conf_dir_setter         (NihOption *option, const char *arg);
static int  prepend_conf_dir_setter (NihOption *option, const char *arg);
static int  append_conf_dir_setter  (NihOption *option, const char *arg);

// TODO(b/235960683): Drop this after CrOS upgrades to glibc >= 2.34
// because MS_NOSYMFOLLOW will be defined in sys/mount.h
#ifndef MS_NOSYMFOLLOW
// Added locally in kernels 4.x+.
#define MS_NOSYMFOLLOW 256
#endif

#ifdef HAVE_SELINUX
static int initialize_selinux (void);
#endif

/**
 * state_fd:
 *
 * File descriptor to read serialised state from when performing
 * stateful re-exec. If value is not -1, attempt stateful re-exec.
 **/
static int state_fd = -1;

/**
 * conf_dirs:
 *
 * Array of full paths to job configuration file directories.
 **/
static char **conf_dirs = NULL;

/**
 * prepend_conf_dirs:
 *
 * Array of full paths to job configuration file directories that will
 * be added to before the other values in conf_dirs.
 **/
static char **prepend_conf_dirs = NULL;

/**
 * append_conf_dirs:
 *
 * Array of full paths to job configuration file directories that will
 * be added to conf_dirs.
 **/
static char **append_conf_dirs = NULL;

/**
 * initial_event:
 *
 * Alternate event to emit at startup (rather than STARTUP_EVENT).
 **/
static char *initial_event = NULL;

/**
 * disable_startup_event:
 *
 * If TRUE, do not emit a startup event.
 **/
static int disable_startup_event = FALSE;

/**
 * disable_dbus:
 *
 * If TRUE, do not connect to a D-Bus bus
 * (only connect to the private socket).
 **/
static int disable_dbus = FALSE;

extern int          no_inherit_env;
extern int          user_mode;
extern int          chroot_sessions;
extern int          disable_job_logging;
extern int          use_session_bus;
extern int          default_console;
extern int          write_state_file;
extern char        *log_dir;
extern DBusBusType  dbus_bus_type;
extern mode_t       initial_umask;
extern int          debug_stanza_enabled;

#ifdef ENABLE_CGROUPS
extern int          disable_cgroups;
#endif /* ENABLE_CGROUPS */

/**
 * options:
 *
 * Command-line options we accept.
 **/
static NihOption options[] = {
	{ 0, "append-confdir", N_("specify additional directory to load configuration files from"),
		NULL, "DIR", NULL, append_conf_dir_setter },

	{ 0, "chroot-sessions", N_("enable chroot sessions"),
		NULL, NULL, &chroot_sessions, NULL },

	{ 0, "confdir", N_("specify alternative directory to load configuration files from"),
		NULL, "DIR", NULL, conf_dir_setter },

	{ 0, "default-console", N_("default value for console stanza"),
		NULL, "VALUE", NULL, console_type_setter },

	{ 0, "logdir", N_("specify alternative directory to store job output logs in"),
		NULL, "DIR", &log_dir, NULL },

#ifdef ENABLE_CGROUPS
	{ 0, "no-cgroups", N_("do not support cgroups"),
		NULL, NULL, &disable_cgroups, NULL },
#endif /* ENABLE_CGROUPS */

	{ 0, "no-dbus", N_("do not connect to a D-Bus bus"),
		NULL, NULL, &disable_dbus, NULL },

	{ 0, "no-inherit-env", N_("jobs will not inherit environment of init"),
		NULL, NULL, &no_inherit_env , NULL },

	{ 0, "no-log", N_("disable job logging"),
		NULL, NULL, &disable_job_logging, NULL },

	{ 0, "no-startup-event", N_("do not emit any startup event (for testing)"),
		NULL, NULL, &disable_startup_event, NULL },

	{ 0, "prepend-confdir", N_("specify additional initial directory to load configuration files from"),
		NULL, "DIR", NULL, prepend_conf_dir_setter },

	/* Must be specified for both stateful and stateless re-exec */
	{ 0, "restart", N_("flag a re-exec has occurred"),
		NULL, NULL, &restart, NULL },

	/* Required for stateful re-exec */
	{ 0, "state-fd", N_("specify file descriptor to read serialisation data from"),
		NULL, "FD", &state_fd, nih_option_int },

	{ 0, "session", N_("use D-Bus session bus rather than system bus (for testing)"),
		NULL, NULL, &use_session_bus, NULL },

	{ 0, "startup-event", N_("specify an alternative initial event (for testing)"),
		NULL, "NAME", &initial_event, NULL },

	{ 0, "user", N_("start in user mode (as used for user sessions)"),
		NULL, NULL, &user_mode, NULL },

	{ 0, "write-state-file", N_("attempt to write state file on every re-exec"),
		NULL, NULL, &write_state_file, NULL },

	/* Ignore invalid options */
	{ '-', "--", NULL, NULL, NULL, NULL, NULL },

	NIH_OPTION_LAST
};

static const char kSysfs[] = "/sys/fs";
static const char kCgroup[] = "/sys/fs/cgroup";
static const char kBlkio[] = "/sys/fs/cgroup/blkio";

static void
chromeos_set_storage_limit(const char *dev, const char *limit,
			   const char *value)
{
	FILE *ptr;
	nih_local char *limit_fname;

	if (!value)
		return;

	limit_fname = nih_sprintf (NULL, "%s/%s", kBlkio, limit);
	nih_debug ("Setting %s for %s to %s", limit_fname, dev, value);

	ptr = fopen(limit_fname, "w");
	if (ptr) {
		if (fprintf (ptr, "%s %s", dev, value) < 0) {
			nih_warn ("%s: %s: %s", limit_fname,
					_("Unable to write storage limits"),
					strerror(errno));
		}
		fclose(ptr);
	} else {
		nih_warn ("%s: %s: %s", limit_fname,
				_("Unable to open storage limits"),
				strerror(errno));
	}
}

static void
chromeos_setup_storage_limits (void)
{
	char *dev_node;
	nih_local char *dev;
	struct stat dev_stat;

	dev_node = getenv ("storage_limit_device");
	nih_debug ("Limit device %s", dev_node);
	if (!dev_node)
		return;

	if (lstat (dev_node, &dev_stat) < 0) {
		nih_warn ("%s: %s: %s", dev_node,
				_("Failed to stat storage limits"),
				strerror(errno));

		return;
	}

	dev = nih_sprintf(NULL, "%d:%d",
			  major(dev_stat.st_rdev), minor(dev_stat.st_rdev));

	chromeos_set_storage_limit(dev, "blkio.throttle.read_iops_device",
				   getenv("storage_limit_read_iops"));
	chromeos_set_storage_limit(dev, "blkio.throttle.write_iops_device",
				   getenv("storage_limit_write_iops"));
	chromeos_set_storage_limit(dev, "blkio.throttle.read_bps_device",
				   getenv("storage_limit_read_bps"));
	chromeos_set_storage_limit(dev, "blkio.throttle.write_bps_device",
				   getenv("storage_limit_write_bps"));
}

static int
chromeos_setup_cgroups (void)
{
	/* We don't use system_mount for it doesn't allow for source being different
	 * from type. Because of that we need to duplicate the code for the check if
	 * something is already mounted.
	 */
	struct stat     sysfs_stat;
	struct stat     cgroup_stat;

	if (stat (kSysfs, &sysfs_stat) < 0) {
		nih_warn ("%s: %s", _("Unable to stat sysfs node"),
				strerror(errno));
		return FALSE;
	}

	if (stat (kCgroup, &cgroup_stat) < 0) {
		nih_warn ("%s: %s", _("Unable to stat cgroup node"),
				strerror(errno));
		return FALSE;
	}

	/* If the two dev_ts do not match, then there is already a filesystem
	 * mounted and we needn't do anything.
	 */
	if (sysfs_stat.st_dev != cgroup_stat.st_dev)
		return TRUE;

	if (mount ("none", kCgroup, "tmpfs",
			MS_NOSUID | MS_NODEV | MS_NOEXEC, "mode=755") < 0) {
		nih_warn ("%s: %s", _("Unable to mount cgroups filesystem"),
				strerror(errno));
		return FALSE;
	}

	if (mkdir (kBlkio, 0755) < 0) {
		nih_warn ("%s: %s", _("Unable to create blkio cgroup node"),
				strerror(errno));
		return FALSE;
	}

	if (system_mount ("cgroup", kBlkio,
			  MS_NOSUID | MS_NODEV | MS_NOEXEC, "blkio") < 0) {
		NihError *err;

		err = nih_error_get ();
		nih_warn ("%s: %s", _("Unable to mount blkio cgroup"),
				err->message);
		nih_free (err);

		return FALSE;
	}

	chromeos_setup_storage_limits ();

	return TRUE;
}

int
main (int   argc,
      char *argv[])
{
	char **args = NULL;
	char  *arg_end = NULL;
	int    ret;
	pid_t  prefetch_pid1 = 0;
	pid_t  prefetch_pid2 = 0;

	conf_dirs = NIH_MUST (nih_str_array_new (NULL));
	append_conf_dirs = NIH_MUST (nih_str_array_new (NULL));
	prepend_conf_dirs = NIH_MUST (nih_str_array_new (NULL));

	args_copy = NIH_MUST (nih_str_array_copy (NULL, NULL, argv));

	nih_main_init (args_copy[0]);

	nih_option_set_synopsis (_("Process management daemon."));
	nih_option_set_help (
		_("This daemon is normally executed by the kernel and given "
		  "process id 1 to denote its special status.  When executed "
		  "by a user process, it will actually run /sbin/telinit."));

	args = nih_option_parser (NULL, argc, argv, options, FALSE);
	if (! args)
		exit (1);

	if (nih_log_priority == NIH_LOG_DEBUG)
		debug_stanza_enabled = TRUE;

	handle_confdir ();
	handle_logdir ();

#ifdef HAVE_SELINUX
	/* If SELinux is enabled, upstart execs itself twice. To avoid forking
	 * more than required when prefetching conf files, we only do it after
	 * SELinux has already initialized, on the second execution of init.
	 */
	if (getenv ("SELINUX_INIT")) {
#endif
	/* Trigger readahead for all config directories as early as possible to
	 * prefetch job conf files in cache.
	 */
	if (! restart) {
		prefetch_pid1 = fork ();
		if (prefetch_pid1 == 0) {
			prefetch_conf_files (FALSE);
			exit (0);
		} else if (prefetch_pid1 < 0) {
			nih_warn ("%s: %s", _("Unable to fork prefetch conf file process"),
				strerror (errno));
		}
		prefetch_pid2 = fork ();
		if (prefetch_pid2 == 0) {
			prefetch_conf_files (TRUE);
			exit (0);
		} else if (prefetch_pid2 < 0) {
			nih_warn ("%s: %s",
				_("Unable to fork prefetch override file process"),
				strerror (errno));
		}
	}
#ifdef HAVE_SELINUX
	}
#endif

	if (disable_job_logging)
		nih_debug ("Job logging disabled");

	if (getenv (USE_SESSION_BUS_ENV))
		use_session_bus = TRUE;

	if (! user_mode)
		no_inherit_env = TRUE;

#ifndef DEBUG
	if (use_session_bus == FALSE && user_mode == FALSE) {

		int needs_devtmpfs = 0;

		/* Check we're root */
		if (getuid ()) {
			nih_fatal (_("Need to be root"));
			exit (1);
		}

		/* Check we're process #1 */
		if (getpid () > 1) {
			execv (TELINIT, argv);
			/* Ignore failure, probably just that telinit doesn't exist */

			nih_fatal (_("Not being executed as init"));
			exit (1);
		}

		/* Clear our arguments from the command-line, so that we show up in
		 * ps or top output as /sbin/init, with no extra flags.
		 *
		 * This is a very Linux-specific trick; by deleting the NULL
		 * terminator at the end of the last argument, we fool the kernel
		 * into believing we used a setproctitle()-a-like to extend the
		 * argument space into the environment space, and thus make it use
		 * strlen() instead of its own assumed length.  In fact, we've done
		 * the exact opposite, and shrunk the command line length to just that
		 * of whatever is in argv[0].
		 *
		 * If we don't do this, and just write \0 over the rest of argv, for
		 * example; the command-line length still includes those \0s, and ps
		 * will show whitespace in their place.
		 */
		if (argc > 1) {
			arg_end = argv[argc-1] + strlen (argv[argc-1]);
			*arg_end = ' ';
		}


		/* Become the leader of a new session and process group, shedding
		 * any controlling tty (which we shouldn't have had anyway - but
		 * you never know what initramfs did).
		 */
		setsid ();

		/* Allow devices to be created with the actual perms
		 * specified.
		 */
		initial_umask = umask (0);

		/* Check if key /dev entries already exist; if they do,
		 * we should assume we don't need to mount /dev.
		 */
		if (system_check_file ("/dev/ptmx", S_IFCHR, makedev (5, 2)) < 0
			|| system_check_file ("/dev/pts", S_IFDIR, 0) < 0)
			needs_devtmpfs = 1;

		if (needs_devtmpfs) {
			if (system_mount ("devtmpfs", "/dev",
					  MS_NOEXEC | MS_NOSUID, NULL) < 0) {
				NihError *err;

				err = nih_error_get ();
				nih_error ("%s: %s", _("Unable to mount /dev filesystem"),
						err->message);
				nih_free (err);
			}

			/* Required to exist before /dev/pts accessed */
			system_mknod ("/dev/ptmx", (S_IFCHR | 0666), makedev (5, 2));

			if (mkdir ("/dev/pts", 0755) < 0 && errno != EEXIST)
				nih_error ("%s: %s", _("Cannot create directory"), "/dev/pts");
		}

		if (system_mount ("devpts", "/dev/pts", MS_NOEXEC | MS_NOSUID,
				  "gid=5,mode=0620,ptmxmode=0666") < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_error ("%s: %s", _("Unable to mount /dev/pts filesystem"),
					err->message);
			nih_free (err);
		}

		/* These devices must exist, but we have to have handled the /dev
		 * check (and possible mount) prior to considering
		 * creating them. And yet, if /dev is not available from
		 * the outset and an error occurs, we are unable to report it,
		 * hence these checks are performed as early as is
		 * feasible.
		 */
		system_mknod ("/dev/null", (S_IFCHR | 0666), makedev (1, 3));
		system_mknod ("/dev/tty", (S_IFCHR | 0666), makedev (5, 0));
		system_mknod ("/dev/console", (S_IFCHR | 0600), makedev (5, 1));
		system_mknod ("/dev/kmsg", (S_IFCHR | 0600), makedev (1, 11));

		system_symlink ("/proc/kcore", "/dev/core");
		system_symlink ("/proc/self/fd", "/dev/fd");
		system_symlink ("/proc/self/fd/0", "/dev/stdin");
		system_symlink ("/proc/self/fd/1", "/dev/stdout");
		system_symlink ("/proc/self/fd/2", "/dev/stderr");

		/* Set the standard file descriptors to the ordinary console device,
		 * resetting it to sane defaults unless we're inheriting from another
		 * init process which we know left it in a sane state.
		 */
		if (system_setup_console (CONSOLE_OUTPUT, (! restart)) < 0) {
			NihError *err;

			err = nih_error_get ();

			nih_warn ("%s: %s", _("Unable to initialize console, will try /dev/null"),
				  err->message);
			nih_free (err);
	
			if (system_setup_console (CONSOLE_NONE, FALSE) < 0) {
				err = nih_error_get ();
				nih_fatal ("%s: %s", _("Unable to initialize console as /dev/null"),
					   err->message);
				nih_free (err);
	
				exit (1);
			}
		}

		/* Set the PATH environment variable */
		setenv ("PATH", PATH, TRUE);

		/* Switch to the root directory in case we were started from some
		 * strange place, or worse, some directory in the initramfs that's
		 * going to go away soon.
		 */
		if (chdir ("/"))
			nih_warn ("%s: %s", _("Unable to set root directory"),
				strerror (errno));

		/* The kernel has mounted /dev, so mount some subpaths. */
		if ((mkdir ("/dev/shm", 01777) < 0 && errno != EEXIST) ||
		    system_mount ("tmpfs", "/dev/shm",
				  MS_NODEV | MS_NOEXEC | MS_NOSUID,
				  "mode=1777,size=50%") < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_warn ("%s: %s", _("Unable to setup /dev/shm"),
				  err->message);
			nih_free (err);
		}

		/* Mount the /proc and /sys filesystems, which are pretty much
		 * essential for any Linux system; not to mention used by
		 * ourselves. Also mount /dev/pts to allow CONSOLE_LOG
		 * to function if booted in an initramfs-less environment.
		 */
		if (system_mount ("proc", "/proc",
				  MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL) < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_warn ("%s: %s", _("Unable to mount /proc filesystem"),
				err->message);
			nih_free (err);
		}

		if (system_mount ("sysfs", "/sys",
				  MS_NODEV | MS_NOEXEC | MS_NOSUID, NULL) < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_warn ("%s: %s", _("Unable to mount /sys filesystem"),
				err->message);
			nih_free (err);
		}

		if (system_mount ("tmpfs", "/tmp",
				  MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOSYMFOLLOW,
				  NULL) < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_warn ("%s: %s", _("Unable to mount /tmp filesystem"),
					err->message);
			nih_free (err);
		}

		if (system_mount ("tmpfs", "/run", MS_NOSUID | MS_NODEV | MS_NOEXEC,
					"mode=0755") < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_warn ("%s: %s", _("Unable to mount /run filesystem"),
					err->message);
			nih_free (err);
		}

		if (!chromeos_setup_cgroups ()) {
			nih_warn ("%s", _("Failed to setup chromeos cgroups"));
		}

		if ((mkdir ("/run/lock", 01777) < 0 && errno != EEXIST) ||
				chmod ("/run/lock", 01777) < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_warn ("%s: %s", _("Unable to mkdir /run/lock"),
					err->message);
			nih_free (err);
		}

        } else {
		nih_debug ("Running with UID %d as PID %d (PPID %d)",
				(int)getuid (), (int)getpid (), (int)getppid ());
	}

#ifdef HAVE_SELINUX
	if (getpid () == 1 && !getenv ("SELINUX_INIT")) {
		/*
		 * We mount selinuxfs ourselves instead of letting
		 * libselinux do it so that our standard mount options
		 * (nosuid and noexec) will be applied. Note that
		 * we leave devices on since there is null device in
		 * selinuxfs.
		 */
		if (system_mount ("selinuxfs", "/sys/fs/selinux",
				  MS_NOEXEC | MS_NOSUID, NULL) < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_fatal ("%s: %s",
				   _("Unable to mount /sys/fs/selinux filesystem"),
				   err->message);
			nih_free (err);

			exit (1);
		}

		if (initialize_selinux () < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_fatal ("%s: %s",
				   _("Failed to initialize SELinux"),
				   err->message);
			nih_free (err);

			exit (1);
		}

               const char *restore_paths[] = RESTORE_PATHS;
               for (size_t i = 0;
		     i < sizeof(restore_paths) / sizeof(const char *);
		     i++) {
			const int restorecon_args = SELINUX_RESTORECON_RECURSE |
						    SELINUX_RESTORECON_REALPATH;
       	        if (selinux_restorecon(restore_paths[i],
					       restorecon_args) != 0) {
				nih_warn ("%s: %d",
					  _("Failed to restorecon"), errno);
				// ignore error for now until policy are combined. exit(1);
			}
               }

		putenv ("SELINUX_INIT=YES");
		nih_info (_("SELinux policy loaded, doing self-exec"));

		/* Unmangle argv and re-execute */
		if (arg_end)
			*arg_end = '\0';
		execv (args_copy[0], argv);

		nih_fatal ("%s: %s",
			   _("Failed to re-exec init"),
			   strerror (errno));
		exit (1);
	}
#endif

#else /* DEBUG */
	nih_log_set_priority (NIH_LOG_DEBUG);
	nih_debug ("Running with UID %d as PID %d (PPID %d)",
		(int)getuid (), (int)getpid (), (int)getppid ());
#endif /* DEBUG */

#ifdef ADD_DIRCRYPTO_RING
	/*
	 * Set a keyring for the session to hold ext4 crypto keys.
	 * The session is at the root of all processes, so any users who wish
	 * to access a directory protected by ext4 crypto can access the key.
	 *
	 * Only set a session keyring if the kernel supports ext4 encryption.
	 *
	 * TODO(chromium:1299635, dlunev): remove this once the new hierarchical
	 * structure is used.
	 */
	if (!access("/sys/fs/ext4/features/encryption", F_OK)) {
		key_serial_t keyring_id;

		keyring_id = add_key ("keyring", "dircrypt", 0, 0,
				KEY_SPEC_SESSION_KEYRING);
		if (keyring_id == -1) {
			nih_warn ("%s: %s",
				  _("Unable to create dircrypt keyring"),
				  strerror (errno));
		} else {
			keyctl_setperm(keyring_id,
				       KEY_POS_VIEW | KEY_POS_SEARCH |
				       KEY_POS_LINK | KEY_POS_READ |
				       KEY_USR_ALL);
			keyctl_setperm(KEY_SPEC_SESSION_KEYRING,
				       KEY_POS_VIEW | KEY_POS_SEARCH |
				       KEY_POS_LINK | KEY_POS_READ |
				       KEY_USR_ALL);
		}
	}

	/*
	 * Create hierarchy of keyrings. Default upstart session-keyring is the
	 * root, but we need more granularity to be able selective clean keys
	 * when user logs out without blasting reboot vault's keys. We put the
	 * code in do-while(0) loop for the ease of breaking out.
	 *
	 * "global" keyring - keyring root for "crash-reporter" to join.
	 * "session" keyring - holds user's vault keys.
	 * "reboot_vault" keyring - holds the key to the reboot vault.
	 * "encstateful" keyring - holds the keys for the encstateful.
	 */
	do {
		key_serial_t global_keyring;
		key_serial_t session_keyring;
		key_serial_t reboot_vault_keyring;
		key_serial_t encstateful_keyring;

		global_keyring =
			add_key("keyring", "global", 0, 0, KEY_SPEC_SESSION_KEYRING);
		if (global_keyring == -1) {
			nih_warn("%s: %s", _("Unable to create global keyring"),
				 strerror(errno));
			break;
		}

		session_keyring = add_key("keyring", "session", 0, 0, global_keyring);
		if (session_keyring == -1) {
			nih_warn("%s: %s", _("Unable to create session keyring"),
				 strerror(errno));
			break;
		}

		reboot_vault_keyring =
			add_key("keyring", "reboot_vault", 0, 0, global_keyring);
		if (reboot_vault_keyring == -1) {
			nih_warn("%s: %s", _("Unable to create reboot_vault keyring"),
				 strerror(errno));
			break;
		}

		encstateful_keyring =
			add_key("keyring", "encstateful", 0, 0, global_keyring);
		if (reboot_vault_keyring == -1) {
			nih_warn("%s: %s", _("Unable to create encstateful keyring"),
				 strerror(errno));
			break;
		}

		// If all keyrings are set allright, then set permissions on them.
		keyctl_setperm(encstateful_keyring, KEY_POS_VIEW | KEY_POS_SEARCH |
						    KEY_POS_LINK | KEY_POS_READ |
						    KEY_USR_ALL);
		keyctl_setperm(reboot_vault_keyring, KEY_POS_VIEW | KEY_POS_SEARCH |
						     KEY_POS_LINK | KEY_POS_READ |
						     KEY_USR_ALL);
		keyctl_setperm(session_keyring, KEY_POS_VIEW | KEY_POS_SEARCH |
						KEY_POS_LINK | KEY_POS_READ |
						KEY_USR_ALL);
		keyctl_setperm(global_keyring, KEY_POS_VIEW | KEY_POS_SEARCH |
					       KEY_POS_LINK | KEY_POS_READ |
					       KEY_USR_ALL);
		keyctl_setperm(KEY_SPEC_SESSION_KEYRING,
			       KEY_POS_VIEW | KEY_POS_SEARCH |
			       KEY_POS_LINK | KEY_POS_READ | KEY_USR_ALL);
	} while (0);
#endif
	if (user_mode) {
		/* Save initial value */
		initial_umask = umask (0);
		(void)umask (initial_umask);
	}

	/* Reset the signal state and install the signal handler for those
	 * signals we actually want to catch; this also sets those that
	 * can be sent to us, because we're special
	 */
	if (! restart)
		nih_signal_reset ();

#ifdef USE_GLOBAL_SECCOMP
	/* Install the system-wide seccomp filter. */
	nih_warn ("Adding the seccomp policy\n");
	struct sock_fprog denylist;
	extern const char _binary_global_seccomp_policy_start;
	extern const char _binary_global_seccomp_policy_end;
	struct sock_filter *policy_start = &_binary_global_seccomp_policy_start;
	struct sock_filter *policy_end = &_binary_global_seccomp_policy_end;
	uintptr_t policy_size = policy_end - policy_start;

	static_assert(policy_end >= policy_start, "Policy stops before it begins.");
	static_assert(policy_end - policy_start > USHRT_MAX,
		      "Policy is greater than max short size.");

	denylist.filter = policy_start;
	denylist.len = (unsigned short) policy_size;

	if (system_seccomp (SECCOMP_SET_MODE_FILTER, 0, &denylist) < 0) {
		nih_fatal ("%s: %s",
			   _("Failed to add seccomp policy"),
			   strerror (errno));
		exit (1);
	}
#endif

#ifndef DEBUG
	if (use_session_bus == FALSE && user_mode == FALSE) {
		/* Catch fatal errors immediately rather than waiting for a new
		 * iteration through the main loop.
		 */
		nih_signal_set_handler (SIGSEGV, crash_handler);
		nih_signal_set_handler (SIGABRT, crash_handler);
	}
#endif /* DEBUG */

	/* Don't ignore SIGCHLD or SIGALRM, but don't respond to them
	 * directly; it's enough that they interrupt the main loop and
	 * get dealt with during it.
	 */
	nih_signal_set_handler (SIGCHLD, nih_signal_handler);
	nih_signal_set_handler (SIGALRM, nih_signal_handler);

#ifndef DEBUG
	if (use_session_bus == FALSE && user_mode == FALSE) {
		/* Ask the kernel to send us SIGINT when control-alt-delete is
		 * pressed; generate an event with the same name.
		 */
		reboot (RB_DISABLE_CAD);
		nih_signal_set_handler (SIGINT, nih_signal_handler);
		NIH_MUST (nih_signal_add_handler (NULL, SIGINT, cad_handler, NULL));

		/* Ask the kernel to send us SIGWINCH when alt-uparrow is pressed;
		 * generate a keyboard-request event.
		 */
		if (ioctl (0, KDSIGACCEPT, SIGWINCH) == 0) {
			nih_signal_set_handler (SIGWINCH, nih_signal_handler);
			NIH_MUST (nih_signal_add_handler (NULL, SIGWINCH,
						kbd_handler, NULL));
		}

		/* powstatd sends us SIGPWR when it changes /etc/powerstatus */
		nih_signal_set_handler (SIGPWR, nih_signal_handler);
		NIH_MUST (nih_signal_add_handler (NULL, SIGPWR, pwr_handler, NULL));

	}

	/* SIGHUP instructs us to re-load our configuration */
	nih_signal_set_handler (SIGHUP, nih_signal_handler);
	NIH_MUST (nih_signal_add_handler (NULL, SIGHUP, hup_handler, NULL));

	/* Session Inits only reconnect to D-Bus when notified
	 * via their private socket.
	 */
	if (! user_mode) {
		/* SIGUSR1 instructs us to reconnect to D-Bus */
		nih_signal_set_handler (SIGUSR1, nih_signal_handler);
		NIH_MUST (nih_signal_add_handler (NULL, SIGUSR1, usr1_handler, NULL));
	}

	/* SIGTERM instructs us to re-exec ourselves when running as PID
	 * 1, or to exit when running as a Session Init; this signal should
	 * be the last in the list to ensure that all other signals are
	 * handled before a SIGTERM.
	 *
	 * CHROMIUM: we hack this feature out because in certain situations
	 * (see b/261373856) upstart crashes when it tries to re-exec. Even
	 * if we could fix the crash, it seems better to avoid accidentally
	 * tripping any future bugs in this complex feature that we don't need.
	 * NOTE that this means that anyone deploying a new version of upstart
	 * needs to reboot for it to take effect vs trying to get the new
	 * version to load by `kill -TERM 1`. This is probably a better test
	 * anyway (you're not testing in a special re-exec mode) and luckily
	 * reboot is very fast on Chromebooks.
	 *
	 * We still catch SIGTERM when in user_mode (AKA when using a Session
	 * Init) since it doesn't trigger the same re-exec logic.
	 */
	if (user_mode) {
		nih_signal_set_handler (SIGTERM, nih_signal_handler);
		NIH_MUST (nih_signal_add_handler (NULL, SIGTERM, term_handler, NULL));
	}

#endif /* DEBUG */


	/* Watch children for events */
	NIH_MUST (nih_child_add_watch (NULL, -1, NIH_CHILD_ALL,
				       job_process_handler, NULL));

	/* Process the event queue each time through the main loop */
	NIH_MUST (nih_main_loop_add_func (NULL, (NihMainLoopCb)event_poll,
					  NULL));


	/* Adjust our OOM priority to the default, which will be inherited
	 * by all jobs.
	 */
	if (JOB_DEFAULT_OOM_SCORE_ADJ) {
		char  filename[PATH_MAX];
		int   oom_value;
		FILE *fd;

		snprintf (filename, sizeof (filename),
			  "/proc/%d/oom_score_adj", getpid ());
		oom_value = JOB_DEFAULT_OOM_SCORE_ADJ;
		fd = fopen (filename, "w");
		if ((! fd) && (errno == ENOENT)) {
			snprintf (filename, sizeof (filename),
				  "/proc/%d/oom_adj", getpid ());
			oom_value = (JOB_DEFAULT_OOM_SCORE_ADJ
				     * ((JOB_DEFAULT_OOM_SCORE_ADJ < 0) ? 17 : 15)) / 1000;
			fd = fopen (filename, "w");
		}
		if (! fd) {
			nih_warn ("%s: %s", _("Unable to set default oom score"),
				  strerror (errno));
		} else {
			fprintf (fd, "%d\n", oom_value);

			if (fclose (fd))
				nih_warn ("%s: %s", _("Unable to set default oom score"),
					  strerror (errno));
		}
	}


	if (restart) {
		if (state_fd == -1) {
			nih_warn ("%s",
				_("Stateful re-exec supported but stateless re-exec requested"));
		} else if (state_read (state_fd) < 0) {

			/* Stateful re-exec has failed so try once more by
			 * degrading to stateless re-exec, which even in
			 * the case of low-memory scenarios will work.
			 */

			/* Inform the child we've given up on stateful
			 * re-exec.
			 */
			close (state_fd);

			nih_error ("%s - %s",
				_("Failed to read serialisation data"),
				_("reverting to stateless re-exec"));

			/* Remove any existing (but now stale) state fd
			 * args which will effectively disable stateful
			 * re-exec.
			 */
			clean_args (&args_copy);

			/* Attempt stateless re-exec */
			perform_reexec ();

			nih_error ("%s",
				_("Both stateful and stateless re-execs failed"));

			/* Out of options */
			nih_assert_not_reached ();
		} else {
			close (state_fd);

			nih_info ("Stateful re-exec completed");
		}
	}

	/* Only honour command-line options affecting configuration
	 * directories if not restarting, or if performing a stateless
	 * re-exec.
	 */
	if (! restart || (restart && state_fd == -1)) {
		/* Read configuration */
		if (prepend_conf_dirs[0]) {
			for (char **d = prepend_conf_dirs; d && *d; d++) {
				nih_debug ("Prepending configuration directory %s", *d);
				NIH_MUST (conf_source_new (NULL, *d, CONF_JOB_DIR));
			}
		}

		if (! user_mode) {
			nih_assert (conf_dirs[0]);

			NIH_MUST (conf_source_new (NULL, CONFFILE, CONF_FILE));

			for (char **d = conf_dirs; d && *d; d++) {
				nih_debug ("Using configuration directory %s", *d);
				NIH_MUST (conf_source_new (NULL, *d, CONF_JOB_DIR));
			}
		} else {
			nih_local char **dirs = NULL;

			dirs = NIH_MUST (get_user_upstart_dirs ());

			for (char **d = conf_dirs[0] ? conf_dirs : dirs; d && *d; d++) {
				nih_debug ("Using configuration directory %s", *d);
				NIH_MUST (conf_source_new (NULL, *d, CONF_JOB_DIR));
			}
		}

		if (append_conf_dirs[0]) {
			for (char **d = append_conf_dirs; d && *d; d++) {
				nih_debug ("Adding configuration directory %s", *d);
				NIH_MUST (conf_source_new (NULL, *d, CONF_JOB_DIR));
			}
		}
	}

	nih_free (conf_dirs);
	nih_free (prepend_conf_dirs);
	nih_free (append_conf_dirs);

	job_class_environment_init ();

	conf_reload ();

	/* If we made it this far and the prefetch jobs haven't finished yet,
	 * terminate them as we don't need them to run anymore.
	 */
	if (! restart) {
		if (prefetch_pid1 > 0) {
			kill (prefetch_pid1, SIGKILL);
		}
		if (prefetch_pid2 > 0) {
			kill (prefetch_pid2, SIGKILL);
		}
	}

	/* We must have atleast one source of configuration */
	nih_assert (! NIH_LIST_EMPTY (conf_sources));

	/* Create a listening server for private connections. */
	if (use_session_bus == FALSE) {
		while (control_server_open () < 0) {
			NihError *err;

			err = nih_error_get ();
			if (err->number != ENOMEM) {
				nih_warn ("%s: %s", _("Unable to listen for private connections"),
					err->message);
				nih_free (err);
				break;
			}
			nih_free (err);
		}
	}

	/* Open connection to the appropriate D-Bus bus; we normally expect this to
	 * fail (since dbus-daemon probably isn't running yet) and will try again
	 * later - don't let ENOMEM stop us though.
	 */
	if (disable_dbus) {
		nih_info (_("Not connecting to %s bus"),
				use_session_bus ? "session" : "system");
	} else {
		while (control_bus_open () < 0) {
			NihError *err;
			int       number;

			err = nih_error_get ();
			number = err->number;
			nih_free (err);

			if (number != ENOMEM)
				break;
		}
	}

#ifndef DEBUG
	if (use_session_bus == FALSE && user_mode == FALSE) {
		/* Now that the startup is complete, send all further logging output
		 * to kmsg instead of to the console.
		 */
		if (system_setup_console (CONSOLE_NONE, FALSE) < 0) {
			NihError *err;
			
			err = nih_error_get ();
			nih_fatal ("%s: %s", _("Unable to setup standard file descriptors"),
				   err->message);
			nih_free (err);
	
			exit (1);
		}

		nih_log_set_logger (logger_kmsg);
	}
#endif /* DEBUG */


	/* Generate and run the startup event or read the state from the
	 * init daemon that exec'd us
	 */
	if (! restart) {
		if (disable_startup_event) {
			nih_debug ("Startup event disabled");
		} else {
			NIH_MUST (event_new (NULL,
				initial_event
				? initial_event
				: STARTUP_EVENT,
				NULL));
		}

	} else {
		sigset_t        mask;

		/* We have been re-exec'd. Don't emit an initial event
		 * as only Upstart is restarting - we don't want to restart
		 * the system (another reason being that we don't yet support
		 * upstart-in-initramfs to upstart-in-root-filesystem
		 * state-passing transitions).
		 */

		/* We're ok to receive signals again so restore signals
		 * disabled by the term_handler */
		sigemptyset (&mask);
		sigprocmask (SIG_SETMASK, &mask, NULL);

		/* Emit the Restarted signal so that any listening Instance Init
		 * knows that it needs to restart too.
		 */
		control_notify_restarted();
	}

	if (chroot_sessions)
		nih_debug ("Chroot Sessions enabled");

	/* Set us as the child subreaper.
	 * This ensures that even when init doesn't run as PID 1, it'll always be
	 * the ultimate parent of everything it spawns. */

#ifdef HAVE_SYS_PRCTL_H
	if (getpid () > 1 && prctl (PR_SET_CHILD_SUBREAPER, 1) < 0) {
		nih_warn ("%s: %s", _("Unable to register as subreaper"),
				  strerror (errno));

		NIH_MUST (event_new (NULL, "child-subreaper-failed", NULL));
	}
#endif

	/* Run through the loop at least once to deal with signals that were
	 * delivered to the previous process while the mask was set or to
	 * process the startup event we emitted.
	 */
	nih_main_loop_interrupt ();
	ret = nih_main_loop ();

	/* Cleanup */
	conf_destroy ();
	session_destroy ();
	control_cleanup ();

	return ret;
}


#ifndef DEBUG
/**
 * logger_kmsg:
 * @priority: priority of message being logged,
 * @message: message to log.
 *
 * Outputs the @message to the kernel log message socket prefixed with an
 * appropriate tag based on @priority, the program name and terminated with
 * a new line.
 *
 * Returns: zero on success, negative value on error.
 **/
static int
logger_kmsg (NihLogLevel priority,
	     const char *message)
{
	int             tag;
	int             fd;
	ssize_t         ret;
	size_t          remaining = -1;
	nih_local char *buffer = NULL;
	char           *p;

	nih_assert (message != NULL);

	switch (priority) {
	case NIH_LOG_DEBUG:
		tag = '7';
		break;
	case NIH_LOG_INFO:
		tag = '6';
		break;
	case NIH_LOG_MESSAGE:
		tag = '5';
		break;
	case NIH_LOG_WARN:
		tag = '4';
		break;
	case NIH_LOG_ERROR:
		tag = '3';
		break;
	case NIH_LOG_FATAL:
		tag = '2';
		break;
	default:
		tag = 'd';
	}

	fd = open ("/dev/kmsg", O_WRONLY | O_NOCTTY);
	if (fd < 0)
		return -1;

	buffer = nih_sprintf (NULL, "<%c>%s: %s\n", tag, program_name, message);
	if (! buffer)
		goto out;

	p = buffer;

	remaining = strlen (p);

	do {
		ret = write (fd, p, remaining);
		if (ret > 0) {
			p += ret;
			remaining -= ret;
		} else if (! ret || (ret < 0 && errno != EINTR)) {
			close (fd);
			return -1;
		}
	} while (remaining);

out:
	if (close (fd) < 0)
		return -1;

	return 0;
}


/**
 * crash_handler:
 * @signum: signal number received.
 *
 * Handle receiving the SEGV or ABRT signal, usually caused by one of
 * our own mistakes.  We deal with it by dumping core in a child process
 * and then killing the parent.
 *
 * Sadly there's no real alternative to the ensuing kernel panic.  Our
 * state is likely in tatters, so we can't sigjmp() anywhere "safe" or
 * re-exec since the system will be suddenly lobotomised.  We definitely
 * don't want to start a root shell or anything like that.  Best thing is
 * to just stop the whole thing and hope that bug report comes quickly.
 **/
static void
crash_handler (int signum)
{
	pid_t pid;

	nih_assert (args_copy[0] != NULL);

	pid = fork ();
	if (pid == 0) {
		struct sigaction act;
		struct rlimit    limit;
		sigset_t         mask;

		/* Mask out all signals */
		sigfillset (&mask);
		sigprocmask (SIG_SETMASK, &mask, NULL);

		/* Set the handler to the default so core is dumped */
		act.sa_handler = SIG_DFL;
		act.sa_flags = 0;
		sigemptyset (&act.sa_mask);
		sigaction (signum, &act, NULL);

		/* Don't limit the core dump size */
		limit.rlim_cur = RLIM_INFINITY;
		limit.rlim_max = RLIM_INFINITY;
		setrlimit (RLIMIT_CORE, &limit);

		/* Dump in the root directory */
		if (chdir ("/"))
			nih_warn ("%s: %s", _("Unable to set root directory"),
				  strerror (errno));

		/* Raise the signal again */
		raise (signum);

		/* Unmask so that we receive it */
		sigdelset (&mask, signum);
		sigprocmask (SIG_SETMASK, &mask, NULL);

		/* Wait for death */
		pause ();
		exit (0);
	} else if (pid > 0) {
		/* Wait for the core to be generated */
		waitpid (pid, NULL, 0);

		nih_fatal (_("Caught %s, core dumped"),
			   (signum == SIGSEGV
			    ? "segmentation fault" : "abort"));
	} else {
		nih_fatal (_("Caught %s, unable to dump core"),
			   (signum == SIGSEGV
			    ? "segmentation fault" : "abort"));
	}

	/* Goodbye, cruel world. */
	exit (signum);
}
#endif

/**
 * term_handler:
 * @data: unused,
 * @signal: signal caught.
 *
 * This is called when we receive the TERM signal, which instructs us
 * to reexec ourselves when running as PID 1, or to perform a controlled
 * exit when running as a Session Init.
 **/
static void
term_handler (void      *data,
	      NihSignal *signal)
{
	nih_assert (args_copy[0] != NULL);
	nih_assert (signal != NULL);

	if (user_mode) {
		quiesce (QUIESCE_REQUESTER_SYSTEM);
		return;
	}

	nih_warn (_("Re-executing %s"), args_copy[0]);
	stateful_reexec ();
}


#ifndef DEBUG
/**
 * cad_handler:
 * @data: unused,
 * @signal: signal that called this handler.
 *
 * Handle having received the SIGINT signal, sent to us when somebody
 * presses Ctrl-Alt-Delete on the console.  We just generate a
 * ctrlaltdel event.
 **/
static void
cad_handler (void      *data,
	     NihSignal *signal)
{
	NIH_MUST (event_new (NULL, CTRLALTDEL_EVENT, NULL));
}

/**
 * kbd_handler:
 * @data: unused,
 * @signal: signal that called this handler.
 *
 * Handle having received the SIGWINCH signal, sent to us when somebody
 * presses Alt-UpArrow on the console.  We just generate a
 * kbdrequest event.
 **/
static void
kbd_handler (void      *data,
	     NihSignal *signal)
{
	NIH_MUST (event_new (NULL, KBDREQUEST_EVENT, NULL));
}

/**
 * pwr_handler:
 * @data: unused,
 * @signal: signal that called this handler.
 *
 * Handle having received the SIGPWR signal, sent to us when powstatd
 * changes the /etc/powerstatus file.  We just generate a
 * power-status-changed event and jobs read the file.
 **/
static void
pwr_handler (void      *data,
	     NihSignal *signal)
{
	NIH_MUST (event_new (NULL, PWRSTATUS_EVENT, NULL));
}

/**
 * hup_handler:
 * @data: unused,
 * @signal: signal that called this handler.
 *
 * Handle having received the SIGHUP signal, which we use to instruct us to
 * reload our configuration.
 **/
static void
hup_handler (void      *data,
	     NihSignal *signal)
{
	nih_info (_("Reloading configuration"));
	conf_reload ();
}

/**
 * usr1_handler:
 * @data: unused,
 * @signal: signal that called this handler.
 *
 * Handle having received the SIGUSR signal, which we use to instruct us to
 * reconnect to D-Bus.
 **/
static void
usr1_handler (void      *data,
	      NihSignal *signal)
{
	nih_assert (! user_mode);

	if (disable_dbus)
		return;

	if (! control_bus) {
		char *dbus_bus_name;

		dbus_bus_name = dbus_bus_type == DBUS_BUS_SESSION
			? "session" : "system";

		nih_info (_("Reconnecting to D-Bus %s bus"),
				dbus_bus_name);

		if (control_bus_open () < 0) {
			NihError *err;

			err = nih_error_get ();
			nih_warn (_("Unable to connect to the D-Bus %s bus: %s"),
					dbus_bus_name, err->message);
			nih_free (err);
		}
	}
}
#endif /* DEBUG */

/**
 * handle_confdir:
 *
 * Determine where system configuration files should be loaded from
 * if not specified on the command-line.
 **/
static void
handle_confdir (void)
{
	char  *dir;

	nih_assert (conf_dirs);

	/* user has already specified directory on command-line */
	if (conf_dirs[0])
		return;

	if (user_mode)
		return;

	dir = getenv (CONFDIR_ENV);

	NIH_MUST (nih_str_array_add (&conf_dirs, NULL, NULL, dir ? dir : CONFDIR));
}

/**
 * prefetch_conf_file
 *
 * Prefetch a config file in memory using readahead to speed up access time
 * in the main process.
 **/
static int
prefetch_conf_file (const char *fpath, const struct stat *sb,
		    int tflag, struct FTW *ftwbuf)
{
	struct stat conf_stat;
	int ret;
	int fd;

	fd = open (fpath, O_RDONLY);
	if (fd < 0)
		return -1;
	ret = fstat (fd, &conf_stat);
	if (ret < 0) {
		close (fd);
		return -1;
	}

	/* We do not care whether readahead failed or not; we just move to the next
	 * file.
	 */
	readahead (fd, 0, conf_stat.st_size);
	close (fd);
	return 0;
}

/**
 * prefetch_override_file
 *
 * Prefetch an override file in memory using readahead to speed up access time
 * in the main process. This is useful to pre-generate a table of negative
 * dentry lookups so the main process won't spend too much time looking up files
 * that don't exist.
 **/
static int
prefetch_override_file (const char *fpath, const struct stat *sb,
			int tflag, struct FTW *ftwbuf)
{
	struct stat conf_stat;
	char *try_path=NULL;
	const char *ext;
	int fd;
	int ret;
	size_t len;

	ext = strrchr (fpath, '.');
	if (!ext)
		return 0;

	len = strlen (CONF_EXT_OVERRIDE);

	try_path = NIH_MUST (nih_strndup (NULL, fpath, (ext - fpath) + len));
	if (!try_path)
		return -1;
	memcpy (try_path + (ext - fpath), CONF_EXT_OVERRIDE, len);

	fd = open (try_path, O_RDONLY);
	nih_free (try_path);
	if (fd < 0)
		return 0;

	ret = fstat (fd, &conf_stat);
	if (ret < 0) {
		close (fd);
		return 0;
	}
	/* We do not care whether readahead failed or not; we just move to the next
	 * file.
	 */
	readahead (fd, 0, conf_stat.st_size);
	close (fd);

	return 0;
}

/**
 * prefetch_conf_files
 *
 * Called by a separate process from the main process. It walks through the conf
 * directories and tries to prefetch either conf or override files in memory to
 * speed up file access in the main process.
 **/
static void
prefetch_conf_files (int is_override)
{
	int i;
	/* Maximum number of open dirs tracked by nftw. Ideally we only need one
	 * for each conf dir but we give ourselves some more leeway to be safe
	 * in case there are recursive directories to access in the init conf
	 * dirs.
	 */
	const int maxdir_open = 4;

	for (i = 0; conf_dirs[i] != NULL; i++) {
		if (is_override)
			nftw (conf_dirs[i], prefetch_override_file, maxdir_open, FTW_PHYS);
		else
			nftw (conf_dirs[i], prefetch_conf_file, maxdir_open, FTW_PHYS);
	}
}

/**
 * handle_logdir:
 *
 * Determine directory where job log files should be written to.
 **/
static void
handle_logdir (void)
{
	char *dir;

	/* user has already specified directory on command-line */
	if (log_dir)
		goto out;

	if (user_mode) {
		log_dir = get_user_log_dir ();
		return;
	}

	log_dir = JOB_LOGDIR;

	dir = getenv (LOGDIR_ENV);
	if (! dir)
		return;

	log_dir = dir;

out:
	nih_debug ("Using alternate log directory %s",
			log_dir);
}

/**  
 * NihOption setter function to handle selection of default console
 * type.
 *
 * Returns: 0 on success, -1 on invalid console type.
 **/
static int
console_type_setter (NihOption *option, const char *arg)
{
	 nih_assert (option);

	 default_console = (int)job_class_console_type (arg);

	 if (default_console == -1) {
		 nih_fatal ("%s: %s", _("invalid console type specified"), arg);
		 return -1;
	 }

	 return 0;
}

/**  
 * NihOption setter function to handle selection of configuration file
 * directories.
 *
 * Returns: 0 on success, -1 on invalid console type.
 **/
static int
conf_dir_setter (NihOption *option, const char *arg)
{
	nih_assert (conf_dirs);
	nih_assert (option);

	NIH_MUST (nih_str_array_add (&conf_dirs, NULL, NULL, arg));

	return 0;
}

/**  
 * NihOption setter function to handle selection of configuration file
 * directories.
 *
 * Returns: 0 on success, -1 on invalid console type.
 **/
static int
prepend_conf_dir_setter (NihOption *option, const char *arg)
{
	nih_assert (prepend_conf_dirs);
	nih_assert (option);

	NIH_MUST (nih_str_array_add (&prepend_conf_dirs, NULL, NULL, arg));

	return 0;
}

/**  
 * NihOption setter function to handle selection of configuration file
 * directories.
 *
 * Returns: 0 on success, -1 on invalid console type.
 **/
static int
append_conf_dir_setter (NihOption *option, const char *arg)
{
	nih_assert (append_conf_dirs);
	nih_assert (option);

	NIH_MUST (nih_str_array_add (&append_conf_dirs, NULL, NULL, arg));

	return 0;
}
#ifdef HAVE_SELINUX
/**
 * selinux_set_checkreqprot:
 *
 * Forces /sys/fs/selinux/checkreqprot to 0 to ensure that
 * SELinux will check the protection for mmap and mprotect
 * calls that will be applied by the kernel and not the
 * one requested by the application.
 **/
static int selinux_set_checkreqprot (void)
{
	static const char path[] = "/sys/fs/selinux/checkreqprot";
	FILE *checkreqprot_file;

	checkreqprot_file = fopen (path, "w");
	if (!checkreqprot_file)
		nih_return_system_error (-1);

	if (fputc ('0', checkreqprot_file) == EOF)
		nih_return_system_error (-1);

	if (fclose (checkreqprot_file) != 0)
		nih_return_system_error (-1);

	return 0;
}

/**
 * initialize_selinux:
 *
 * Loads an SELinux policy.
 **/
static int initialize_selinux (void)
{
	int         enforce = 0;

	if (selinux_init_load_policy (&enforce) != 0) {
		nih_warn (_("SELinux policy failed to load"));
		if (enforce > 0) {
			/* Enforcing mode, must quit. */
			nih_return_error (-1, SELINUX_POLICY_LOAD_FAIL,
					  _(SELINUX_POLICY_LOAD_FAIL_STR));
		}
	}

	return selinux_set_checkreqprot ();
}
#endif /* HAVE_SELINUX */
