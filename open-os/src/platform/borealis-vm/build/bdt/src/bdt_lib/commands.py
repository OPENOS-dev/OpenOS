# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Miscellaneous self-contained bdt functionality."""

import datetime
import glob
import os
import re
import shutil
import signal
import subprocess
import sys
import tarfile
import time

# report() takes an `upload` arg, so we need to rename the `upload` module :(
from . import upload as _upload


LAUNCH_ALIASES = {
    "apitrace": "APITRACE",
    "env": "ENV_DUMP",
    "mangohud": "MANGOHUD",
    "output": "OUTPUT_DUMP",
    "overrides": "GAME_OVERRIDES",
    "proton-debug": "PROTON_DEBUG",
    "strace": "STRACE",
    "pressure-vessel-debug": "PRESSURE_VESSEL_DEBUG",
    "zink": "ZINK",
    "proton-heap-validation": "PROTON_HEAP_VALIDATION",
    "proton-sync-x11": "PROTON_SYNC_X11",
    "proton-esync": "PROTON_ESYNC",
    "proton-wineserver-sync": "PROTON_WINESERVER_SYNC",
    "venus-debug": "VENUS_DEBUG",
    "venus-dump": "VENUS_DUMP",
}

# Maps a config option to a list of relevant Borealis components
# Implicit assumption: the name of the key in this dictionary matches
# the name of the config option in the corresponding component.
CONFIG_ALIASES = {
    "enable-x11-move-windows": ["sommelier"],
    "no-quirks": ["sommelier"],
    "persistent-logs": ["sommelier", "launch-wrap", "bdt", "steam"],
    "wayland-debug-exo": ["sommelier"],
    "wayland-debug-xwayland": ["sommelier"],
}
COMPONENTS_REQUIRING_PKILL = ["sommelier"]
CONFIG_ROOT = "/home/chronos/.config"
CACHE_ROOT = "/home/chronos/.cache"


class LogConfig:
    """Represents the log configuration of Borealis modules

    Borealis components write their logs to different paths based on the
    existence of an empty file at CONFIG_ROOT/component/persistent-logs.
    When this toggle file exists, Borealis components log to
    CACHE_ROOT/component/. When the toggle file does not exist, Borealis
    components log to /tmp/.
    """

    def __init__(self):
        self.__component_log_dir = {}

    def get_path_in_log_dir(self, component, file):
        """Returns the current log directory for a component"""
        # __component_log_dir is lazily populated
        if component not in self.__component_log_dir:
            # Assume all components log out to /tmp/ by default
            log_dir = "/tmp/"
            # Assume all components use the "persistent-logs" config option
            log_config_path = os.path.join(
                CONFIG_ROOT, component, "persistent-logs"
            )
            if os.path.isfile(log_config_path):
                log_dir = os.path.join(CACHE_ROOT, component)
            self.__component_log_dir[component] = log_dir
        return os.path.join(self.__component_log_dir[component], file)


log_config = LogConfig()

# bdt logs and artifacts
BDT_LOG_FILENAME = log_config.get_path_in_log_dir("bdt", "bdt.log")
REPORT_FILENAME = log_config.get_path_in_log_dir("bdt", "bdtlogs.tar.gz")
COMPAT_SNAPSHOT_FILENAME = log_config.get_path_in_log_dir(
    "bdt", "bdtcompatdata.tar.gz"
)
COMPAT_SNAPSHOT_PRE_FILENAME = log_config.get_path_in_log_dir(
    "bdt", "bdtcompatdata-post.tar.gz"
)
COMPAT_SNAPSHOT_POST_FILENAME = log_config.get_path_in_log_dir(
    "bdt", "bdtcompatdata-post.tar.gz"
)

# launch-wrap logs and artifacts
LAUNCH_OPTIONS_FILENAME = "/tmp/launch-options.sh"
LAUNCH_LOG_FILENAME = log_config.get_path_in_log_dir(
    "launch-wrap", "launch-log.txt"
)
GAME_ENV_FILENAME = log_config.get_path_in_log_dir("launch-wrap", "game.env")

