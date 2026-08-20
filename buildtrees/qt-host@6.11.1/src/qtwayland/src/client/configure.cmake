# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# configure.cmake for the WaylandClientFeaturesPrivate module

# The following client-side features have not been moved
# to qtbase, as they are very Qt specific.

qt_feature("wayland-client-ivi-shell" PRIVATE
    LABEL "ivi-shell"
    CONDITION QT_FEATURE_wayland_client
)
qt_feature("wayland-client-qt-shell" PRIVATE
    LABEL "qt-shell"
    CONDITION QT_FEATURE_wayland_client
)

# And this feature depends on QtSvg, so we can't have it in qtbase

qt_feature("wayland-decoration-adwaita" PRIVATE
    LABEL "GNOME-like client-side decorations"
    CONDITION NOT WIN32 AND QT_FEATURE_wayland_client AND TARGET Qt::DBus AND TARGET Qt::Svg
)

qt_configure_add_summary_section(NAME "Wayland Client")
qt_configure_add_summary_section(NAME "Shell Integrations")
qt_configure_add_summary_entry(ARGS "wayland-client-ivi-shell")
qt_configure_add_summary_entry(ARGS "wayland-client-qt-shell")
qt_configure_end_summary_section() # end of "Shell Integrations" section
qt_configure_add_summary_section(NAME "Decoration Plugins")
qt_configure_add_summary_entry(ARGS "wayland-decoration-adwaita")
qt_configure_end_summary_section() # end of "Decoration Plugins" section
qt_configure_end_summary_section() # end of "Wayland Client" section
