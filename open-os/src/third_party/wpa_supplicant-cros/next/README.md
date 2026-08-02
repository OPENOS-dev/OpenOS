# Contributing to wpa_supplicant in CrOS

This documents how to develop, test, and submit code to wpa_supplicant in
ChromeOS.

## Build and deploy

This follows the standard ChromeOS development flow:

```bash
cros-workon-${BOARD} wpa_supplicant-cros
emerge-${BOARD} wpa_supplicant-cros
cros deploy ${DUT} wpa_supplicant-cros
```

Note that `wpa_supplicant-cros/current` and `wpa_supplicant-cros/next` are
identical. This is a vestige of how we used to uprevs, but now that we've
converted to [automated merges](http://go/cros-wpa-supplicant-merge-automation-readme),
we no longer regularly use both directories other than for the occasional
rollout of a new feature flag. Please develop in `wpa_supplicant-cros/current`.

To restart wpa_supplicant after deploying:

```bash
(DUT) # restart wpasupplicant
```

## Testing

1. If applicable, implement the appropriate shill hooks (e.g.
   [supplicant_interface_proxy_interface.h](https://source.chromium.org/chromiumos/chromiumos/codesearch/+/main:src/platform2/shill/supplicant/supplicant_interface_proxy_interface.h))
   and execute the appropriate shill flow. See
   [go/shill-cheatsheet](http://go/shill-cheatsheet) for shill development tips.
2. Use the wpa_supplicant command line interface wpa_cli:
```bash
sudo -u wpa -g wpa wpa_cli
```
3. Use dbus-send:
```bash
dbus-send --system --print-reply --dest=fi.w1.wpa_supplicant1 \
	/fi/w1/wpa_supplicant1/Interfaces/0 \
        fi.w1.wpa_supplicant1.Interface.AddNetwork \
	...
```

or gdbus:
```bash
gdbus call --system --dest fi.w1.wpa_supplicant1 --object-path \
	/fi/w1/wpa_supplicant1/Interfaces/0 --method \
	fi.w1.wpa_supplicant1.Interface.AddNetwork \
	...
```
4. Use
   [hostap_hwsim](https://chromium.googlesource.com/chromiumos/platform/tast-tests/+/HEAD/src/go.chromium.org/tast-tests/cros/local/bundles/cros/wifi/README.hostap_hwsim.md)
5. Run matfunc tests

## Uploading to Gerrit

We try to follow kernel conventions detailed [here](https://www.chromium.org/chromium-os/developer-library/guides/kernel/kernel-development/#commit-messages-summary-lines-chromium-upstream-fromlist-backport
).
For trivial changes, feel free to send them upstream without internal review
(see below for more details). Otherwise, upload them with a **WIP:** prefix to
indicate that you'd like internal feedback first. After getting a +1 from
relevant reviewers, you should send the patch upstream. For time-sensitive
changes, we allow landing the change as **FROMLIST** with an **UPSTREAM-TASK**
tag at the end specifying a bug number to track the task of upstreaming the
change. Please also add the
[CrOSWiFi-PendingUpstreamReview](https://b.corp.google.com/hotlists/5433623)
hotlist to the task and add the patch to
[go/cros_supplicant_patches](http://go/cros_supplicant_patches). For other
changes, we prefer landing the change as **UPSTREAM** or **BACKPORT** to avoid
accruing technical debt.

If you've landed your change as **FROMLIST**, make sure to monitor the hostap
mailing list so you can revise your patch if necessary. After it has been
accepted upstream, revert the original **FROMLIST** patch and land it as
**UPSTREAM** (or **BACKPORT**) to update the change to its latest version if
necessary. There's no need to do this if there is no diff between the
**UPSTREAM** and **FROMLIST** patches. An easy way to do this is to run the
following command:
```bash
diff <(git show ${FROMLIST_HASH}) <(git show ${UPSTREAM_HASH})
```
Note that there will always be diffs, but you can skip relanding as long as
these diffs are part of the patch itself. Remember to close the task that was
opened to track upstreaming, and remove the patch from
[go/cros_supplicant_patches](http://go/cros_supplicant_patches).

## Contributing upstream

For convenience, we suggest subscribing to the
[hostap](http://lists.infradead.org/mailman/listinfo/hostap) mailing list so
that your patches will be automatically posted to the list without approval.
Note that DMARC restrictions may prevent subscribing to the mailing list with
your @google.com email. Sending changes upstream is fairly similar to the
[kernel process](https://www.chromium.org/chromium-os/developer-library/guides/kernel/kernel-development/#how-do-i-send-a-patch-upstream).
Follow those instructions to set up your git configuration and for best
practices with respect to patch titling and formatting. Note that our
wpa_supplicant repository already contains an `upstream/main` branch that you
can use to make sure the patch applies cleanly upstream. Once you are ready to
send your patch(es), you can send them to `j@w1.fi` (Jouni Malinen, the
maintainer) and `hostap@lists.infradead.org` (the mailing list).