# sommelier logs and artifacts
TIMING_LOG_FILENAME = log_config.get_path_in_log_dir(
    "sommelier", "timing-trace_set_*"
)

GAME_RUN_LARGE_FILES = [
    "/tmp/combined_metrics.json",
    "/tmp/game.log",
    "/tmp/game.strace",
    "/tmp/game.trace",
    "/tmp/metrics.json",
    "/tmp/proton_crashreports",
    COMPAT_SNAPSHOT_FILENAME,
    COMPAT_SNAPSHOT_PRE_FILENAME,
    COMPAT_SNAPSHOT_POST_FILENAME,
]
GAME_RUN_FILES = GAME_RUN_LARGE_FILES + [GAME_ENV_FILENAME]
MISC_FILES = [
    TIMING_LOG_FILENAME,
]
LAUNCH_WRAPPER_FILES = [
    LAUNCH_LOG_FILENAME,
    LAUNCH_OPTIONS_FILENAME,
]
REPORT_FILES = GAME_RUN_FILES + MISC_FILES + LAUNCH_WRAPPER_FILES
CLEAN_FILES = [REPORT_FILENAME] + GAME_RUN_LARGE_FILES + MISC_FILES
CLEAN_ALL_FILES = LAUNCH_WRAPPER_FILES

# Mapping of strings to game ids (as strings).  Keys are assumed lowercase.
GAME_IDS = {
    "deadcells": "588650",
    "dead cells": "588650",
    "hades": "1145360",
    "portal": "400",
    "portal2": "620",
}

# Mapping of strings to compatability layers.
COMPAT_ALIASES = {
    "none": "none",
    "slr": "steamlinuxruntime",
    "5": "proton_513",
    "5.13": "proton_513",
    "6": "proton_63",
    "6.3": "proton_63",
    "7": "proton_7",
    "8": "proton_8",
    "exp": "proton_experimental",
    "experimental": "proton_experimental",
    "proton-stable": "proton_8",
    "proton-exp": "proton_experimental",
}

AWAIT_ALIASES = {
    "start": "Running command: ",
    "stop": "Command exit status",
}


class LaunchLogWatcher:
    """Launch log watcher."""

    POLL_TIME_S = 0.35

    def __init__(self, filename=LAUNCH_LOG_FILENAME):
        self.filename = filename
        self.file = None
        self.open(True)

    def __del__(self):
        if self.file:
            self.file.close()
            self.file = None

    def open(self, seek):
        """Opens the launch log.

        Returns:
            True if the launch log was opened.
        """
        # To ensure that logs are not missed, the watcher should be opened
        # prior to kicking off any operations that might generate the log.
        # Assuming this is done, we seek to the end to ignore any prior logs.
        # Otherwise, if the file doesn't exist, don't seek to the end to
        # fully search all the logs.
        if not self.file:
            try:
                # pylint: disable=consider-using-with
                self.file = open(self.filename, encoding="utf-8")
                # pylint: enable=consider-using-with
            except FileNotFoundError:
                return False
            if self.file:
                # Seek to the end if specified.
                if seek:
                    self.file.seek(0, os.SEEK_END)
                return True
        return False

    def await_log(self, pattern=None, match=None, timeout=None):
        """Await a particular line in the launch log."""
        # Convert relative timeouts to absolute.
        if timeout:
            timeout = calculate_end_time(timeout)

        if match is None and pattern is not None:
            match = re.compile(pattern).search

        # Just keep reading from the end of the file.
        line = None
        while True:
            # Attempt to open the file if it's not already open.
            if not self.file:
                self.open(False)

            # Attempt to read a line if the file is open.
            if self.file:
                line = self.file.readline()

                # If there was a line, see if it matches.
                if line and match(line):
                    return line.rstrip()

            # Timeout if too much time has elapsed.
            if timeout and datetime.datetime.now() > timeout:
                return None

            # Wait to do it all over again.
            if not line:
                time.sleep(self.POLL_TIME_S)


