#!/bin/bash -eu
#
# Copyright 2024 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/bin/bash

# Fail on any error.
set -e

# Display commands being run.
set -x

print_usage() {
  echo "Usage: build-and-copy-to-androidx.sh <path-to-androidx-repo>"
  exit 1
}

if [[ $# -lt 1 ]]; then
  print_usage
fi

NDK_ARCHS="x86 x86_64 armeabi-v7a arm64-v8a"
# Optimized build with debug symbols.  The .so.stripped file is then fully
# stripped and the one we ship.  The .so file is retained for symbolizing stack
# traces.
RELEASE_ARGS="-c opt --copt=-g --stripopt=--strip-unneeded"
OUTDIR="$1"

for arch in $NDK_ARCHS; do
  bazel build --config=android-$arch $RELEASE_ARGS \
    ink/jni:libink.so ink/jni:libink.so.stripped
  mkdir -p $OUTDIR/$arch
  cp bazel-bin/ink/jni/libink.so $OUTDIR/$arch/libink.so.debug
  cp bazel-bin/ink/jni/libink.so.stripped $OUTDIR/$arch/libink.so
  chmod u+w $OUTDIR/$arch/libink.so
  chmod u+w $OUTDIR/$arch/libink.so.debug
done
