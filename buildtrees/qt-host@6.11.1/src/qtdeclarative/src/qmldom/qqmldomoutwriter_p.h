// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QMLDOMOUTWRITER_P_H
#define QMLDOMOUTWRITER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qqmldom_global.h"
#include "qqmldom_fwd_p.h"
#include "qqmldomlinewriter_p.h"
#include "qqmldomcomments_p.h"
#include "qqmldomexternalitems_p.h"

#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

namespace QQmlJS {
namespace Dom {

class QMLDOM_EXPORT OutWriter
{
public:
    int indent = 0;
    bool indentNextlines = false;
    // TODO QTBUG-139623
    bool skipComments = false;
    LineWriter &lineWriter;
    QString writtenStr;
    using RegionToCommentMap = QMap<FileLocationRegion, CommentedElement>;
    QStack<RegionToCommentMap> pendingComments;
    QStringView code;
    bool isFormatterEnabled = true;

    using OffsetToDisabledRegionMap = QMap<quint32, SourceLocation>;
    OffsetToDisabledRegionMap formatDisabledRegions;

    explicit OutWriter(LineWriter &lw) : lineWriter(lw)
    {
        lineWriter.addInnerSink([this](QStringView s) { writtenStr.append(s); });
        lineWriter.addTextAddCallback([this](LineWriter &, LineWriter::TextAddType tt) {
            if (indentNextlines && tt == LineWriter::TextAddType::Normal
                && QStringView(lineWriter.currentLine()).trimmed().isEmpty())
                lineWriter.setLineIndent(indent);
            return true;
        });
    }

    OutWriter(std::shared_ptr<ExternalOwningItem> ownerFile, LineWriter &lw)
        : OutWriter(lw)
    {
        if (!ownerFile)
            return;

        code = ownerFile->code();

        const auto engine = [&ownerFile]() -> std::shared_ptr<QQmlJS::Engine> {
            if (auto qmlfilePtr = std::dynamic_pointer_cast<QmlFile>(ownerFile))
                return qmlfilePtr->engine();
            else if (auto jsfilePtr = std::dynamic_pointer_cast<JsFile>(ownerFile))
                return jsfilePtr->engine();
            return nullptr;
        }();

        if (engine)
            scanFormatDirectives(code, engine->comments());
    }

    int increaseIndent(int level = 1)
    {
        int oldIndent = indent;
        indent += lineWriter.options().formatOptions.indentSize * level;
        return oldIndent;
    }
    int decreaseIndent(int level = 1, int expectedIndent = -1)
    {
        indent -= lineWriter.options().formatOptions.indentSize * level;
        Q_ASSERT(expectedIndent < 0 || expectedIndent == indent);
        return indent;
    }

    void itemStart(const DomItem &it);
    void itemEnd();
    void writePreComment(FileLocationRegion region);
    void writePostComment(FileLocationRegion regino);

    quint32 counter() const { return lineWriter.counter(); }
    OutWriter &writeRegion(const FileLocations::Tree &fLoc, FileLocationRegion region, QStringView toWrite);
    OutWriter &writeRegion(const FileLocations::Tree &fLoc, FileLocationRegion region);
    OutWriter &ensureNewline(int nNewlines = 1)
    {
        if (formatterEnabled())
            lineWriter.ensureNewline(nNewlines);
        return *this;
    }

    OutWriter &ensureSpace()
    {
        if (formatterEnabled())
            lineWriter.ensureSpace();
        return *this;
    }

    OutWriter &ensureSpace(QStringView space)
    {
        if (formatterEnabled())
            lineWriter.ensureSpace(space);
        return *this;
    }

    OutWriter &newline()
    {
        if (formatterEnabled())
            lineWriter.newline();
        return *this;
    }

    OutWriter &write(QStringView v, LineWriter::TextAddType t = LineWriter::TextAddType::Normal)
    {
        if (formatterEnabled())
            lineWriter.write(v, t);
        return *this;
    }
    void flush()
    {
        if (formatterEnabled())
            lineWriter.flush();
    }

    void eof(bool ensureNewline = true)
    {
        if (formatterEnabled())
            lineWriter.eof(ensureNewline);
    }

    int addNewlinesAutospacerCallback(int nLines)
    {
        return lineWriter.addNewlinesAutospacerCallback(nLines);
    }
    int addTextAddCallback(std::function<bool(LineWriter &, LineWriter::TextAddType)> callback)
    {
        return lineWriter.addTextAddCallback(callback);
    }
    bool removeTextAddCallback(int i) { return lineWriter.removeTextAddCallback(i); }

    void maybeWriteComment(const Comment &comment);
    bool formatterEnabled() const;
    bool shouldFormat(const FileLocations::Tree &fLoc, FileLocationRegion region);
    void maybeWriteDisabledRegion(const SourceLocation &loc);
    void scanFormatDirectives(QStringView code, const QList<SourceLocation> &comments);
    void writeDisabledRegion(const SourceLocation &loc);
    QStringView attachedDisableCode(quint32 offset) const;
};

} // end namespace Dom
} // end namespace QQmlJS

QT_END_NAMESPACE
#endif // QMLDOMOUTWRITER_P_H
