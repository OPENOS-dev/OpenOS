# proto/src

This folder contains the protobuf source files for the Chromite Build API and Recipes implementations.

## Making changes

Before committing any changes to this repo, make sure to run `generate.sh` one
directory up and also `~/chromiumos/chromite/api/compile_build_api_proto`.

## Directories:

### chromite/api

This is the core API definitions, including all of the services and most of the messages.

The build_api.proto contains service and method options that define some of the implementation details. Specifically, the service options define the controller module that implements the methods, and optionally enforces whether or not the methods should run in the chroot. The method options allow overriding the default function name for the implementation, and overriding the service-level chroot setting.

* Service
  * module - Required
  * service_chroot_assert - Recommended
* Method
  * implementation_name - Optional
  * method_chroot_assert - Optional

These options are all implementation details that should not affect consumers of the endpoints, but are important details for anyone implementing endpoints.

### chromiumos

This folder contains more widely shared proto files.

### test_platform

This folder contains definitions of the cros_test_platform API, as well as internal protos used for communication between cros_test_platform components.

### uprev

This folder contains protos used for communication between uprev recipes and their components.

### device

### testplans

### cycler

Cycler protos: go/cros-gs-lifecycler

### bot_scaling

RoboCrop protos: go/robocrop

### ide_query

This directory contains the response proto for the ide_query script, defined at go/reqs-for-peep, which allows Cider to query ChromeOS for language support instructions. The proto definition should be pulled down from google3/devtools/cider/services/build/companion/ide_query.proto.
