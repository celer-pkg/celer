// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


// TransformkSVCStream extracts encoded video frames from the |scalabilityMode|
// (k-SVC) stream and feeds into a video decoder so that the input stream is
// decodable k-SVC stream whose spatial index is |maxDecodeSpatialIndex|.
// Because of the bitrate adaptation in a WebRTC peer connection, the encoded
// video frames doesn't necessary compose a |scalabilityMode| stream. In this
// case, TransformkSVCStream inputs the current top spatial index layer.
//
// Caveat: WebRTC Encoded Transform API doesn't invoke transform() in decode
// order, but calls transform() as a frame is assembled from received packets.
// This implementation is not resilient to the reordered frames. Handling the
// frame dependency is troublesome so we don't implement it assuming the
// reordering seldom happens.
class TransformkSVCStream {
  constructor(scalabilityMode, maxDecodeSpatialIndex) {
    this.maxDecodeSpatialIndex = maxDecodeSpatialIndex;
    this.currentTopSpatialLayer = 0;
    this.decodeLowerLayers = false;
    if (!scalabilityMode.endsWith('KEY')) {
      console.log('Unexpected scalabilityMode: ', scalabilityMode);
    }
  }
  async transform(frame, controller) {
    const metadata = frame.getMetadata();
    if (metadata.spatialIndex > this.maxDecodeSpatialIndex) {
      return;
    }
    const isKeyFrame = frame.type == 'key';
    if (isKeyFrame) {
      if (metadata.spatialIndex !== 0) {
        console.log('Keyframe is only in the bottom spatial layer');
        return;
      }
      this.decodeLowerLayers = true;
    } else if (metadata.spatialIndex == 0) {
      this.decodeLowerLayers = false;
    }

    if (this.decodeLowerLayers) {
      this.currentTopSpatialLayer = metadata.spatialIndex;
    }
    const decode =
      metadata.spatialIndex == this.currentTopSpatialLayer ||
      this.decodeLowerLayers;
    if (decode) {
      controller.enqueue(frame);
    }
  }
}

class VideoConference {
  constructor() {
    this.cameraPreview = document.getElementById('gum');
    this.cameraPreview.muted = true;
    this.cameraPreview.style.transform = 'scaleX(-1)';
    this.cameraPreview.width = 1280;
    this.cameraPreview.height = 720;

    this.cameraStream = null;
    this.sentStream = null;
    this.receiverVideos = [];
    this.localPCs = [];
    this.remotePCs = [];
  }

  async getUserMedia(audio) {
    let constraints = {
      video: {
        width: { ideal: 1920, max: 1920, min: 1280 },
        height: { ideal: 1080, max: 1080, min: 720 },
        aspectRatio: { exact: 1.77778 },
        frameRate: 30,
        facingMode: { ideal: 'user' },
      },
    };
    if (audio) {
      constraints.audio = {
        echoCancellation: true,
        autoGainControl: true,
        noiseSuppression: true,
      };
    }

    let stream = await navigator.mediaDevices.getUserMedia(constraints);
    this.cameraStream = stream;
    this.sentStream = stream;
  }

  async startedMediaStream(media) {
    await new Promise(function (resolve) {
      media.onplaying = (e) => {
        resolve();
      };
    });
  }

  // Starts camera capturing.
  async startCamera() {
    await this.getUserMedia(false);
  }

  // Starts audio capturing in addition to camera.
  async micOn() {
    await this.getUserMedia(true);
  }

  // Get the camera resolution.
  getCameraResolution() {
    const streamSettings = this.sentStream.getVideoTracks()[0].getSettings();
    return { width: streamSettings.width, height: streamSettings.height };
  }

  // Get the display capture resolution. This must not be called if
  // present() is not called.
  getDisplayCaptureResolution() {
    const streamSettings = this.displayStream.getVideoTracks()[0].getSettings();
    return { width: streamSettings.width, height: streamSettings.height };
  }

  // Shows the camera preview.
  async showCameraPreview() {
    this.cameraPreview.srcObject = this.sentStream;
    await this.startedMediaStream(this.cameraPreview);
  }

