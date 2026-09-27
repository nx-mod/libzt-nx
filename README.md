# libzt-nx

[ZeroTier's libzt](https://github.com/zerotier/libzt) for the **Nintendo Switch under Atmosphère**, small
enough to run inside a system module. Tested on Atmosphère 1.11.2, firmware 22.5.0.

## Credits

- **[AJstylishhh](https://github.com/AJstylishhh)** -- the Switch port itself (CMake platform detection, lwIP
  thread-safety and Unix-port fixes, PHY checksums, POSIX header guards), from
  [switch-ldn-zt](https://github.com/AJstylishhh/switch-ldn-zt). This fork builds on that work.
- **[ZeroTier](https://github.com/zerotier)** -- libzt and ZeroTierOne (1.16.2, `fc5c3ec2`).
- **[lwIP](https://savannah.nongnu.org/projects/lwip/)** -- the TCP/IP stack.
- **nx-mod** -- the fixes and system-module profile below.

## What's different

All Switch changes are under `__SWITCH__`; other platforms build as upstream.

**Fixes found on hardware**
- *Node dropped offline every 30-90 s.* Roots answered HELLO but never ECHO, and upstreams were contacted only
  every 224 s. Upstream contact now happens every 14 s (ping period 10 s). A workaround: why ECHO goes
  unanswered is unknown.
- *`zts_peer_info_t` was filled by `memcpy` from a differently laid out `ZT_Peer`* (garbage `path_count`, reads
  past the object). Now copied field by field.
- Hardening from upstream `misc-fixes` (identity NULL/bounds checks, `C25519` -> `ECC`, `ZTS_DISABLE_CENTRAL_API`
  no longer forced on in the header).

**System-module profile** (a sysmodule has ~2 MiB of heap and small stacks)
- Metrics are empty stubs; the first use of each is reported through the weak hook `zt_stub_hit()`.
- Smaller tables and queues: RX queue 4, rate gates 1024, Phy poll buffer 16 KiB, 16 bindings; 128 KiB thread
  stacks; no 1 MiB UDP socket buffer requests; big packet locals moved to the heap.
- Multicast TX queue bounded (2 per group, 4 in all) and pruned on send. Each entry is ~30 KiB and ARP opens a
  group per target, so while no peer answered, the queue used to exhaust the heap in seconds. A failed
  allocation drops the frame instead of crashing.
- The 2 MiB identity-hash buffer is borrowed only when needed, through `zt_genmem_acquire()` /
  `zt_genmem_release()`. Unknown HELLOs are resolved with a WHOIS to a root instead of a local hash.
- Packet compression off; events nobody enabled are not queued; platform reported as `nx-mod`.
- NAT-PMP/UPnP are compiled out.

## Embedding

Define these in the program linking the library (all optional; weak):

```c
void  zt_stub_hit(const char *what);           /* a stubbed feature was used (log it) */
void *zt_genmem_acquire(unsigned long size);   /* 2 MiB for an identity hash; NULL to refuse */
void  zt_genmem_release(void *p);
```

Traffic over the ZeroTier network must use `zts_bsd_*` calls. Plain `socket()`/`bind()` compile just as well
but bind to the physical interface and never see ZeroTier traffic.

In a system module: run every `zts_*` call on one thread with a large stack (128 KiB), and keep newlib's
per-thread state available to libzt's threads (libnx threads, not bare kernel threads).

## Caveats

- Diagnostic tracing (`[SWITCH-AUTH]`, `[SWITCH-DIAG]`) is on whenever `__SWITCH__` is defined; to be moved
  behind its own flag.
- `zts_core_query_path_count()` / `zts_core_query_path()` are stubs.
- IPv6 binds fail on Switch (`EAFNOSUPPORT`) and are retried harmlessly.
- Only the static library is built and tested for Switch.

## Building for Switch

devkitPro (devkitA64) and CMake:

```sh
cmake -S . -B build-switch \
  -DCMAKE_TOOLCHAIN_FILE=<switch.toolchain.cmake> -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE=Release -DSWITCH=ON \
  -DBUILD_STATIC_LIB=ON -DBUILD_SHARED_LIB=OFF -DBUILD_HOST_EXAMPLES=OFF -DBUILD_HOST_SELFTEST=OFF \
  -DALLOW_INSTALL_TARGET=OFF -DZTS_DISABLE_CENTRAL_API=ON \
  -DCMAKE_C_FLAGS="-D__SWITCH__ -DSWITCH -DFD_SETSIZE=1024 -D__BSD_VISIBLE=1 -D__POSIX_VISIBLE=200809 -D_DEFAULT_SOURCE -DLWIP_PROVIDE_ERRNO=1 -I<repo>/ext -I<repo>/ext/lwip-contrib/ports/unix/port/include" \
  -DCMAKE_CXX_FLAGS="<same>"
cmake --build build-switch --target zt-static
```

Produces `build-switch/lib/libzt.a`. `ext/endian.h` and `ext/arpa/inet.h` stand in for headers libnx lacks.
Other platforms: as upstream (`build.sh` / `build.ps1`). API docs: [docs.zerotier.com](https://docs.zerotier.com/sockets/tutorial.html).

## License

ZeroTier and libzt are under the [BSL 1.1](./LICENSE.txt); third-party code is listed in
[AUTHORS.md](ext/ZeroTierOne/AUTHORS.md).
