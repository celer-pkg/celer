// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qplatformdefs.h"
#include "qurl.h"
#include "private/qdataurl_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals;

/*!
    \internal

    Decode a data: URL into its mimetype and payload. Returns a null string if
    the URL could not be decoded.
*/
Q_CORE_EXPORT bool qDecodeDataUrl(const QUrl &uri, QString &mimeType, QByteArray &payload)
{
    /* https://www.rfc-editor.org/rfc/rfc2397.html

        data:[<mediatype>][;base64],<data>
        dataurl    := "data:" [ mediatype ] [ ";base64" ] "," data
        mediatype  := [ type "/" subtype ] *( ";" parameter )
        data       := *urlchar
        parameter  := attribute "=" value
    */

    if (uri.scheme() != "data"_L1 || !uri.host().isEmpty())
        return false;

    payload = uri.toEncoded(QUrl::RemoveScheme);
    // parse it:
    // percent decode after finding the `,`, to workaround parameter
    // values containing a percent-encoded comma
    const qsizetype pos = payload.indexOf(',');
    if (pos != -1) {
        QByteArray contentType = payload.first(pos).percentDecoded();
        auto data = QLatin1StringView{contentType};
        data = data.trimmed();

        QLatin1StringView mime;
        QLatin1StringView charsetParam;
        constexpr auto charset = "charset"_L1;
        bool first = true;
        for (auto part : qTokenize(data, u';', Qt::SkipEmptyParts)) {
            part = part.trimmed();
            if (first) {
                if (part.contains(u'/'))
                    mime = part;
                first = false;
            }
            // Minimal changes, e.g. if it's "charset=;" or "charset;" without
            // an encoding, leave it as-is
            if (part.startsWith(charset, Qt::CaseInsensitive))
                charsetParam = part;

            if (!mime.isEmpty() && !charsetParam.isEmpty())
                break;
        }

        if (mime.isEmpty()) {
            mime = "text/plain"_L1;
            if (charsetParam.isEmpty())
                charsetParam = "charset=US-ASCII"_L1;
        }
        if (!charsetParam.isEmpty())
            mimeType = mime + u';' + charsetParam;
        else
            mimeType = mime;

        // find out if the payload is encoded in Base64
        constexpr auto base64 = ";base64"_L1; // per the RFC, at the end of `data`
        const bool isBas64 = data.endsWith(base64, Qt::CaseInsensitive);

        payload.slice(pos + 1);
        data = {};
        payload = std::move(payload).percentDecoded();

        if (isBas64) {
            auto r = QByteArray::fromBase64Encoding(std::move(payload));
            if (!r) {
                // just in case someone uses `payload` without checking the returned bool
                payload = {};
                return false; // decoding failed
            }
            payload = std::move(r.decoded);
        }

        return true;
    } else {
        payload = {};
    }

    return false;
}

QT_END_NAMESPACE
