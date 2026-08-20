/*
 * Copyright (C) 2025 The Chromium Authors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "config.h"

#include "wayland/meta-wayland-ui-controls.h"

#include <glib.h>
#include <linux/input-event-codes.h>
#include <stdint.h>
#include <wayland-server-core.h>

#include "clutter/clutter.h"
#include "compositor/compositor-private.h"
#include "compositor/meta-window-actor-private.h"
#include "compositor/meta-window-drag.h"
#include "gio/gio.h"
#include "glib-object.h"
#include "meta/compositor.h"
#include "meta/display.h"
#include "meta/meta-context.h"
#include "meta/meta-debug.h"
#include "meta/meta-window-actor.h"
#include "ui-controls-unstable-v2-server-protocol.h"
#include "wayland/meta-wayland-private.h"
#include "wayland/meta-wayland-types.h"
#include "wayland/meta-wayland-versions.h"
#include "wayland/meta-wayland-xdg-shell.h"

typedef struct
{
  struct wl_resource *resource;
  uint32_t id;
  ClutterEventType type;
  uint64_t time_us;
  bool wait_for_pointer_focus;
} MetaWaylandUiControlsRequest;

typedef struct _MetaWaylandUiControls
{
  MetaWaylandCompositor *compositor;
  bool has_client;
  ClutterStage *stage;
  struct wl_global *global;
  ClutterVirtualInputDevice *virtual_pointer;
  ClutterVirtualInputDevice *virtual_keyboard;
  MetaWaylandEventHandler *event_handler;
  MetaWindow *window;
  MetaWindowDrag *window_drag;
  GQueue *requests;
  GQueue *pointer_move_requests_during_grab;
  GQueue *pointer_release_requests_during_grab;
} MetaWaylandUiControls;

static void
notify_request_done (MetaWaylandUiControlsRequest *request)
{

  meta_topic (META_DEBUG_INPUT, "%s %u", __func__, request->id);
  zcr_ui_controls_v2_send_request_processed (request->resource, request->id);
  g_free (request);
}

static gboolean
set_true (gpointer user_data)
{
  gboolean *done = user_data;

  *done = TRUE;

  return G_SOURCE_REMOVE;
}

static void
on_wayland_input_event_handled (MetaWaylandUiControls *ui_controls,
                                const ClutterEvent    *event)
{
  int n;
  gboolean timed_out;
  MetaWaylandUiControlsRequest *request;
  MetaWaylandPointer *pointer = ui_controls->compositor->seat->pointer;

  meta_topic (META_DEBUG_INPUT, "%s %s", __func__,
              clutter_event_get_name (event));
  for (n = 0; n < g_queue_get_length (ui_controls->requests); n++)
    {
      request = g_queue_peek_nth (ui_controls->requests, n);
      if (request->type == clutter_event_type (event) &&
          request->time_us == clutter_event_get_time_us (event))
        {
          if (request->wait_for_pointer_focus)
            {
              if (!meta_wayland_pointer_get_focus_surface (pointer))
                meta_topic (META_DEBUG_INPUT,
                            "waiting for surface to have pointer focus before "
                            "signaling done");
              timed_out = FALSE;
              guint timeout_id =
                g_timeout_add_seconds (1, set_true, &timed_out);
              while (!meta_wayland_pointer_get_focus_surface (pointer) &&
                     !timed_out)
                {
                  g_main_context_iteration (NULL, TRUE);
                  // Abort if the ui_controls client was destroyed.
                  if (!ui_controls->has_client)
                    {
                      meta_topic (META_DEBUG_INPUT,
                                  "aborting wait for pointer focus as "
                                  "ui_controls client was destroyed");
                      if (!timed_out)
                        g_clear_handle_id (&timeout_id, g_source_remove);
                      return;
                    }
                }
              if (timed_out)
                meta_warning ("timed out waiting for surface to have pointer "
                              "focus before signaling done");
              else
                g_clear_handle_id (&timeout_id, g_source_remove);
            }
          g_queue_remove (ui_controls->requests, request);
          notify_request_done (request);
          break;
        }
    }
}

static void
notify_request_done_during_grab (MetaWaylandUiControls *ui_controls,
                                 const ClutterEventType event_type)
{
  MetaWaylandUiControlsRequest *request;

  bool is_motion = event_type == CLUTTER_MOTION;
  GQueue *requests = is_motion
                         ? ui_controls->pointer_move_requests_during_grab
                         : ui_controls->pointer_release_requests_during_grab;
  guint queue_length = g_queue_get_length (requests);
  if (!is_motion && queue_length > 1)
    {
      // In case a test issues multiple release requests during window
      // grab, which is unlikely, release all of them on grab end as
      // best-effort.
      meta_warning ("Multiple release events were received during window "
                    "grab, so releasing all of them on grab end. Note: "
                    "This likely indicates a test bug.");
    }
  while (!g_queue_is_empty (requests))
    {
      request = g_queue_pop_head (requests);
      notify_request_done (request);
      // Consider one motion event as done.
      if (is_motion)
        break;
    }
}

static void
on_moved_during_grab (MetaWaylandUiControls *ui_controls)
{
  meta_topic (META_DEBUG_INPUT, "%s", __func__);
  notify_request_done_during_grab (ui_controls, CLUTTER_MOTION);
}

static void
on_resized_during_grab (MetaWaylandUiControls *ui_controls)
{
  meta_topic (META_DEBUG_INPUT, "%s", __func__);
  notify_request_done_during_grab (ui_controls, CLUTTER_MOTION);
}

static void
on_grab_ended (MetaWaylandUiControls *ui_controls)
{
  meta_topic (META_DEBUG_INPUT, "%s", __func__);
  notify_request_done_during_grab (ui_controls, CLUTTER_BUTTON_RELEASE);
}

/**
 * Listens for stage grab state and subscribes to window drag signals if a
 * window drag is found during grab state.
 **/
