# memtracker

Tracks memory usage over a gaming session, using `detailed_metrics` on both the host and guest.

To run on a ChromeOS DUT:

    ./run_on_dut.sh dut

To run on a Reference Linux Device:

    1. Install detailed_metrics from borealis to /usr/local/bin/ in your reference device.
    2. Ensure you can ssh directly as root
    3. Run `MEMTRACKER_INSTALL_DIR=/device/path/where/to/install/memtracker/artifacts ./run_on_dut.sh host --host-only`