  obtainSsrcFromMid(description) {
    const lines = description.sdp.split('\r\n');
    let midFound = false;
    for (let i = 0; i < lines.length; ++i) {
      const line = lines[i];
      if (line.startsWith('a=mid:0')) {
        midFound = true;
      }
      if (midFound && line.startsWith('a=ssrc:')) {
        const spaceIndex = line.indexOf(' ');
        const ssrc = line.substr(7, spaceIndex - 7);
        return ssrc;
      }
    }
    return null;
  }

  async connect(localPC, remotePC, codec) {
    function findFirstCodec(name) {
      return RTCRtpReceiver.getCapabilities(name.split('/')[0]).codecs.filter(
        (c) =>
          c.mimeType.localeCompare(name, undefined, { sensitivity: 'base' }) ===
          0
      )[0];
    }

    localPC.onicecandidate = (e) => remotePC.addIceCandidate(e.candidate);
    remotePC.onicecandidate = (e) => localPC.addIceCandidate(e.candidate);

    const senders = localPC.getSenders();
    if (senders.length !== 1) {
      console.log('Unexpected senders length: ', senders);
      return;
    }
    if (codec !== 'VP9' && codec !== 'VP8') {
      console.log('Unexpected codec: ', codec);
      return;
    }
    let sender = senders[0];
    let params = sender.getParameters();
    params.degradationPreference = 'maintain-resolution';
    const rtcRTPCodec = findFirstCodec('video/' + codec);
    params.encodings[0].codec = rtcRTPCodec;
    await sender.setParameters(params);

    let offer = await localPC.createOffer();
    await localPC.setLocalDescription(offer);
    await remotePC.setRemoteDescription(localPC.localDescription);
    await remotePC.setLocalDescription();
    await localPC.setRemoteDescription(remotePC.localDescription);
    return this.obtainSsrcFromMid(localPC.localDescription, 0);
  }

  // Computes width and height for each video and the grid columns, which fits
  // the videos in the page.
  getVideoGridStyle(N) {
    const columns = Math.ceil(Math.sqrt(N));
    const rows = (N + columns - 1) / columns;
    const docW = document.documentElement.clientWidth;
    const docH = document.documentElement.clientHeight;
    const width = docW / columns;
    const height = docH / rows;
    return { width: width, height: height, columns: columns };
  }

  getTopSpatialIndex(scalabilityMode) {
    if (scalabilityMode == 'L2T3_KEY') {
      return 1;
    }
    return 0;
  }

  getEncoderConfig(numPeople) {
    const cameraHeight = this.getCameraResolution().height;
    let encodeHeight = 0;
    let scalabilityMode = '';
    if (numPeople == 2) {
      encodeHeight = cameraHeight;
      scalabilityMode = 'L1T3';
    } else if (numPeople <= 8) {
      encodeHeight = 360;
      scalabilityMode = 'L2T3_KEY';
    } else if (numPeople <= 16) {
      encodeHeight = 270;
      scalabilityMode = 'L2T3_KEY';
    } else {
      encodeHeight = 135;
      scalabilityMode = 'L1T3';
    }

    return {
      Codec: 'VP9',
      inputHeight: cameraHeight,
      outputHeight: encodeHeight,
      scalabilityMode: scalabilityMode,
    };
  }