static void
on_stage_is_grabbed_change (MetaWaylandUiControls *ui_controls)
{
  bool stage_has_grab =
    clutter_stage_get_grab_actor (ui_controls->stage) != NULL;
  if (stage_has_grab)
    {
      MetaContext *context =
        meta_wayland_compositor_get_context (ui_controls->compositor);
      MetaDisplay *display = meta_context_get_display (context);
      MetaCompositor *compositor = meta_display_get_compositor (display);

      ui_controls->window_drag = meta_compositor_get_current_window_drag (
        META_COMPOSITOR (compositor));
      meta_topic (META_DEBUG_INPUT, "%s window drag: %p", __func__,
                  ui_controls->window_drag);
      if (ui_controls->window_drag)
        {
          g_signal_connect_swapped (ui_controls->window_drag, "ended",
                                    G_CALLBACK (on_grab_ended), ui_controls);
          g_signal_connect_swapped (
            ui_controls->window_drag, "update-resize-done",
            G_CALLBACK (on_resized_during_grab), ui_controls);
          g_signal_connect_swapped (
            ui_controls->window_drag, "update-move-done",
            G_CALLBACK (on_moved_during_grab), ui_controls);
        }
    }
  else
    {
      meta_topic (META_DEBUG_INPUT,
                  "%s stage doesn't have grab, clearing window drag", __func__);
      ui_controls->window_drag = NULL;
    }
}

/**
 * Stores the request until it is processed and `request_processed` is sent.
 **/
static void
store_request (MetaWaylandUiControls *ui_controls,
               struct wl_resource    *resource,
               uint32_t               request_id,
               ClutterEventType       type,
               uint64_t               time_us,
               bool                   wait_for_pointer_focus)
{
  MetaWaylandUiControlsRequest *request;
  request = g_new0 (MetaWaylandUiControlsRequest, 1);
  request->resource = resource;
  request->id = request_id;
  request->type = type;
  request->time_us = time_us;
  request->wait_for_pointer_focus = wait_for_pointer_focus;
  if (ui_controls->window_drag != NULL)
  // During window drag, motion and release events may not be handled by the
  // wayland input handler. So store these requests separately from the
  // regular requests. Window drag events will signal their completion.
  // See on_stage_is_grabbed_change().
    {
      if (type == CLUTTER_MOTION)
        {
          g_queue_push_tail (ui_controls->pointer_move_requests_during_grab,
                             request);
          return;
        }
      if (type == CLUTTER_BUTTON_RELEASE)
        {
          g_queue_push_tail (ui_controls->pointer_release_requests_during_grab,
                             request);
          return;
        }
    }
  g_queue_push_tail (ui_controls->requests, request);
}

