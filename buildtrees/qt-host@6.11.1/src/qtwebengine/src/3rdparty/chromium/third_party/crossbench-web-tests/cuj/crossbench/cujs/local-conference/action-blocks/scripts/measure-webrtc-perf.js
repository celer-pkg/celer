// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const numPeople = $[NUM_PEOPLE];
const numDecoders = numPeople - 1;

await waitForStabilized(numPeople);
// Sleep 5s to eliminate the performance effect of rtc peer connection start up.
await new Promise(r => setTimeout(r, webRTCCoolDownDuration));

let [decodePromises, encodePromise] = measureRTCStats(numDecoders);
let encodePerf = await encodePromise;
performance.mark("tx.encode_time_camera_enc", { detail: encodePerf.txEncodeTime });
performance.mark("tx.frames_per_second_camera_enc", { detail: encodePerf.framesPerSecond });

let decodePerfs = await Promise.all(decodePromises);
let midDecodePerf = getMedianDecodePerf(decodePerfs);
performance.mark("rx.decode_time_median_dec", { detail: midDecodePerf.rxDecodeTime });
performance.mark("rx.dropped_frames_percentage_median_dec", { detail: midDecodePerf.rxDroppedFramesPercentage });