class LaunchOptions:
    """Manipulate launch-wrapper launch-options.sh."""

    def __init__(self, filename=LAUNCH_OPTIONS_FILENAME, read=True, write=True):
        self.lines = []
        self.filename = filename
        if read:
            self.read()
        self.finish_write = write

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.finish_write:
            self.write()

    def read(self):
        """Read the launch options in."""
        if os.path.exists(self.filename):
            print(f"Reading launch options from {self.filename}")
            with open(self.filename, encoding="utf-8") as f:
                self.lines = f.readlines()

    def write(self):
        """Write the launch options out."""
        print(f"Writing launch options to {self.filename}")
        with open(self.filename, "w", encoding="utf-8") as f:
            f.writelines(self.lines)

    def set(self, option, value):
        """Set a launch option to a value."""
        option_re = re.compile(rf"\s*{option}\s*:?=")
        self.lines = [l for l in self.lines if not option_re.match(l)]
        print(f"Setting {option}={value}")
        # TODO(b/325093294): Remove once fixed in the latest Proton stable version.
        if option == "OUTPUT_DUMP" and value == "1":
            print(
                "[Proton 8.0] Run `bdt venus-dump --off`"
                " AND `socat pty,link=/tmp/game.log,rawer stdout` in a"
                " separate borealis terminal to work around b/325093294."
            )
        elif option == "VENUS_DUMP" and value == "1":
            print(
                "[Proton 8.0] VENUS_DUMP uses 'tee' to redirect output while"
                " the game is still running. Use of 'tee' can cause game"
                " output to never flush, hiding issues. See b/325093294."
            )
        self.lines.append(f"{option}={value}\n")

    def enable(self, option, enable):
        """Enable or disable a boolean launch option."""
        self.set(option, "1" if enable else "0")


def log(msg, level="INFO", tz=None, stdout=True, stderr=False, also_exit=False):
    """Add a log to the bdt log."""
    with open(BDT_LOG_FILENAME, "a", encoding="utf-8") as f:
        now = datetime.datetime.now()
        iso = now.astimezone(tz).replace(microsecond=0).isoformat()
        game_id = "BDT"
        pid = os.getpid()
        print(f"{iso} {level} {sys.argv[0]}/{game_id}[{pid}]: {msg}", file=f)
        if also_exit:
            sys.exit(msg)
        elif stderr:
            print(msg, file=sys.stderr)
        elif stdout:
            print(msg)


def determine_last_game():
    """Attempt to determine last game id and PID from the launch-log.txt."""
    if not os.path.exists(LAUNCH_LOG_FILENAME):
        return None, None

    # Attempt to match line of the form:
    # 2022-04-08T22:10:46+00:00 INFO /usr/bin/launch-wrap.sh/1145360[1662]: xx
    with open(LAUNCH_LOG_FILENAME, encoding="utf-8") as f:
        # TODO(davidriley): This doesn't handle large logs well since it
        # is being reversed in memory.
        for line in reversed(f.readlines()):
            m = re.search(r"/usr/bin/launch-wrap.sh/(\d+)\[(\d+)\]", line)
            if m:
                game_id = m.group(1)
                pid = m.group(2)
                print(f"Found last game {game_id} with pid {pid}")
                return game_id, pid

    return None, None


def enable_options(options, file=LAUNCH_OPTIONS_FILENAME, component=None):
    """Enable options, leaving the rest unchanged."""
    for o in options:
        if o in LAUNCH_ALIASES:
            launch_option_set(o, True, file)
        elif o in CONFIG_ALIASES:
            config_option_set(o, True, component)


def disable_options(options, file=LAUNCH_OPTIONS_FILENAME, component=None):
    """Disable options, leaving the rest unchanged."""
    for o in options:
        if o in LAUNCH_ALIASES:
            launch_option_set(o, False, file)
        elif o in CONFIG_ALIASES:
            config_option_set(o, False, component)


