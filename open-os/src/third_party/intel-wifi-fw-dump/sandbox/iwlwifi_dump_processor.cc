// Copyright 2017 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <base/command_line.h>
#include <base/files/file.h>
#include <base/files/file_enumerator.h>
#include <base/files/file_path.h>
#include <base/files/file_util.h>
#include <base/files/scoped_temp_dir.h>
#include <base/posix/eintr_wrapper.h>
#include <base/strings/string_split.h>
#include <base/strings/string_util.h>
#include <base/time/time.h>
#include <brillo/process.h>
#include <brillo/syslog_logging.h>
#include <chromeos/libminijail.h>

namespace iwlwifi {
namespace {

constexpr char kDumpName[] = "iwl-fw-error.dump";
constexpr char kFwParserPath[] = "/opt/intel/fw_parser";
constexpr char kFwParserSeccompFile[] =
    "/usr/share/policy/fw_parser-seccomp.policy";
constexpr char kUser[] = "iwlwifi-dump";

const char* const kAllowedDumpFiles[] = {
  "csr.lst",
  "fh_regs.lst",
  "monitor.lst.sysmon",
  "radio_reg.lst",
};

}  // namespace

std::vector<base::FilePath> GetBindMounts() {
  std::vector<base::FilePath> mounts;

  const std::string mount_list(BIND_MOUNTS);
  std::vector<std::string> parsed_mount_list = base::SplitString(
      mount_list, ",", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);
  for (const std::string& mount : parsed_mount_list) {
    base::FilePath path(mount);
    if (path.IsAbsolute() && !path.ReferencesParent())
      mounts.push_back(path);
  }

  return mounts;
}

bool RunFwParser(const base::FilePath& work_dir) {
  struct passwd* pw = getpwnam(kUser);  // NOLINT(runtime/threadsafe_fn)
  CHECK(pw);
  if (chmod(work_dir.value().c_str(), 0755) < 0 ||
      chown(work_dir.value().c_str(), pw->pw_uid, pw->pw_gid) < 0) {
    LOG(ERROR) << "Could not change permissions on work directory "
               << work_dir.value();
    return false;
  }

  base::FilePath dump_file = work_dir.Append(kDumpName);
  if (!base::PathExists(dump_file))
    return false;

  // Create the target directories for bind-mounting an extremely minimal
  // rootfs. We want to lock down this binary as much as possible.
  const base::FilePath kRoot("/");
  const std::vector<base::FilePath> bind_mounts = GetBindMounts();
  for (const base::FilePath& mount : bind_mounts) {
    base::FilePath work_dir_bind_target(work_dir);
    CHECK(kRoot.AppendRelativePath(mount, &work_dir_bind_target));
    base::File::Error error;
    if (!base::CreateDirectoryAndGetError(work_dir_bind_target, &error)) {
      LOG(ERROR) << "Failed to create directory \""
                 << work_dir_bind_target.value() << "\": "
                 << base::File::ErrorToString(error);
      return false;
    }
  }

  struct minijail* j = minijail_new();
  minijail_no_new_privs(j);
  minijail_change_user(j, kUser);
  minijail_change_group(j, kUser);
  minijail_use_seccomp_filter(j);
  minijail_parse_seccomp_filters(j, kFwParserSeccompFile);
  minijail_namespace_ipc(j);
  minijail_namespace_uts(j);
  minijail_namespace_net(j);
  minijail_namespace_pids(j);
  for (const base::FilePath& mount : bind_mounts) {
    const char* bind_target = mount.value().c_str();
    minijail_bind(j, bind_target, bind_target, false);
  }
  minijail_enter_chroot(j, work_dir.value().c_str());

  std::string argv0(kFwParserPath);
  std::string argv1(dump_file.BaseName().value());
  std::vector<char*> argv{&argv0[0], &argv1[0], nullptr};
  minijail_run(j, argv[0], argv.data());

  int exit_status = minijail_wait(j);
  minijail_destroy(j);

  return (exit_status == 0);
}

bool ProcessDump(const base::FilePath& dump_file) {
  base::ScopedTempDir temp_dir;
  if (!temp_dir.CreateUniqueTempDir()) {
    DLOG(ERROR) << "Could not create temporary directory";
    return false;
  }

  base::FilePath dump_file_target = temp_dir.GetPath().Append(kDumpName);
  if (!base::Move(dump_file, dump_file_target)) {
    DLOG(ERROR) << "Failed to move " << dump_file.value()
                << " into the temp sandbox directory";
    return false;
  }

  if (!RunFwParser(temp_dir.GetPath()))
    return false;

  dprintf(STDOUT_FILENO, "\n");

  // Only print the relevant output files. Other files may contain PII
  // that we don't know how to filter in the general case, so we leave
  // them out.
  base::FileEnumerator fe(
      temp_dir.GetPath(), false, base::FileEnumerator::FILES);
  for (base::FilePath current = fe.Next();
       !current.empty();
       current = fe.Next()) {
    bool allowed = false;
    for (const char* allowed_file : kAllowedDumpFiles) {
      if (base::EndsWith(current.value(),
                         allowed_file,
                         base::CompareCase::SENSITIVE)) {
        allowed = true;
        break;
      }
    }
    if (!allowed)
      continue;

    dprintf(STDOUT_FILENO, "{{{ %s }}}\n", current.value().c_str());
    base::ScopedFD file(open(current.value().c_str(),
                             O_RDONLY | O_EXCL | O_NONBLOCK));
    if (!file.is_valid()) {
      DPLOG(INFO) << "Couldn't open file " << current.value();
      return false;
    }
    struct stat stat_buf;
    if (fstat(file.get(), &stat_buf) < 0) {
      DPLOG(INFO) << "Couldn't stat file " << current.value();
      return false;
    }
    if (!S_ISREG(stat_buf.st_mode)) {
      DLOG(INFO) << current.value() << " is not a regular file";
      return false;
    }
    if (sendfile(STDOUT_FILENO, file.get(), nullptr, stat_buf.st_size) < 0) {
      DPLOG(ERROR) << "Failed to write " << current.value() << " out";
      return false;
    }

    dprintf(STDOUT_FILENO, "\n");
  }

  return true;
}

}  // namespace iwlwifi

int main(int argc, char** argv) {
  brillo::InitLog(brillo::kLogToStderrIfTty | brillo::kLogToSyslog);
  base::CommandLine command_line(argc, argv);

  if (command_line.GetArgs().size() != 1) {
    printf("Usage: %s dump_file\n"
           "  Parse Intel WiFi debug log dumps for feedback reporting.\n",
           command_line.GetProgram().value().c_str());
    return EXIT_FAILURE;
  }

  base::FilePath dump_file(command_line.GetArgs()[0]);
  LOG(INFO) << "Processing dump file " << dump_file.value();
  return iwlwifi::ProcessDump(dump_file) ? EXIT_SUCCESS : EXIT_FAILURE;
}