  // Start a video call whose attendee is numPeople, which includes myself.
  // |numPeople-1| decoders and one encoder will run.
  async holdCall(numPeople, present) {
    this.resetCall();
    const numReceivers = numPeople - 1;

    let gridStyle;
    // If the number of people is more than 9, then put a camera preview in the
    // bottom right. Otherwise, the camera preview is shown in the same grid
    // view. If present is true, a grid style is forced and screen share preview
    // is shown in the next video to camera preview in the grid.
    if (numPeople > 9 && !present) {
      gridStyle = this.getVideoGridStyle(numReceivers);
      this.cameraPreview.width = 320;
      this.cameraPreview.height = 180;
      const cameraOverlay = document.getElementById('cameraOverlay');
      cameraOverlay.style.position = 'absolute';
      cameraOverlay.style.right = 0;
      cameraOverlay.style.bottom = 0;
      cameraOverlay.style.zIndex = 1;
    } else {
      // The display preview is shown in a video in a grid style.
      if (present) {
        gridStyle = this.getVideoGridStyle(numPeople + 1);
      } else {
        gridStyle = this.getVideoGridStyle(numPeople);
      }
      this.cameraPreview.width = gridStyle.width;
      this.cameraPreview.height = gridStyle.height;
    }

    let encCfg = this.getEncoderConfig(numPeople);
    let sendEncodings = {
      scaleResolutionDownBy: encCfg.inputHeight / encCfg.outputHeight,
      scalabilityMode: encCfg.scalabilityMode,
    };

    if (present) {
      this.displayPreview = document.createElement('video');
      this.displayPreview.autoplay = true;
      this.displayPreview.muted = true;
      this.displayPreview.width = gridStyle.width;
      this.displayPreview.height = gridStyle.height;
      const container = document.getElementById('container');
      const div = document.createElement('div');
      div.appendChild(this.displayPreview);
      container.appendChild(div);
      await this.present();
    }
    this.addVideos(numReceivers, gridStyle);
    await this.establishConnections(numReceivers, sendEncodings);
  }

  addVideos(numReceivers, gridStyle) {
    const container = document.getElementById('container');
    container.style.display = 'grid';
    container.style.gridTemplateColumns =
      'repeat(' + gridStyle.columns + ', 1fr)';
    // Let put camera preview a bottom right in grid.
    this.receiverVideos = new Array(numReceivers);
    for (let i = 0; i < numReceivers; i++) {
      const receiverVideo = document.createElement('video');
      receiverVideo.autoplay = true;
      receiverVideo.width = gridStyle.width;
      receiverVideo.height = gridStyle.height;
      this.receiverVideos[i] = receiverVideo;
      const div = document.createElement('div');
      div.appendChild(receiverVideo);
      container.appendChild(div);
    }
  }

  setUpHeaderExtension(transceiver) {
    // TODO(b/320375799), TODO(crbug.com/1513866): Don't set header extension
    // once spatialIndex and temporalIndex are filled without the dependency
    // descriptor header extension.
    const DependencyDescriptorURI =
      'http://www.webrtc.org/experiments/rtp-hdrext/generic-frame-descriptor-00';
    let headerExtensions = transceiver.getHeaderExtensionsToNegotiate();
    headerExtensions = headerExtensions.map((ext) => {
      if (ext.uri == DependencyDescriptorURI) {
        ext.direction = 'sendrecv';
      }
      return ext;
    });
    transceiver.setHeaderExtensionsToNegotiate(headerExtensions);
  }