def set_options(options, file=LAUNCH_OPTIONS_FILENAME, component=None):
    """Set the options to an explicit set."""
    # Clear launch options
    with LaunchOptions(file, read=False):
        pass
    # Clear config options
    for c in CONFIG_ALIASES:
        config_option_set(c, enable=False, component=None)
    # Enable the requested set
    enable_options(options, file, component)


def launch_option_set(option, enable, file):
    """Set a single launch-wrapper option leaving the rest unchanged."""
    with LaunchOptions(file) as opts:
        opts.enable(LAUNCH_ALIASES[option], enable)


def config_option_set(option, enable, component):
    """Toggle a single config file leaving the rest unchanged."""
    if not os.path.exists(CONFIG_ROOT):
        print(
            f"{CONFIG_ROOT} does not exist. Maitred should have created it."
            " Check your Borealis installation."
        )
        return
    for c in CONFIG_ALIASES[option]:
        if component is not None and c != component:
            continue
        directory = os.path.join(CONFIG_ROOT, c)
        path = os.path.join(directory, option)
        if enable:
            if not os.path.exists(directory):
                # Create the component config directory if it doesn't exist
                os.mkdir(directory, mode=0o755)
                shutil.chown(directory, user="chronos", group="chronos")
            print(f"Adding config option at {path}")
            with open(path, "w", encoding="utf-8"):
                pass
        else:
            print(f"Removing config option at {path}")
            if os.path.isfile(path):
                os.remove(path)
        if c in COMPONENTS_REQUIRING_PKILL:
            print(
                f" NOTE: {option} requires a {component} restart."
                f" Run: pkill {component}"
            )


def run_steam_console_command(command):
    """Run Steam console command, similar to subprocess.call."""
    if not is_steam_running():
        sys.exit("ERROR: Steam not running")

    # TODO(b/238390964): Fail early if Steam is not logged in

    if not command.startswith("+"):
        # Commands needs to start with '+' so that Steam executes the command.
        command = "+" + command

    if " " not in command:
        # Space is required for argument-less console commands to be
        # parsed correctly, ex:
        #   `$ steam steam://open/console/ +quit `
        #   Steam: *Steam exits*
        # Not doing so outputs error, ex:
        #   `$ steam steam://open/console +quit`
        #   Steam: Command not found quit'
        command += " "

    print(f"Running Steam console command {command}")
    subprocess.call(["steam", "steam://open/console/", f"{command}"])


def get_ext(filename):
    """Determine tarfile ext from filename."""
    if filename.endswith(".bz2") or filename.endswith(".tbz2"):
        return "bz2"
    if filename.endswith(".gz") or filename.endswith(".tgz"):
        return "gz"
    return ""


def login(args):
    """Log in to the log uploader, for `bdt report -u`."""
    del args
    _upload.Uploader().login()


def report(output=REPORT_FILENAME, upload=False, verbose=False):
    """Generate report of relevant logs."""
    ext = get_ext(output)

    files = REPORT_FILES
    last_game, _ = determine_last_game()
    if last_game:
        files.append(f"/home/chronos/steam-{last_game}.log")

    # Find files to include in the report
    report_files = []
    for g in files:
        for f in glob.glob(g):
            if os.path.exists(f):
                report_files.append(f)

    if upload:
        # Upload report to log service.
        u = _upload.Uploader()
        if verbose:
            u.enable_debugging()
        u.upload_report(report_files, steam_id=last_game)
    else:
        # Save report to tarball.
        with tarfile.open(output, f"w:{ext}") as t:
            for f in report_files:
                t.add(f)


def is_steam_running(app="steam"):
    """Checks if Steam is running."""
    retcode = subprocess.call(["pgrep", app], stdout=subprocess.DEVNULL)
    return retcode == 0