static void
notify_modifiers (MetaWaylandUiControls *ui_controls,
                  uint32_t               pressed_modifiers,
                  uint32_t               key_state)
{
  if (pressed_modifiers != 0)
    {
      if (pressed_modifiers & ZCR_UI_CONTROLS_V2_MODIFIER_SHIFT)
        {
          clutter_virtual_input_device_notify_key (
            ui_controls->virtual_keyboard, g_get_monotonic_time (),
            KEY_LEFTSHIFT, key_state);
        }
      if (pressed_modifiers & ZCR_UI_CONTROLS_V2_MODIFIER_CONTROL)
        {
          clutter_virtual_input_device_notify_key (
            ui_controls->virtual_keyboard, g_get_monotonic_time (),
            KEY_LEFTCTRL, key_state);
        }
      if (pressed_modifiers & ZCR_UI_CONTROLS_V2_MODIFIER_ALT)
        {
          clutter_virtual_input_device_notify_key (
            ui_controls->virtual_keyboard, g_get_monotonic_time (),
            KEY_LEFTALT, key_state);
        }
    }
}

/**
 * Creates clutter virtual pointer and keyboard devices.
 **/
static void
create_virtual_input_devices (MetaWaylandUiControls *ui_controls)
{

  MetaWaylandCompositor *compositor = ui_controls->compositor;
  MetaContext *context =
    meta_wayland_compositor_get_context (compositor);
  MetaBackend *backend = meta_context_get_backend (context);
  ClutterBackend *clutter_backend =
    meta_backend_get_clutter_backend (backend);
  ClutterSeat *seat = clutter_backend_get_default_seat (clutter_backend);

  ui_controls->virtual_pointer =
    clutter_seat_create_virtual_device (seat, CLUTTER_POINTER_DEVICE);
  ui_controls->virtual_keyboard =
    clutter_seat_create_virtual_device (seat, CLUTTER_KEYBOARD_DEVICE);
}

static void
ui_controls_send_key_events (struct wl_client   *client,
                             struct wl_resource *resource,
                             uint32_t            key,
                             uint32_t            key_state,
                             uint32_t            pressed_modifiers,
                             uint32_t            id)
{
  MetaWaylandUiControls *ui_controls = wl_resource_get_user_data (resource);
  ClutterEventType event_type;
  uint64_t time_us;

  meta_topic (META_DEBUG_INPUT, "%s id=%u key=%u state=%u has_modifiers=%d",
              __func__, id, key, key_state, pressed_modifiers != 0);

  if (key_state & ZCR_UI_CONTROLS_V2_KEY_STATE_PRESS)
    {
      event_type = CLUTTER_KEY_PRESS;
      time_us = g_get_monotonic_time ();
      notify_modifiers (ui_controls, pressed_modifiers,
                        CLUTTER_KEY_STATE_PRESSED);
      clutter_virtual_input_device_notify_key (ui_controls->virtual_keyboard,
                                               time_us, key,
                                               CLUTTER_KEY_STATE_PRESSED);
    }
  if (key_state & ZCR_UI_CONTROLS_V2_KEY_STATE_RELEASE)
    {
      event_type = CLUTTER_KEY_RELEASE;
      time_us = g_get_monotonic_time ();
      clutter_virtual_input_device_notify_key (ui_controls->virtual_keyboard,
                                               time_us, key,
                                               CLUTTER_KEY_STATE_RELEASED);
      notify_modifiers (ui_controls, pressed_modifiers,
                        CLUTTER_KEY_STATE_RELEASED);
    }

  store_request (ui_controls, resource, id, event_type, time_us,
                 false /*wait_for_pointer_focus=*/);
}

static void
on_effects_completed (MetaWindowActor *window_actor,
                      gboolean        *done)
{
  meta_topic (META_DEBUG_INPUT, "window effects complete");
  *done = TRUE;
}

static void
wait_for_window_effects_completed (MetaWindowActor *window_actor)
{
  meta_topic (META_DEBUG_INPUT, "waiting for window effects to complete");
  gboolean done = FALSE;
  gulong handler_id;

  handler_id = g_signal_connect (window_actor, "effects-completed",
                                 G_CALLBACK (on_effects_completed), &done);

  while (!done)
    g_main_context_iteration (NULL, TRUE);

  g_signal_handler_disconnect (window_actor, handler_id);
}

