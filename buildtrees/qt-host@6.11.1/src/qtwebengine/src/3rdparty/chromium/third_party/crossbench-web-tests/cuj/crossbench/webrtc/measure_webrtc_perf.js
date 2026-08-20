// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const webRTCCoolDownDuration = 5000;

// Wait for peer connection stabilized.
async function readCodec(peerConnection) {
    const stats = await peerConnection.getStats(null);
    if (!stats) {
        throw new Error("getStats() failed");
    }
    let codecReport = null;
    for (const [_, report] of stats) {
        if (report['type'] === 'codec') {
            codecReport = report;
        }
    }
    if (!codecReport) {
        throw new Error("stat not found");
    }

    if (!codecReport.mimeType) {
        throw new Error("mimetype is not filled");
    }
    for (const codec of ["H264", "VP8", "VP9", "AV1"]) {
        if (codecReport.mimeType.includes(codec)) {
            return codec;
        }
    }
    throw new Error(`unknown mimeType ${codecReport.mimeType}`);
}

async function readRTCReport(id, displayCapture, decode) {
    let peerConnection, staticType = null;
    if (displayCapture) {
        peerConnection = VC.displayLocalPC
        staticType = "outbound-rtp"
    } else {
        if (decode) {
            peerConnection = VC.remotePCs[id]
            staticType = "inbound-rtp"
        } else {
            peerConnection = VC.localPCs[id]
            staticType = "outbound-rtp"
        }
    }
    const stats = await peerConnection.getStats(null);
    if (!stats) {
        throw new Error("getStats() failed");
    }
    var currentReport = null;
    for (const [_, report] of stats) {
        if (report['type'] === staticType &&
            (!currentReport || currentReport['frameHeight'] < report['frameHeight'])) {
            currentReport = report;
        }
    }
    if (!currentReport) {
        throw new Error("stat not found");
    }
    return currentReport;
}

function waitForStabilized(numPeople) {
    const encoderConfig = VC.getEncoderConfig(numPeople);
    const streamHeight = encoderConfig.outputHeight;
    const streamWidth = streamHeight * 16 / 9;

    return new Promise((resolve, reject) => {
        const startTime = Date.now();
        const timeoutDuration = 60000;
        const interval = 1000;
        const mainPeerConnectionIndex = numPeople - 2;

        function checkStabilized() {
            if (Date.now() - startTime > timeoutDuration) {
                reject(new Error("Timeout exceeds"));
                return;
            }

            readCodec(VC.localPCs[mainPeerConnectionIndex])
                .then(codec => {
                    if (codec != encoderConfig.Codec) {
                        console.warn(`Codec doesn't match, got: ${codec}, expected: ${encoderConfig.Codec}`);
                        setTimeout(checkStabilized, interval);
                        return;
                    }
                    return readRTCReport(mainPeerConnectionIndex, false, false);
                })
                .then(txm => {
                    if (txm) {
                        if (txm.frameHeight != streamHeight || txm.frameWidth != streamWidth) {
                            console.warn(`Unexpected frame size, got height: ${txm.frameHeight}, width: ${txm.frameWidth}, expected height: ${streamHeight}, width: ${streamWidth}`);
                            setTimeout(checkStabilized, interval);
                            return;
                        }
                        if (encoderConfig.scalabilityMode != "" && !encoderConfig.scalabilityMode.startsWith("L1") && encoderConfig.scalabilityMode != txm.scalabilityMode) {
                            console.warn(`Unexpected scalability mode, got: ${txm.scalabilityMode}, expected: ${encoderConfig.scalabilityMode}`);
                            setTimeout(checkStabilized, interval);
                            return;
                        }
                        resolve(true);
                    }
                })
                .catch(err => {
                    console.warn(`Failed to read RTC report or codec: ${err}`);
                    setTimeout(checkStabilized, interval);
                });
        }

        checkStabilized();
    });
}

async function readMeasurement(timeSamples, readRTCReport, id, displayCapture, decode) {
    let measurements = [];
    for (let i = 0; i < timeSamples; i++) {
        // sleep 1 second so that getStats() is not called too many times in a short term.
        await new Promise(r => setTimeout(r, 1000));
        try {
            let report = await readRTCReport(id, displayCapture, decode);
            measurements.push(report);
        } catch (err) {
            throw new Error(`failed to collect RTC report: ${err}`);
        }
    }
    return measurements
}

async function measureRTCDecodeStats(id) {
    // timeSamples specifies number of frame decode time samples to get.
    const timeSamples = 10;
    let rxMeasurements = [];
    try {
        rxMeasurements = await readMeasurement(timeSamples, readRTCReport, id, false, true);
    } catch (err) {
        throw new Error(`failed to read rx measurement: ${err}`);
    }
    // Calculate decode time.
    let rxDecodeTime = [];
    for (let i = 1; i < rxMeasurements.length; i++) {
        if (rxMeasurements[i].framesDecoded == rxMeasurements[i - 1].framesDecoded) {
            continue;
        }
        const averageDecodeTime = (rxMeasurements[i].totalDecodeTime - rxMeasurements[i - 1].totalDecodeTime) / (rxMeasurements[i].framesDecoded - rxMeasurements[i - 1].framesDecoded) * 1000;
        rxDecodeTime.push(averageDecodeTime);
    }
    const lastRxm = rxMeasurements.length - 1
    const rxDroppedFramesPercentage = rxMeasurements[lastRxm].framesDropped / rxMeasurements[lastRxm].framesDecoded;
    let decodePerf = {};
    decodePerf.rxDecodeTime = rxDecodeTime;
    decodePerf.rxDroppedFramesPercentage = rxDroppedFramesPercentage;
    return decodePerf
}

async function measureRTCEncodeStats(id, displayCapture) {
    // timeSamples specifies number of frame decode time samples to get.
    const timeSamples = 10;
    let txMeasurements = [];
    try {
        txMeasurements = await readMeasurement(timeSamples, readRTCReport, id, displayCapture, false);
    } catch (err) {
        throw new Error(`failed to read tx measurement: ${err}`);
    }
    // Calculate encode time.
    let txEncodeTime = [];
    for (let i = 1; i < txMeasurements.length; i++) {
        if (txMeasurements[i].framesEncoded == txMeasurements[i - 1].framesEncoded) {
            continue;
        }
        const averageEncodeTime = (txMeasurements[i].totalEncodeTime - txMeasurements[i - 1].totalEncodeTime) / (txMeasurements[i].framesEncoded - txMeasurements[i - 1].framesEncoded) * 1000;
        txEncodeTime.push(averageEncodeTime);
    }
    let framesPerSecond = [];
    for (const txMeasurement of txMeasurements) {
        framesPerSecond.push(txMeasurement.framesPerSecond)
    }
    let encodePerf = {};
    encodePerf.txEncodeTime = txEncodeTime;
    encodePerf.framesPerSecond = framesPerSecond;
    return encodePerf
}

function measureRTCStats(numDecoders) {
    let decodePromises = [];
    let encodePromise;
    for (let i = 0; i < numDecoders; i++) {
        decodePromises.push(measureRTCDecodeStats(i));
        if (i == numDecoders - 1) {
            encodePromise = measureRTCEncodeStats(numDecoders - 1, false);
        }
    }
    return [decodePromises, encodePromise];
}

function getMedianDecodePerf(decodePerfs) {
    const average = array => array.reduce((a, b) => a + b) / array.length;
    decodePerfs.sort(
        (a, b) => average(a.rxDecodeTime) - average(b.rxDecodeTime)
    )
    return decodePerfs[Math.floor(decodePerfs.length / 2)]
}