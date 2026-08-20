// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#define STRINGIFY_LITERAL(x) #x
#define STRINGIFY(x) STRINGIFY_LITERAL(x)
#define CAT(x,y) CAT2(x,y)
#define CAT2(x,y) x##y
// headers mapping begin
// This file could be auto generaterd in the future
// for now it should match generated_union_sources_in_modules
// in generated_in_modules.gni
#define ORGINAL_blob_htmlcanvaselement_htmlimageelement_htmlvideoelement_imagebitmap_imagedata_offscreencanvas_svgimageelement_videoframe _blob_htmlcanvaselement_htmlimageelement_htmlvideoelement_imagebitmap_imagedata_offscreencanvas_svgimageelement_videoframe.h
#define REPLACE_blob_htmlcanvaselement_htmlimageelement_htmlvideoelement_imagebitmap_imagedata_offscreencanvas_svgimageelement_videoframe _blob_htmlcanvaselement_htmlimageelement_htmlvideoelement_imagebitmap_.h

#define ORGINAL_cssimagevalue_htmlcanvaselement_htmlimageelement_htmlvideoelement_imagebitmap_offscreencanvas_svgimageelement_videoframe _cssimagevalue_htmlcanvaselement_htmlimageelement_htmlvideoelement_imagebitmap_offscreencanvas_svgimageelement_videoframe.h
#define REPLACE_cssimagevalue_htmlcanvaselement_htmlimageelement_htmlvideoelement_imagebitmap_offscreencanvas_svgimageelement_videoframe _cssimagevalue_htmlcanvaselement_htmlimageelement_htmlvideoelement_.h

#if _MSC_VER
#define REPLACE_TOKEN(x) REPLACE_ ## x
#define SHORT_INCLUDE_FILE(DIRECTORY,FILENAME) STRINGIFY(CAT(DIRECTORY,REPLACE_TOKEN(FILENAME)))
#else
#define ORGINAL_TOKEN(x) ORGINAL_ ## x
#define SHORT_INCLUDE_FILE(DIRECTORY,FILENAME) STRINGIFY(CAT(DIRECTORY,ORGINAL_TOKEN(FILENAME)))
#endif