def get_game_id(game, use_last=True):
    """Attempt to map to a game id or use the most recent."""
    if game:
        if not game.isnumeric():
            # Attempt to map game via dictionary, assume keps are lowercase.
            game = GAME_IDS.get(game.lower())
    elif use_last:
        game, _ = determine_last_game()
    return game


def start_game(game, wait=None, timeout=None):
    """Request for Steam to start a game."""
    if not is_steam_running():
        sys.exit("ERROR: Steam not running")

    game = get_game_id(game)
    if not game:
        sys.exit("ERROR: no game specified or determinable")
    print(f"Starting game {game}")
    watcher = LaunchLogWatcher()
    subprocess.call(["steam", f"steam://rungameid/{game}"])

    # Wait for the game if requested.
    if wait:
        print(f"Waiting for game to {wait}")
        line = watcher.await_log(pattern=AWAIT_ALIASES[wait], timeout=timeout)
        if line:
            print(f"Found {wait} event")
        else:
            print(f"WARNING: Timeout before {wait} occurred")


def install_game(game, layer=None):
    """Request for Steam to install a game."""
    if not is_steam_running():
        sys.exit("ERROR: Steam not running")

    game = get_game_id(game, use_last=False)
    if not game:
        sys.exit("ERROR: no game specified")

    if layer:
        set_compat_layer(game, layer)

    print(f"Installing game {game}")
    subprocess.call(["steam", f"steam://install/{game}"])


def stop_game(game=None):
    """Request for Steam to stop a game."""
    game = get_game_id(game)
    if not game:
        sys.exit("ERROR: no game specified or determinable")

    print(f"Stopping game {game}")
    run_steam_console_command(f"app_stop {game}")


def find_children(pid, recurse=False):
    """Find child processes of a given PID."""
    result = subprocess.run(
        ["pgrep", "-P", str(pid)], capture_output=True, check=False
    )
    if result.returncode:
        return []

    children = result.stdout.decode().splitlines()
    if recurse:
        for c in children.copy():
            children.extend(find_children(c, recurse))
    return children


def kill_game(pid=None, sig="SIGHUP", kill_wrapper=False):
    """Request for Steam to stop a game."""
    if not is_steam_running():
        sys.exit("ERROR: Steam not running")

    if not pid:
        pid = determine_last_game()[1]
    sig_value = signal.Signals[sig].value  # pylint: disable=no-member
    if not pid:
        sys.exit("ERROR: no game specified or determinable")

    print(f"Stopping game with PID {pid}")
    children = find_children(pid, True)
    if kill_wrapper:
        children.append(pid)
    print(f'Killing PIDs {", ".join(children)} with {sig}')
    for p in children:
        os.kill(int(p), sig_value)


def set_compat_layer(game_id, layer):
    """Helper to set Steam compatibility layer."""
    layer = COMPAT_ALIASES.get(layer, layer)
    print(f"Setting game {game_id} to compatibility layer {layer}")
    pri = 250
    run_steam_console_command(f"app_change_compat_tool {game_id} {layer} {pri}")


def set_compat(game, layer):
    """Request for Steam to set compatibility layer."""
    game = get_game_id(game)
    if not game:
        sys.exit("ERROR: no game specified or determinable")

    set_compat_layer(game, layer)


def read_game_env(filename=GAME_ENV_FILENAME):
    """Read in a game.env file as a dict."""
    env = {}
    with open(filename, encoding="utf-8") as f:
        for line in f.readlines():
            try:
                key, value = line.rstrip().split("=")
                env[key] = value
            except ValueError:
                # Skip invalid lines, likely values that have newlines.
                pass
    return env


