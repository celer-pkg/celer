# VA-API fake backend for libva

VA-API is an API for video/image decoding/encoding acceleration implemented by
[libva] ([x11-libs/libva]). `libva-fake-driver` provides a fake backend for it
for VMs and other test-related images.

It can be explicitly exercised by running e.g.:

    LIBVA_DRIVER_NAME="fake" vainfo

wherever it might be installed. See https://tinyurl.com/libva-fake-driver for
more information.

[libva]: https://github.com/intel/libva
[x11-libs/libva]: https://chromium.googlesource.com/chromiumos/overlays/chromiumos-overlay/+/refs/heads/master/x11-libs/libva/
