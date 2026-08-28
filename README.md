# libzt-nx

A fork of [ZeroTier's libzt](https://github.com/zerotier/libzt) ported to run
on **Nintendo Switch under Atmosphère CFW**, plus two correctness fixes found
on real hardware and not yet upstreamed.

Tested against **Atmosphère 1.11.2-master-5388824be, firmware 22.5.0**.
Should track other recent Atmosphère releases fine -- nothing here depends on
a specific Atmosphère internal API, only on the standard libnx toolchain.

Base: [zerotier/libzt](https://github.com/zerotier/libzt) with
`ext/ZeroTierOne` bumped to **1.16.2** (upstream commit `fc5c3ec2`).
`ext/ZeroTierOne`, `ext/lwip`, and `ext/lwip-contrib` are vendored as plain
source here rather than git submodules -- one self-contained repo instead of
three that have to stay in lockstep, and ZeroTierOne in particular has moved
little upstream in recent years. History starts fresh at this fork rather
than carrying libzt's own commit history forward, since this is a heavy
restructure (vendored deps, a different platform target), not something
meant to track upstream commit-by-commit.

## What's different from upstream libzt

**Switch portability** -- CMake platform detection, lwIP thread-safety and
Unix-port fixes, PHY checksum handling, various POSIX header/include guards
(`ifaddrs.h`, `sys/un.h`, `sys/uio.h`, `endian.h`, `strtok_r` -> `strtok`) --
is [AJstylishhh](https://github.com/AJstylishhh)'s work from
[switch-ldn-zt](https://github.com/AJstylishhh/switch-ldn-zt), carried over
here so this builds standalone without a separate patch-script pipeline at
build time.

**Two real bugs**, found and fixed while getting ZeroTier working reliably on
a real console:

- **Node stayed "online" for only ~30-90s at a time, then dropped, then came
  back, repeating forever.** A node only counts as online while it has heard
  from an upstream (root) within `ZT_PEER_ACTIVITY_TIMEOUT` (30s). Between
  full HELLOs that contact is meant to be refreshed by small encrypted
  `VERB_ECHO` keepalives -- but on the test network used here, roots reliably
  answer HELLO and never answer ECHO (confirmed at both the socket level and,
  separately, via authentication tracing: every packet that *did* arrive from
  a root passed auth cleanly, ruling out a receive-side bug). `Node.cpp`'s
  `_PingPeersThatNeedPing` deliberately contacts upstreams "as infrequently
  as possible" -- a role-based timer scale of 16, i.e. every
  `ZT_PATH_HEARTBEAT_PERIOD * 16` == 224 seconds -- and returns before a full
  HELLO is ever considered. Scale 1 (14s) plus `ZT_PEER_PING_PERIOD` lowered
  to 10s puts full-HELLO upstream contact inside the 30s window. Derived
  path-expiration constants are pinned to their original absolute values so
  only contact cadence changes. **Why roots don't answer ECHO on that network
  is still unexplained** -- this fix works around it, not around its cause.

- **`zts_peer_info_t` reported garbage for `path_count` (and would in
  principle read past the end of the real object).** The peer-event dispatch
  did `memcpy(pr, peer, sizeof(zts_peer_info_t))` from a `ZT_Peer*`, assuming
  an identical field layout between `ZT_Peer` and `zts_peer_info_t`. They
  diverge completely past `latency` -- confirmed by comparing both headers
  field-by-field, and by a `PEER_PATH_DISCOVERED` event on hardware printing
  a `path_count` of 0 right next to it, since the event itself is gated on
  the real `ZT_Peer` field before the broken copy runs. Since the two structs
  are also different sizes, the memcpy could read past the real `ZT_Peer`
  object into whatever memory follows it. Replaced with an explicit
  field-by-field copy.

**Also included**: `[SWITCH-AUTH]` diagnostic tracing in `IncomingPacket.cpp`
(auth/MAC rejection, both call sites) and `Peer.cpp` (accepted packets),
gated behind `#ifdef __SWITCH__` so it compiles out entirely on every other
platform. This is what was used to confirm the keepalive fix above wasn't
masking an authentication problem.

**Also picked up** from zerotier/libzt's `misc-fixes` branch (which sits
directly on the same base commit this fork started from, so these carried
over cleanly): NULL/bounds-check hardening in `NodeService.cpp`'s identity
handling (a commented-out guard previously left `strlen()` able to
dereference NULL and a caller-supplied length able to overrun a fixed
buffer), the required `C25519` -> `ECC` rename in `Utilities.cpp` (removed
upstream in the 1.16.2 jump) with the capacity-check hardening that came with
it, and a real fix in `ZeroTierSockets.h` where `ZTS_DISABLE_CENTRAL_API` was
`#define`d unconditionally in the header itself, permanently overriding
CMake's own option for it.

## Building for Switch

Requires [devkitPro](https://devkitpro.org/) (devkitA64) and CMake.

```sh
export DEVKITPRO=/opt/devkitpro
export DEVKITA64=/opt/devkitpro/devkitA64
export PATH="$DEVKITA64/bin:$DEVKITPRO/tools/bin:$PATH"

cmake -S . -B build-switch \
  -DCMAKE_TOOLCHAIN_FILE=<path-to-switch.toolchain.cmake> \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_STATIC_LIB=ON -DBUILD_SHARED_LIB=OFF \
  -DBUILD_HOST_EXAMPLES=OFF -DBUILD_HOST_SELFTEST=OFF \
  -DALLOW_INSTALL_TARGET=OFF -DZTS_DISABLE_CENTRAL_API=ON \
  -DSWITCH=ON \
  -DCMAKE_C_FLAGS="-D__SWITCH__ -DSWITCH -DFD_SETSIZE=1024 -D__BSD_VISIBLE=1 -D__POSIX_VISIBLE=200809 -D_DEFAULT_SOURCE -DLWIP_PROVIDE_ERRNO=1 -I<this-repo>/ext -I<this-repo>/ext/lwip-contrib/ports/unix/port/include" \
  -DCMAKE_CXX_FLAGS="<same as above>"

cmake --build build-switch --target zt-static --parallel 4
```

Produces `build-switch/lib/libzt.a`. A `switch.toolchain.cmake` example
(setting the aarch64-none-elf devkitA64 toolchain and `CMAKE_SYSTEM_NAME` to
`Switch`) lives in the consuming project; `-DSWITCH=ON` plus
`CMAKE_SYSTEM_NAME STREQUAL "Switch"` are both checked throughout this
repo's `CMakeLists.txt`, so either is sufficient to opt into the Switch code
paths (both are set together above for clarity, not because both are
required).

`ext/endian.h` and `ext/arpa/inet.h` are committed compatibility stubs for
headers libnx doesn't provide; they're picked up automatically by the `-I
<this-repo>/ext` flag above.

### The `zts_bsd_*` API, not plain sockets

Any code using this library on Switch that needs to talk over the ZeroTier
virtual network **must** use `zts_bsd_socket()` / `zts_bsd_bind()` /
`zts_bsd_sendto()` / `zts_bsd_recvfrom()`, not plain `socket()`/`bind()`/etc.
A consuming app can (and typically will) also `-Wl,--wrap=socket` etc. to
route its *own* real network I/O (talking to roots, the controller) through
the physical interface -- that's a separate, legitimate use of the same
plain POSIX names. But anything meant to ride the ZeroTier tunnel itself has
to go through the `zts_bsd_*` names specifically; the two APIs look
identical and both compile fine, so getting this wrong doesn't fail to
build, it just silently binds to the wrong interface and never sees the
traffic it's listening for.

## Building for everything else

Unchanged from upstream libzt -- see `build.sh` / `build.ps1`, or the CMake
invocation above with `-DBUILD_HOST=ON` and no Switch-specific flags.

## Upstream docs

For the ZeroTier Sockets API itself (not the Switch port): [Docs](https://docs.zerotier.com/sockets/tutorial.html) | [Examples](./examples) | [ZeroTier issue tracker](https://github.com/zerotier/libzt/issues)

## Licensing

ZeroTier and the ZeroTier SDK (libzt and libztcore) are licensed under the
[BSL version 1.1](./LICENSE.txt). ZeroTier is free to use internally in
businesses and academic institutions and for non-commercial purposes.
Certain types of commercial use such as building closed-source apps and
devices based on ZeroTier or offering ZeroTier network controllers and
network management as a SaaS service require a commercial license. A small
amount of third party code is also included in ZeroTier and is not subject
to our BSL license -- see [AUTHORS.md](ext/ZeroTierOne/AUTHORS.md) for a
list of third-party code, where it is included, and the licenses that apply
to it.