def save_compat(
    path=None,
    env=None,
    game=None,
    output=COMPAT_SNAPSHOT_FILENAME,
    delete=False,
):
    """Save a snapshot of the compat data directory."""
    # Determine the path of the compat data.
    if not path and env and os.path.isfile(env):
        # No path specified but have env which includes the path.
        game_env = read_game_env(env)
        path = game_env.get("STEAM_COMPAT_DATA_PATH")
    if not path:
        # No path specified so attempt to resolve via game (either provided
        # or the last one run).
        game = get_game_id(game)
        if not game:
            sys.exit("ERROR: no game specified or determinable")
        path = f"/home/chronos/.steam/steam/steamapps/compatdata/{game}"

    if not os.path.isdir(path):
        print(f"WARNING: compat data path {path} does not exist")
        return

    print(f"Saving compat tarball {output} with contents of {path}")
    ext = get_ext(output)
    with tarfile.open(output, f"w:{ext}") as t:
        t.add(path)

    if delete:
        print(f"Deleting compat data directory {path}")
        shutil.rmtree(path)


def dump_timing_log(pid=None, sig="SIGHUP"):
    """Request for Sommelier to dump timing log."""
    if not pid:
        pid = subprocess.check_output(["pgrep", "sommelier"]).decode().rstrip()

    if not pid:
        sys.exit("ERROR: no Sommelier PID specified or determinable")

    sig_value = signal.Signals[sig].value  # pylint: disable=no-member
    print(f"Sending Sommelier PID {pid} signal {sig}")
    os.kill(int(pid), sig_value)


def calculate_end_time(timeout):
    """Calculate the end time of a timeout."""
    # If timeout is an int, treat as seconds.  Treat as delta if it is one.
    if isinstance(timeout, (float, int)):
        timeout = datetime.timedelta(seconds=timeout)
    if isinstance(timeout, datetime.timedelta):
        timeout = datetime.datetime.now() + timeout
    return timeout


def await_log(event=None, pattern=None, timeout=None):
    """Wait for a given log to appear in the launch log."""
    watcher = LaunchLogWatcher()
    if event:
        pattern = AWAIT_ALIASES[event]
    print(f"Waiting for log line to match: {pattern}")
    line = watcher.await_log(pattern=pattern, timeout=timeout)
    if line:
        print(f"Found matching line: {line}")
    else:
        print("WARNING: Did not found matching line")
    return line


def clean_files(all_files=False, dry_run=False, basedir="/"):
    """Remove files from previous game runs."""
    targets = CLEAN_FILES.copy()
    if all_files:
        targets.extend(CLEAN_ALL_FILES)
    for g in targets:
        # Don't use os.path.join() because CLEAN_FILES are absolute paths
        # and os.path.join() will just treat it as an absolute path.
        g = basedir + g
        for f in glob.glob(g):
            if os.path.isdir(f):
                print(f"Removing directory {f}")
                if not dry_run:
                    shutil.rmtree(f)
            elif os.path.isfile(f):
                print(f"Removing file {f}")
                if not dry_run:
                    os.unlink(f)


def script_attempt_start(game, timeout=None, save_compat_post=False):
    """Attempt to start a game and wait for to exit."""
    watcher = LaunchLogWatcher()
    start_game(game)
    print(f"Waiting for game to start (timeout={timeout})")
    line = watcher.await_log(pattern=AWAIT_ALIASES["start"], timeout=timeout)
    if line is None:
        print("ERROR: Game did not start, stage should be retried")
    else:
        print(f"Waiting for game to exit (timeout={timeout})")
        line = watcher.await_log(pattern=AWAIT_ALIASES["stop"], timeout=timeout)
        if line is None:
            print("Game did not exit, terminating...")
            stop_game()
            line = watcher.await_log(pattern=AWAIT_ALIASES["stop"], timeout=10)
            if line is None:
                print("ERROR: Game still did not exit")
    if save_compat_post:
        save_compat(
            env=GAME_ENV_FILENAME,
            game=game,
            output=COMPAT_SNAPSHOT_POST_FILENAME,
        )
    report()
    print()


def replay_cache_exists(game_id, cache_directories):
    """Return True if Fossilize replay cache exists for game_id."""
    for library_root in cache_directories:
        cache_file = os.path.join(library_root, game_id, "**", "replay_cache*")
        if glob.glob(cache_file):
            return True
    return False


