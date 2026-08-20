// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QSBINSPECTORHELPER_H
#define QSBINSPECTORHELPER_H

#include <QObject>
#include <QVariantList>
#include <QtGui/private/qshader_p.h>

// Model for common qsb information
struct QsbShaderData {
    Q_GADGET
    Q_PROPERTY(QString currentFile MEMBER m_currentFile)
    Q_PROPERTY(QString stage MEMBER m_stage)
    Q_PROPERTY(QString reflectionInfo MEMBER m_reflectionInfo)
    Q_PROPERTY(int qsbVersion MEMBER m_qsbVersion)
    Q_PROPERTY(int shaderCount MEMBER m_shaderCount)
    Q_PROPERTY(int size MEMBER m_size)
public:
    QString m_currentFile;
    QString m_stage;
    QString m_reflectionInfo;
    int m_qsbVersion = 0;
    int m_shaderCount = 0;
    int m_size = 0;
};

// Model for shaders and their index in the qsb
struct ShaderSelectorData {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(int sourceIndex MEMBER m_sourceIndex)
public:
    QString m_name;
    int m_sourceIndex;
};

class QsbInspectorHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QsbShaderData shaderData READ shaderData NOTIFY shaderDataChanged)
    Q_PROPERTY(QVariantList sourceSelectorModel READ sourceSelectorModel NOTIFY sourceSelectorModelChanged)
    Q_PROPERTY(int currentSourceIndex READ currentSourceIndex WRITE setCurrentSourceIndex NOTIFY currentSourceIndexChanged)
    Q_PROPERTY(QString currentSourceCode READ currentSourceCode NOTIFY currentSourceCodeChanged)

public:
    explicit QsbInspectorHelper(QObject *parent = nullptr);

    QsbShaderData shaderData() const;
    QVariantList sourceSelectorModel() const;
    int currentSourceIndex() const;
    QString currentSourceCode() const;

public Q_SLOTS:
    bool loadQsb(const QString &filename);
    void setCurrentSourceIndex(int index);

Q_SIGNALS:
    void shaderDataChanged();
    void sourceSelectorModelChanged();
    void currentSourceIndexChanged();
    void currentSourceCodeChanged();

private:
    QsbShaderData m_shaderData;
    QVariantList m_sourceSelectorModel;
    int m_currentSourceIndex = -1;
    QStringList m_sourceCodes;
};

Q_DECLARE_METATYPE(QsbShaderData);

#endif // QSBINSPECTORHELPER_H
