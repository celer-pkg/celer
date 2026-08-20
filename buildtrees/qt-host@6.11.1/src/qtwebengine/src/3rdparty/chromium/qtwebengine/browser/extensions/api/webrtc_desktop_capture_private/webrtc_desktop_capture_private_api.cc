// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

// based on //chrome/browser/extensions/api/webrtc_desktop_capture_private/webrtc_desktop_capture_private_api.cc
// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "qtwebengine/browser/extensions/api/webrtc_desktop_capture_private/webrtc_desktop_capture_private_api.h"

#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/desktop_media_id.h"
#include "content/public/browser/desktop_streams_registry.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/media_stream_request.h"
#include "content/public/common/origin_util.h"
#include "extensions/browser/extension_host.h"
#include "extensions/browser/process_manager.h"
#include "qtwebengine/common/extensions/api/webrtc_desktop_capture_private.h"
#include "third_party/blink/public/mojom/mediastream/media_stream.mojom.h"

using extensions::api::webrtc_desktop_capture_private::Options;
using extensions::api::webrtc_desktop_capture_private::ChooseDesktopMedia::Results::Create;

namespace extensions {

namespace {

const char kTargetNotFoundError[] = "The specified target is not found.";

}  // namespace

WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::
    WebrtcDesktopCapturePrivateChooseDesktopMediaFunction() {
}

WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::
    ~WebrtcDesktopCapturePrivateChooseDesktopMediaFunction() {
      DesktopCaptureRequestsRegistry::GetInstance()->RemoveRequest(
      source_process_id(), request_id_);
}

ExtensionFunction::ResponseAction
WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::Run() {
  using Params =
      extensions::api::webrtc_desktop_capture_private::ChooseDesktopMedia
          ::Params;
  EXTENSION_FUNCTION_VALIDATE(args().size() > 0);
  const auto& request_id_value = args()[0];
  EXTENSION_FUNCTION_VALIDATE(request_id_value.is_int());
  request_id_ = request_id_value.GetInt();
  DesktopCaptureRequestsRegistry::GetInstance()->AddRequest(source_process_id(),
                                                            request_id_, this);
  GetMutableArgs().erase(args().begin());

  std::optional<Params> params = Params::Create(args());
  EXTENSION_FUNCTION_VALIDATE(params);

  content::RenderFrameHost* rfh = content::RenderFrameHost::FromID(
      params->request.guest_process_id,
      params->request.guest_render_frame_id);

  if (!rfh) {
    return RespondNow(Error(kTargetNotFoundError));
  }

  const url::Origin origin = rfh->GetLastCommittedOrigin();
  content::WebContents* web_contents =
      content::WebContents::FromRenderFrameHost(rfh);
  if (!web_contents) {
    return RespondNow(Error(kTargetNotFoundError));
  }

  content::RenderFrameHost* const main_frame = web_contents->GetPrimaryMainFrame();
  content::MediaStreamRequest request(main_frame->GetProcess()->GetDeprecatedID() /* render_process_id */,
                                      main_frame->GetRoutingID() /* render_frame_id */,
                                      request_id_ /* page_request_id */,
                                      origin /* security_origin */,
                                      true /* user_gesture */,
                                      blink::MediaStreamRequestType::MEDIA_DEVICE_ACCESS /* request_type */,
                                      {} /* requested_audio_device_ids */,
                                      {} /* requested_video_device_ids */,
                                      blink::mojom::MediaStreamType::GUM_DESKTOP_AUDIO_CAPTURE /* audio_type */,
                                      blink::mojom::MediaStreamType::GUM_DESKTOP_VIDEO_CAPTURE /* video_type */,
                                      true /* disable_local_echo */,
                                      false /* request_pan_tilt_zoom_permission */,
                                      false /* captured_surface_control_active */);

  extensions::ExtensionHost *host = extensions::ProcessManager::Get(browser_context())->GetBackgroundHostForExtension(extension_id());
  host->RequestMediaAccessPermission(web_contents, request,
                                     base::BindOnce(&WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::ProcessAccessRequestResponse,
                                                    this, main_frame, origin.GetURL()));

  return RespondLater();
}

void WebrtcDesktopCapturePrivateChooseDesktopMediaFunction::ProcessAccessRequestResponse(
      content::RenderFrameHost* const main_frame,
      const GURL &origin,
      const blink::mojom::StreamDevicesSet& devicesSet,
      blink::mojom::MediaStreamRequestResult stream_request_result,
      std::unique_ptr<content::MediaStreamUI> stream_ui)
{
  if (stream_request_result != blink::mojom::MediaStreamRequestResult::OK) {
    // The request is canceled either by the desktopMediaRequest or the permission request.
    // Respond with no arguments to mimic DesktopCaptureChooseDesktopMediaFunctionBase::Cancel()
    // form desktop_capture_base.cc.
    Respond(ArgumentList(Create(std::string(), Options())));
    return;
  }

  DCHECK(!devicesSet.stream_devices.empty());

  content::DesktopMediaID source = content::DesktopMediaID(content::DesktopMediaID::TYPE_SCREEN, 0);
  auto it = devicesSet.stream_devices.begin();
  for (; it != devicesSet.stream_devices.end(); ++it) {
    content::DesktopMediaID id = content::DesktopMediaID::Parse((*it)->video_device->id);
    if (id.type == content::DesktopMediaID::TYPE_SCREEN ||
        id.type == content::DesktopMediaID::TYPE_WINDOW) {
        source = id;
        break;
    }
  }

  std::string result = content::DesktopStreamsRegistry::GetInstance()->RegisterStream(
        main_frame->GetProcess()->GetDeprecatedID(), main_frame->GetRoutingID(),
        url::Origin::Create(origin), source, content::kRegistryStreamTypeDesktop);

  Options options;
  options.can_request_audio_track = source.audio_share;
  Respond(ArgumentList(Create(result, options)));
}

DesktopCaptureRequestsRegistry::RequestId::RequestId(int process_id,
                                                     int request_id)
    : process_id(process_id), request_id(request_id) {}

bool DesktopCaptureRequestsRegistry::RequestId::operator<(
    const RequestId& other) const {
  return std::tie(process_id, request_id) <
         std::tie(other.process_id, other.request_id);
}

DesktopCaptureRequestsRegistry::DesktopCaptureRequestsRegistry() {}
DesktopCaptureRequestsRegistry::~DesktopCaptureRequestsRegistry() {}

// static
DesktopCaptureRequestsRegistry* DesktopCaptureRequestsRegistry::GetInstance() {
  return base::Singleton<DesktopCaptureRequestsRegistry>::get();
}

void DesktopCaptureRequestsRegistry::AddRequest(
    int process_id,
    int request_id,
    WebrtcDesktopCapturePrivateChooseDesktopMediaFunction* handler) {
  requests_.insert(
      RequestsMap::value_type(RequestId(process_id, request_id), handler));
}

void DesktopCaptureRequestsRegistry::RemoveRequest(int process_id,
                                                   int request_id) {
  requests_.erase(RequestId(process_id, request_id));
}

}  // namespace extensions