def shader_prune(game, timeout, cache_directories):
    """Request for Steam to delete Steam Fossilize replay cache.

    Args:
        game: game ID (integer).
        timeout: replay existence check timeout in seconds.
        cache_directories: list of string directories to search for
        replay caches. This supports glob, but not recursive.
    """
    if not game:
        print("Game not set, deleting cache for previously-run game.")
        game = determine_last_game()[0]
        if not game:
            print("Failed to determine last game")
            return

    game = get_game_id(game)

    print(f"Attempting to delete Fossilized replay cache for {game}")
    run_steam_console_command(f"shader_prune {game}")

    print("Checking if replay_cache* still exists..")
    end_time = calculate_end_time(timeout)
    while replay_cache_exists(game, cache_directories):
        if datetime.datetime.now() > end_time:
            sys.exit(f"ERROR: replay_cache* still exists for {game}")
        time.sleep(0.5)
    print("replay_cache* not found, deletion success!")


def script_no_start(stage, game=None, timeout=None):
    """No-start script."""
    # Determine the game in question.
    game = get_game_id(game)
    if not game:
        sys.exit("ERROR: no game specified or determinable")
    else:
        print(f"Debugging game {game} that does not start, stage {stage}")

    # TODO(davidriley): Move this to after upload.
    print("Cleaning files from earlier runs")
    clean_files()

    save_compat_post = False
    if stage in ("1", "baseline"):
        set_options(["env", "output"])
    elif stage in ("2", "strace"):
        set_options(["env", "output", "strace"])
    elif stage in ("3", "slr"):
        set_options(["env", "output"])
        set_compat_layer(game, "slr")
    elif stage in ("4", "proton"):
        set_options(["env", "output"])
        set_compat_layer(game, "proton-stable")
    elif stage in ("5", "pr1", "proton"):
        set_options(["env", "output"])
        set_compat_layer(game, "proton-stable")
    elif stage in ("6", "pr2", "proton-compat"):
        save_compat(
            env=GAME_ENV_FILENAME,
            game=game,
            output=COMPAT_SNAPSHOT_PRE_FILENAME,
            delete=True,
        )
        set_options(["env", "output"])
        set_compat_layer(game, "proton-stable")
        save_compat_post = True
    elif stage in ("7", "pr3", "proton-debug"):
        set_options(["env", "output", "proton-debug"])
        set_compat_layer(game, "proton-stable")
    elif stage in ("8", "pr4", "proton-strace"):
        set_options(["env", "output", "strace"])
        set_compat_layer(game, "proton-stable")
    elif stage in ("9", "pr5", "proton-exp"):
        set_options(["env", "output"])
        set_compat_layer(game, "proton-exp")
    elif stage in ("10", "pr6", "pressure-vessel-debug"):
        set_options(["env", "output", "pressure-vessel-debug"])
        set_compat_layer(game, "proton-stable")
    elif stage in ("11", "zink"):
        set_options(["env", "output", "zink"])
    else:
        print()
        sys.exit("Stage does not exist: script finished")

    script_attempt_start(game, timeout, save_compat_post)
    print("Script stage done: repeat with next stage")


def window_info(out):
    """Log X11 window info"""
    with open(out, "w", encoding="utf-8") as file:
        print(f"Writing X11 window info to {out}")
        subprocess.run(
            ["xwindump.py", "--verbose"],
            env=os.environ,
            stdout=file,
            check=True,
        )


def metrics(out):
    """Monitor and log guest-side metrics"""
    with subprocess.Popen(
        ["detailed_metrics", "--quick-stats", "-o", out, "--quiet"]
    ) as p:

        def signal_handler(signum, frame):
            del signum, frame
            p.terminate()
            p.wait()
            print("Monitoring terminated.")

        orig_handler = signal.signal(signal.SIGINT, signal_handler)
        print("Monitoring until interrupted, press Ctrl-C to stop.")
        signal.pause()
        signal.signal(signal.SIGINT, orig_handler)
