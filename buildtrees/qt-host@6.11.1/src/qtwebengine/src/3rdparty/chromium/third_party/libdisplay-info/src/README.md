# libdisplay-info

EDID and DisplayID library.

Goals:

- Provide a set of high-level, easy-to-use, opinionated functions as well as
  low-level functions to access detailed information.
- Simplicity and correctness over performance and resource usage.
- Well-tested and fuzzed.

Documentation is available on the [website].

## Using

The public API headers are categorised as either high-level or low-level API
as per the comments in the header files. Users of libdisplay-info should prefer
high-level API over low-level API when possible.

If high-level API lacks needed features, please propose additions to the
high-level API upstream before using low-level API to get what you need.
If the additions are rejected, you are welcome to use the low-level API.

This policy is aimed to propagate best practises when interpreting EDID
and DisplayID information which can often be cryptic or even inconsistent.

libdisplay-info uses [semantic versioning]. The public API is not yet stable.

## Contributing

Open issues and merge requests on the [GitLab project]. Discuss and ask
questions in the [#wayland] IRC channel on OFTC.

In general, the [Wayland contribution guidelines] should be followed. In
particular, each commit must carry a Signed-off-by tag to denote that the
submitter adheres to the [Developer Certificate of Origin 1.1]. This project
follows the [freedesktop.org Contributor Covenant].

## Building

libdisplay-info has the following dependencies:

- [hwdata] for the PNP ID database used at build-time only.

libdisplay-info is built using [Meson]:

    meson setup build/
    ninja -C build/

## Testing

The low-level EDID library is tested against [edid-decode]. `test/data/`
contains a small collection of EDID blobs and diffs between upstream
`edid-decode` and our `di-edid-decode` clone. Our CI ensures the diffs are
up-to-date. A patch should never make the diffs grow larger. To re-generate the
test data, build `edid-decode` at the Git revision mentioned in
`.gitlab-ci.yml`, put the executable in `PATH`, and run
`ninja -C build/ gen-test-data`.

The latest code coverage report is available on [GitLab CI][coverage].

## Fuzzing

To fuzz libdisplay-info with [AFL], the library needs to be instrumented:

    CC=afl-gcc meson build/
    ninja -C build/
    afl-fuzz -i test/data/ -o afl/ build/di-edid-decode/di-edid-decode

[website]: https://emersion.pages.freedesktop.org/libdisplay-info/
[semantic versioning]: https://semver.org/
[GitLab project]: https://gitlab.freedesktop.org/emersion/libdisplay-info
[#wayland]: ircs://irc.oftc.net/#wayland
[Wayland contribution guidelines]: https://gitlab.freedesktop.org/wayland/wayland/-/blob/main/CONTRIBUTING.md
[Developer Certificate of Origin 1.1]: https://developercertificate.org/
[freedesktop.org Contributor Covenant]: https://www.freedesktop.org/wiki/CodeOfConduct/
[hwdata]: https://github.com/vcrhonek/hwdata
[Meson]: https://mesonbuild.com/
[coverage]: https://gitlab.freedesktop.org/emersion/libdisplay-info/-/jobs/artifacts/main/file/build/meson-logs/coveragereport/index.html?job=build-gcc
[edid-decode]: https://git.linuxtv.org/edid-decode.git/
[AFL]: https://lcamtuf.coredump.cx/afl/