static void
ui_controls_send_mouse_move(struct wl_client   *client,
                            struct wl_resource *resource,
                            int32_t             x,
                            int32_t             y,
                            struct wl_resource *surface_resource,
                            uint32_t            id) {
  MetaWaylandSurface *surface;
  MetaWindow *window;
  MetaWindowActor *window_actor;
  MetaWaylandUiControls *ui_controls = wl_resource_get_user_data (resource);
  float abs_x, abs_y;
  bool effects_in_progress = false;
  uint64_t time_us = g_get_monotonic_time ();
  // Wait for pointer focus if mouse move was requested relative to a surface.
  const bool wait_for_pointer_focus = surface_resource != 0;

  if (surface_resource)
  // A surface was provided. Use coordinates relative to it.
    {
      surface = wl_resource_get_user_data (surface_resource);
      window = meta_wayland_surface_get_window (surface);
      window_actor = meta_window_actor_from_window (window);
      effects_in_progress = meta_window_actor_effect_in_progress (window_actor);
      if (effects_in_progress)
        wait_for_window_effects_completed (window_actor);
      meta_wayland_surface_get_absolute_coordinates (surface, x, y, &abs_x,
                                                     &abs_y);
      abs_x = roundf (abs_x);
      abs_y = roundf (abs_y);
    }
  else
  // No surface was provided. Use global coordinates.
    {
      abs_x = x;
      abs_y = y;
    }

  meta_topic (META_DEBUG_INPUT,
              "%s id=%u x=%d y=%d has_surface=%d, abs_x=%f, abs_y=%f", __func__,
              id, x, y, surface_resource != 0, abs_x, abs_y);

  clutter_virtual_input_device_notify_absolute_motion (
    ui_controls->virtual_pointer, time_us, abs_x, abs_y);
  store_request (ui_controls, resource, id, CLUTTER_MOTION, time_us,
                 wait_for_pointer_focus);
}

static void
ui_controls_send_mouse_button (struct wl_client   *client,
                               struct wl_resource *resource,
                               uint32_t            button,
                               uint32_t            button_state,
                               uint32_t            pressed_modifiers,
                               uint32_t            id)
{
  MetaWaylandUiControls *ui_controls = wl_resource_get_user_data (resource);
  uint32_t clutter_button;
  ClutterEventType event_type;
  uint64_t time_us;

  meta_topic (META_DEBUG_INPUT, "%s id=%u button=%u state=%u has_modifiers=%d",
              __func__, id, button, button_state, pressed_modifiers != 0);

  switch (button)
    {
    case ZCR_UI_CONTROLS_V2_MOUSE_BUTTON_LEFT:
      clutter_button = CLUTTER_BUTTON_PRIMARY;
      break;
    case ZCR_UI_CONTROLS_V2_MOUSE_BUTTON_RIGHT:
      clutter_button = CLUTTER_BUTTON_SECONDARY;
      break;
    case ZCR_UI_CONTROLS_V2_MOUSE_BUTTON_MIDDLE:
      clutter_button = CLUTTER_BUTTON_MIDDLE;
      break;
    default:
      g_assert_not_reached ();
    }

  if (button_state & ZCR_UI_CONTROLS_V2_MOUSE_BUTTON_STATE_DOWN)
    {
      event_type = CLUTTER_BUTTON_PRESS;
      time_us = g_get_monotonic_time ();
      notify_modifiers (ui_controls, pressed_modifiers,
                        CLUTTER_KEY_STATE_PRESSED);
      clutter_virtual_input_device_notify_button (ui_controls->virtual_pointer,
                                                  time_us, clutter_button,
                                                  CLUTTER_BUTTON_STATE_PRESSED);
    }

  if (button_state & ZCR_UI_CONTROLS_V2_MOUSE_BUTTON_STATE_UP)
    {
      event_type = CLUTTER_BUTTON_RELEASE;
      time_us = g_get_monotonic_time ();
      clutter_virtual_input_device_notify_button (
        ui_controls->virtual_pointer, time_us, clutter_button,
        CLUTTER_BUTTON_STATE_RELEASED);
      notify_modifiers (ui_controls, pressed_modifiers,
                        CLUTTER_KEY_STATE_RELEASED);
    }

  store_request (ui_controls, resource, id, event_type, time_us,
                 false /*wait_for_pointer_focus=*/);
}

