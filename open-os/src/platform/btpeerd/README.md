# btpeerd

A system service for controlling Raspberry Pi btpeers for ChromeOS bluetooth testing.

Design: go/btpeerd


## Building btpeerd

System must have golang installed (any version) in order to download and
configure the local golang version used by the btpeerd build process.

Build btpeerd on the same system environment the executable is meant to be run
on (e.g. in the pi-gen chroot) to ensure system compatibility. The unit tests
should not be system dependent, so running the build script on gLinux should be
sufficient to say those test pass.

Run `scripts/build.sh` to build btpeerd. This will do the following:
* Download the specific version of golang needed for btpeerd to `./go/bin`
* Downloads go package dependencies.
* Run all unit tests
* Build btpeerd binary to `./go/bin/btpeerd`

## Running btpeerd

No files in this repository are used during btpeerd runtime, the `./go/bin/btpeerd`
executable should be runnable directly to launch btpeerd.

Note: Golang is only used during the compilation of the `btpeerd` executable.

The `btpeerd` accepts the following optional command line options:
* `--ListenPort=<port>` (default `8100`) Sets the port the gRPC server will listen on.
* `--LogLevel=<DEBUG|INFO|WARN|ERROR>` (default `INFO`) Sets the log level of btpeerd.

The `btpeerd` process will run continuously until a `SIGINT` signal (Ctrl+C) is
received, upon which it will gracefully shut down the gRPC server and then close.
