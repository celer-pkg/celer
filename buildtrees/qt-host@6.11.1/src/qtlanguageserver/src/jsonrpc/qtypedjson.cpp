// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qtypedjson_p.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonarray.h>
#include <cstring>

QT_BEGIN_NAMESPACE

namespace QTypedJson {

Q_LOGGING_CATEGORY(jsonRpcLog, "qt.jsonrpc");

Reader::Reader(const QJsonValue &v)
    : m_p(new ReaderPrivate { QList({ ValueStack { v, QString(), -1, 0 } }) })
{
}

Reader::~Reader()
{
    for (const QString &msg : m_p->errorMessages)
        qCWarning(jsonRpcLog) << msg;
    delete m_p;
}

QStringList Reader::errorMessages()
{
    return m_p->errorMessages;
}

void Reader::clearErrorMessages()
{
    m_p->errorMessages.clear();
}

void Reader::handleBasic(bool &el)
{
    if (currentValue().isBool())
        el = currentValue().toBool();
    else
        warnMissing(u"bool");
}

void Reader::handleBasic(QByteArray &el)
{
    if (currentValue().isString())
        el = currentValue().toString().toUtf8();
    else
        warnMissing(u"string");
}

void Reader::handleBasic(int &el)
{
    if (currentValue().isDouble())
        el = currentValue().toInt(el);
    else
        warnMissing(u"int");
}

void Reader::handleBasic(double &el)
{
    if (currentValue().isDouble())
        el = currentValue().toDouble();
    else
        warnMissing(u"double");
}

void Reader::handleNullType()
{
    if (!currentValue().isNull() && !currentValue().isUndefined()) {
        warnNonNull();
    }
}

bool Reader::startField(const QString &fieldName)
{
    int oldWarnLevel = (m_p->valuesStack.isEmpty() ? 0 : m_p->valuesStack.last().warnLevel);
    m_p->objectsStack.last().visitedFields.insert(fieldName);
    m_p->valuesStack.append(ValueStack { currentValue()[fieldName], fieldName, -1,
                                         (oldWarnLevel ? oldWarnLevel + 1 : 0) });
    return true;
}

bool Reader::startField(const char *fieldName)
{
    QString f = QString::fromUtf8(fieldName); // conversion needed just to set the fieldPath
    return startField(f);
}

void Reader::endField(const QString &fieldName)
{
    Q_ASSERT(m_p->valuesStack.last().fieldPath == fieldName);
    m_p->valuesStack.removeLast();
}

void Reader::endField(const char *fieldName)
{
    QString f = QString::fromUtf8(fieldName);
    endField(f);
}

bool Reader::startObjectF(const char *type, ObjectOptions options, quintptr)
{
    if (m_p->parseStatus != ParseStatus::Normal)
        return false;
    if (currentValue().isUndefined()) {
        m_p->parseStatus = ParseStatus::Failed;
        return false;
    }
    m_p->objectsStack.append(ObjectStack { type, options, {} });
    return true;
}

void Reader::endObjectF(const char *type, ObjectOptions, quintptr)
{
    Q_ASSERT(std::strcmp(m_p->objectsStack.last().type, type) == 0);
    m_p->objectsStack.removeLast();
}

void Reader::warnExtra(const QJsonObject &e)
{
    if (e.constBegin() != e.constEnd())
        warn(QStringLiteral(u"%1 has extra fields %2")
                     .arg(currentPath(), QString::fromUtf8(QJsonDocument(e).toJson())));
}

void Reader::warnInvalidSize(qint32 size, qint32 expectedSize)
{
    if (size != expectedSize)
        warn(QStringLiteral(u"%1 expected %1 elements, not %2.")
                     .arg(currentPath(), QString::number(expectedSize), QString::number(size)));
}

void Reader::warnMissing(QStringView s)
{
    warn(QStringLiteral(u"%1 misses value of type %2").arg(currentPath(), s));
}

void Reader::warnNonNull()
{
    QByteArray val = QJsonDocument(QJsonArray({ currentValue() })).toJson();
    warn(QStringLiteral(u"%1 is supposed to be null, but is %2")
                 .arg(currentPath(), QString::fromUtf8(val.mid(1, val.size() - 2))));
}

void Reader::warn(const QString &msg)
{
    m_p->errorMessages.append(msg);
    m_p->parseStatus = ParseStatus::Failed;
}

void Reader::handleJson(QJsonValue &v)
{
    v = currentValue();
}

void Reader::handleJson(QJsonObject &v)
{
    if (!currentValue().isObject() && !currentValue().isNull() && !currentValue().isUndefined()) {
        QByteArray val = QJsonDocument(QJsonArray({ currentValue() })).toJson();
        warn(QStringLiteral(u"Error: expected an object at %1, not %2")
                     .arg(currentPath(), QString::fromUtf8(val.mid(1, val.size() - 2))));
    }
    v = currentValue().toObject();
}

void Reader::handleJson(QJsonArray &v)
{
    if (!currentValue().isArray() && !currentValue().isNull() && !currentValue().isUndefined()) {
        QByteArray val = QJsonDocument(QJsonArray({ currentValue() })).toJson();
        warn(QStringLiteral(u"Error: expected an array at %1, not %2")
                     .arg(currentPath(), QString::fromUtf8(val.mid(1, val.size() - 2))));
    }
    v = currentValue().toArray();
}

QJsonObject Reader::getExtraFields() const
{
    QJsonObject extraFields;
    QJsonObject v = currentValue().toObject();
    auto it = v.constBegin();
    auto end = v.constEnd();
    auto &vField = m_p->objectsStack.last().visitedFields;
    while (it != end) {
        if (!vField.contains(it.key())) {
            extraFields.insert(it.key(), it.value());
        }
        ++it;
    }
    return extraFields;
}

void Reader::startArrayF(qint32 &size)
{
    size = int(currentValue().toArray().size());
}

bool Reader::startElement(qint32 index)
{
    int oldWarnLevel = (m_p->valuesStack.isEmpty() ? 0 : m_p->valuesStack.last().warnLevel);
    m_p->valuesStack.append(ValueStack { currentValue().toArray().at(index), QString(), index,
                                         (oldWarnLevel ? oldWarnLevel + 1 : 0) });
    return true;
}

void Reader::endElement(qint32 index)
{
    Q_ASSERT(m_p->valuesStack.last().indexPath == index);
    m_p->valuesStack.removeLast();
}

void Reader::endArrayF(qint32 &) { }

QString Reader::currentPath() const
{
    QStringList res;
    for (const auto &el : std::as_const(m_p->valuesStack)) {
        if (el.indexPath != -1)
            res.append(QString::number(el.indexPath));
        else
            res.append(el.fieldPath);
    }
    return res.join(u".");
}

bool Reader::startTuple(qint32 size)
{
    qint32 expected = qint32(currentValue().toArray().size());
    if (size != expected) {
        warnInvalidSize(size, expected);
        return false;
    };
    return true;
}

void Reader::endTuple(qint32) { }

void JsonBuilder::handleBasic(const bool &v)
{
    m_values.append(QJsonValue(v));
}

void JsonBuilder::handleBasic(const QByteArray &v)
{
    m_values.append(QJsonValue(QString::fromUtf8(v)));
}

void JsonBuilder::handleBasic(const int &v)
{
    m_values.append(QJsonValue(v));
}

void JsonBuilder::handleBasic(const double &v)
{
    m_values.append(QJsonValue(v));
}

void JsonBuilder::handleNullType()
{
    m_values.append(QJsonValue(QJsonValue::Type::Null));
}

void JsonBuilder::handleMissingOptional()
{
    if (m_fieldLevel.isEmpty() || m_fieldLevel.last() != m_values.size())
        handleNullType();
}

bool JsonBuilder::startField(const QString &)
{
    m_fieldLevel.append(m_values.size());
    return true;
}

bool JsonBuilder::startField(const char *)
{
    m_fieldLevel.append(m_values.size());
    return true;
}

void JsonBuilder::endField(const QString &v)
{
    Q_ASSERT(!m_fieldLevel.isEmpty());
    if (m_fieldLevel.last() < m_values.size()) {
        Q_ASSERT(m_values.size() > 1);
        if (QJsonObject *o = std::get_if<QJsonObject>(&m_values[m_values.size() - 2])) {
            o->insert(v, popLastValue());
        } else {
            Q_ASSERT(false);
        }
    }
    Q_ASSERT(!m_fieldLevel.isEmpty() && m_fieldLevel.last() == m_values.size());
    m_fieldLevel.removeLast();
}

void JsonBuilder::endField(const char *v)
{
    endField(QString::fromUtf8(v));
}

bool JsonBuilder::startObjectF(const char *, ObjectOptions, quintptr)
{
    m_values.append(QJsonObject());
    return true;
}

void JsonBuilder::endObjectF(const char *, ObjectOptions, quintptr) { }

bool JsonBuilder::startArrayF(qint32 &)
{
    m_values.append(QJsonArray());
    m_arrayLevel.append(m_values.size());
    return true;
}

bool JsonBuilder::startElement(qint32)
{
    return true;
}

void JsonBuilder::endElement(qint32)
{
    Q_ASSERT(m_values.size() > 1);
    if (QJsonArray *a = std::get_if<QJsonArray>(&m_values[m_values.size() - 2])) {
        a->append(popLastValue());
    } else {
        Q_ASSERT(false);
    }
}

void JsonBuilder::endArrayF(qint32 &)
{
    Q_ASSERT(!m_arrayLevel.isEmpty() && m_arrayLevel.last() == m_values.size());
    m_arrayLevel.removeLast();
}

void JsonBuilder::handleJson(QJsonValue &v)
{
    m_values.append(v);
}

void JsonBuilder::handleJson(QJsonObject &v)
{
    m_values.append(v);
}

void JsonBuilder::handleJson(QJsonArray &v)
{
    m_values.append(v);
}

QJsonValue JsonBuilder::popLastValue()
{
    if (m_values.isEmpty())
        return QJsonValue(QJsonValue::Type::Undefined);
    QJsonValue res = std::visit([](auto &v) { return QJsonValue(v); }, m_values.last());
    m_values.removeLast();
    return res;
}

bool JsonBuilder::startTuple(qint32 size)
{
    return startArrayF(size);
}

void JsonBuilder::endTuple(qint32 size)
{
    endArrayF(size);
}

} // namespace QTypedJson

QT_END_NAMESPACE
