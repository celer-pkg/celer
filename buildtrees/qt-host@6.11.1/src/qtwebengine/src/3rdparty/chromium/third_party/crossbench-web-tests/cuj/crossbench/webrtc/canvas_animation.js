// Copyright 2025 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function nextFrame(t) {
  return new Promise(resolve => {
    // We could use here requestAnimationFrame(resolve) but since the workload
    // is relatively light, we'll get such frames quite fast (~60Hz).
    setTimeout(resolve, t)
  })
}

async function drawAlternatingColours(canvasId, framerate) {
  var context = canvasId.getContext('webgl', {alpha : true});
  for (let i = 0; i < framerate * 2; i++) {
    let v = 1.0 / (framerate * 2.0) * i ;
    context.clearColor(v, v, v, 1);
    context.clear(context.COLOR_BUFFER_BIT);
    await nextFrame(1000 / framerate);
  }
  drawAlternatingColours(canvasId, framerate);
}

async function drawCanvasAlternatingColours(width, height, framerate) {
  const canvas = document.getElementById('canvas');
  canvas.width = width;
  canvas.height = height;
  await drawAlternatingColours(canvas, framerate);
}