static const struct zcr_ui_controls_v2_interface meta_ui_controls_interface = {
  ui_controls_send_key_events,
  ui_controls_send_mouse_move,
  ui_controls_send_mouse_button,
};

static void
destroy_ui_controls (struct wl_resource *resource)
{
  MetaWaylandUiControls *ui_controls = wl_resource_get_user_data (resource);

  meta_topic (META_DEBUG_INPUT, "%s", __func__);

  ui_controls->has_client = false;

  // Ensure any pressed keys and buttons are released when a client resource is
  // destroyed.
  clutter_virtual_input_device_release_pressed (ui_controls->virtual_pointer);
  clutter_virtual_input_device_release_pressed (ui_controls->virtual_keyboard);

  // Clear all queues.
  g_queue_clear_full (ui_controls->requests, g_free);
  g_queue_clear_full (ui_controls->pointer_move_requests_during_grab, g_free);
  g_queue_clear_full (ui_controls->pointer_release_requests_during_grab,
                      g_free);
}

static void
bind_ui_controls (struct wl_client *client,
                  void             *data,
                  uint32_t          version,
                  uint32_t          id)
{
  MetaWaylandUiControls *ui_controls = data;
  struct wl_resource *resource;

  meta_topic (META_DEBUG_INPUT, "%s id=%u requested_version=%u", __func__, id,
              version);

  ui_controls->has_client = true;

  resource = wl_resource_create (client, &zcr_ui_controls_v2_interface,
                                 META_UI_CONTROLS_V2_VERSION, id);
  wl_resource_set_implementation (resource, &meta_ui_controls_interface,
                                  ui_controls, &destroy_ui_controls);
}

static MetaWaylandUiControls *
meta_wayland_ui_controls_new (MetaWaylandCompositor *compositor)
{
  MetaWaylandUiControls *ui_controls;

  ui_controls = g_new0 (MetaWaylandUiControls, 1);
  ui_controls->compositor = compositor;

  return ui_controls;
}

void
meta_wayland_ui_controls_init (MetaWaylandCompositor *compositor)
{
  struct wl_display *wayland_display;
  MetaContext *context;
  MetaBackend *backend;

  compositor->ui_controls = meta_wayland_ui_controls_new (compositor);
  create_virtual_input_devices (compositor->ui_controls);

  if (!compositor->ui_controls->global)
    {
      wayland_display =
        meta_wayland_compositor_get_wayland_display (compositor);

      compositor->ui_controls->global =
        wl_global_create (wayland_display,
                          &zcr_ui_controls_v2_interface,
                          META_UI_CONTROLS_V2_VERSION,
                          compositor->ui_controls, bind_ui_controls);
      if (!compositor->ui_controls->global)
        g_error ("Could not create ui controls global");
    }
  compositor->ui_controls->requests = g_queue_new ();
  compositor->ui_controls->pointer_move_requests_during_grab = g_queue_new ();
  compositor->ui_controls->pointer_release_requests_during_grab =
    g_queue_new ();
  g_signal_connect_swapped (compositor->seat->input_handler, "event-handled",
                            G_CALLBACK (on_wayland_input_event_handled),
                            compositor->ui_controls);
  context = meta_wayland_compositor_get_context (compositor);
  backend = meta_context_get_backend (context);
  compositor->ui_controls->stage =
    CLUTTER_STAGE (meta_backend_get_stage (backend));
  g_signal_connect_swapped (
    compositor->ui_controls->stage, "notify::is-grabbed",
    G_CALLBACK (on_stage_is_grabbed_change), compositor->ui_controls);
}

void
meta_wayland_ui_controls_finalize (MetaWaylandCompositor *compositor)
{
  g_signal_handlers_disconnect_by_func (compositor->ui_controls->stage,
                                        on_stage_is_grabbed_change,
                                        compositor->ui_controls);
  g_signal_handlers_disconnect_by_func (compositor->seat->input_handler,
                                        on_wayland_input_event_handled,
                                        compositor->ui_controls);
  g_clear_pointer (&compositor->ui_controls, g_free);
}
