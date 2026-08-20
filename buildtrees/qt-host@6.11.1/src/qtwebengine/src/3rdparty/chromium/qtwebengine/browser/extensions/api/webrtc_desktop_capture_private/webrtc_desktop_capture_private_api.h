// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/extensions/api/webrtc_desktop_capture_private/webrtc_desktop_capture_private_api.h
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBENGINE_BROWSER_EXTENSIONS_API_WEBRTC_DESKTOP_CAPTURE_PRIVATE_WEBRTC_DESKTOP_CAPTURE_PRIVATE_API_H_
#define WEBENGINE_BROWSER_EXTENSIONS_API_WEBRTC_DESKTOP_CAPTURE_PRIVATE_WEBRTC_DESKTOP_CAPTURE_PRIVATE_API_H_

#include <map>

#include "base/memory/singleton.h"
#include "base/memory/weak_ptr.h"
#include "content/public/browser/media_stream_request.h"
#include "extensions/browser/extension_function.h"
#include "third_party/blink/public/common/mediastream/media_stream_request.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom-shared.h"

namespace content {
class RenderFrameHost;
}

namespace extensions {

class WebrtcDesktopCapturePrivateChooseDesktopMediaFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("webrtcDesktopCapturePrivate.chooseDesktopMedia",
                             WEBRTCDESKTOPCAPTUREPRIVATE_CHOOSEDESKTOPMEDIA)
  WebrtcDesktopCapturePrivateChooseDesktopMediaFunction();

 private:
  ~WebrtcDesktopCapturePrivateChooseDesktopMediaFunction() override;
  void ProcessAccessRequestResponse(
      content::RenderFrameHost* const main_frame,
      const GURL &origin,
      const blink::mojom::StreamDevicesSet& devicesSet,
      blink::mojom::MediaStreamRequestResult stream_request_result,
      std::unique_ptr<content::MediaStreamUI> stream_ui);

  // ExtensionFunction overrides.
  ResponseAction Run() override;

  int request_id_;
};

class DesktopCaptureRequestsRegistry {
 public:
  DesktopCaptureRequestsRegistry();
  ~DesktopCaptureRequestsRegistry();

  static DesktopCaptureRequestsRegistry* GetInstance();

  void AddRequest(int process_id,
                  int request_id,
                  WebrtcDesktopCapturePrivateChooseDesktopMediaFunction* handler);
  void RemoveRequest(int process_id, int request_id);

 private:
  friend struct base::DefaultSingletonTraits<DesktopCaptureRequestsRegistry>;

  struct RequestId {
    RequestId(int process_id, int request_id);

    // Need to use RequestId as a key in std::map<>.
    bool operator<(const RequestId& other) const;

    int process_id;
    int request_id;
  };

  using RequestsMap =
      std::map<RequestId, WebrtcDesktopCapturePrivateChooseDesktopMediaFunction*>;

  RequestsMap requests_;
};

}  // namespace extensions

#endif  // WEBENGINE_BROWSER_EXTENSIONS_API_WEBRTC_DESKTOP_CAPTURE_PRIVATE_WEBRTC_DESKTOP_CAPTURE_PRIVATE_API_H_