  async establishConnections(numReceivers, sendEncodings) {
    this.localPCs = new Array(numReceivers);
    this.remotePCs = new Array(numReceivers);

    let clonedLocalPCWriters = new Array(numReceivers - 1);
    for (let i = 0; i < numReceivers - 1; i++) {
      const clonedLocalPC = new RTCPeerConnection({
        encodedInsertableStreams: true,
      });

      const clonedLocalTransceiver = clonedLocalPC.addTransceiver('video');
      this.setUpHeaderExtension(clonedLocalTransceiver);
      const clonedLocalPCWriter = clonedLocalTransceiver.sender
        .createEncodedStreams()
        .writable.getWriter();
      this.localPCs[i] = clonedLocalPC;
      clonedLocalPCWriters[i] = clonedLocalPCWriter;

      let remotePC = new RTCPeerConnection({ encodedInsertableStreams: true });
      this.remotePCs[i] = remotePC;
    }

    let mainLocalPC = new RTCPeerConnection({ encodedInsertableStreams: true });
    let mainLocalTransceiver = mainLocalPC.addTransceiver(
      this.sentStream.getVideoTracks()[0],
      {
        streams: [this.sentStream],
        sendEncodings: [sendEncodings],
      }
    );
    this.setUpHeaderExtension(mainLocalTransceiver);
    let mainLocalPCStream = mainLocalTransceiver.sender.createEncodedStreams();

    let mainRemotePC = new RTCPeerConnection({
      encodedInsertableStreams: true,
    });
    mainRemotePC.addTransceiver('video');

    this.localPCs[numReceivers - 1] = mainLocalPC;
    this.remotePCs[numReceivers - 1] = mainRemotePC;

    const topSpatialIndex = this.getTopSpatialIndex(
      sendEncodings.scalabilityMode
    );
    for (let i = 0; i < numReceivers; i++) {
      this.remotePCs[i].ontrack = (e) => {
        this.receiverVideos[i].srcObject = new MediaStream([e.track]);
        const receiver = e.receiver;
        const receiverStream = receiver.createEncodedStreams();
        receiverStream.readable
          .pipeThrough(
            new TransformStream(
              new TransformkSVCStream(
                sendEncodings.scalabilityMode,
                topSpatialIndex
              )
            )
          )
          .pipeTo(receiverStream.writable);
      };
    }
    let ssrcs = new Array(numReceivers - 1);
    for (let i = 0; i < numReceivers; i++) {
      const ssrc = await this.connect(
        this.localPCs[i],
        this.remotePCs[i],
        'VP9'
      );
      if (i < ssrcs.length) {
        ssrcs[i] = ssrc;
      }
    }

    mainLocalPCStream.readable
      .pipeThrough(
        new TransformStream({
          transform(frame, controller) {
            const metadata = frame.getMetadata();
            for (let i = 0; i < ssrcs.length; i++) {
              const clonedFrame = structuredClone(frame);
              const modifiedMetadata = structuredClone(metadata);
              modifiedMetadata.synchronizationSource = ssrcs[i];
              clonedFrame.setMetadata(modifiedMetadata);
              clonedLocalPCWriters[i].write(clonedFrame);
            }
            controller.enqueue(frame);
          },
        })
      )
      .pipeTo(mainLocalPCStream.writable);

    // TODO(hiroh): Wait for all the decoders to run.
    // playing event is not fired forever here. What event should I use?
  }

  resetCall() {
    for (let i = 0; i < this.remotePCs.length; i++) {
      this.remotePCs[i].close();
      this.remotePCs[i] = null;
    }
    for (let i = 0; i < this.localPCs.length; i++) {
      this.localPCs[i].close();
      this.localPCs[i] = null;
    }
    this.remotePCs = [];
    this.localPCs = [];
    const container = document.getElementById('container');
    const cameraOverlay = document.getElementById('cameraOverlay');
    while (container.lastChild && container.lastChild !== cameraOverlay) {
      container.removeChild(container.lastChild);
    }
  }

  async present() {
    let constraints = {
      audio: {
        echoCancellation: true,
        autoGainControl: true,
      },
      video: {
        framerate: { min: 30, max: 30 },
        displaySurface: 'browser', // Tab
      },
    };
    this.displayStream = await navigator.mediaDevices.getDisplayMedia(
      constraints
    );
    this.displayStream.getVideoTracks()[0].applyConstraints(constraints);
    this.displayPreview.srcObject = this.displayStream;

    this.displayLocalPC = new RTCPeerConnection({
      encodedInsertableStreams: true,
    });
    const displayLocalPCStream = this.displayLocalPC
      .addTransceiver(this.displayStream.getVideoTracks()[0], {
        streams: [this.displayStream],
        sendEncodings: [{ scalabilityMode: 'L1T3' }],
      })
      .sender.createEncodedStreams();

    const displayRemotePC = new RTCPeerConnection();
    displayRemotePC.addTransceiver('video');
    await this.connect(this.displayLocalPC, displayRemotePC, 'VP8');

    // Drop all encoded frames so that one encoder runs but no decoder runs for
    // screen sharing.
    displayLocalPCStream.readable
      .pipeThrough(
        new TransformStream({
          transform(frame, controller) {
            return;
          },
        })
      )
      .pipeTo(displayLocalPCStream.writable);
  }
}
