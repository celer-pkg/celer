// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DREPEATER_P_H
#define QQUICK3DREPEATER_P_H

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

#include <QtQuick3D/private/qquick3dnode_p.h>

#include <QtQmlModels/private/qqmldelegatemodel_p.h>

#include <QtCore/qvector.h>
#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE
class QQmlChangeSet;
class QQmlContext;
class QQmlInstanceModel;
class QQuick3DRepeaterPrivate;

class Q_QUICK3D_EXPORT QQuick3DRepeater : public QQuick3DNode
{
    Q_OBJECT

    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QQmlDelegateModel::DelegateModelAccess delegateModelAccess READ delegateModelAccess
            WRITE setDelegateModelAccess NOTIFY delegateModelAccessChanged REVISION(6, 10) FINAL)

    Q_CLASSINFO("DefaultProperty", "delegate")

    QML_NAMED_ELEMENT(Repeater3D)

public:
    QQuick3DRepeater(QQuick3DNode *parent = nullptr);
    ~QQuick3DRepeater() override;

    QVariant model() const;
    void setModel(const QVariant &);

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *);

    int count() const;

    Q_INVOKABLE QQuick3DObject *objectAt(int index) const;

    QQmlDelegateModel::DelegateModelAccess delegateModelAccess() const;
    void setDelegateModelAccess(QQmlDelegateModel::DelegateModelAccess delegateModelAccess);

Q_SIGNALS:
    void modelChanged();
    void delegateChanged();
    void countChanged();

    void objectAdded(int index, QQuick3DObject *object);
    void objectRemoved(int index, QQuick3DObject *object);

    Q_REVISION(6, 10) void delegateModelAccessChanged();

private:
    void clear();
    void regenerate();

protected:
    void componentComplete() override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;
    virtual void initDelegate(int, QQuick3DNode *) {}

private Q_SLOTS:
    void createdObject(int index, QObject *item);
    void initObject(int, QObject *item);
    void modelUpdated(const QQmlChangeSet &changeSet, bool reset);

private:
    Q_DISABLE_COPY(QQuick3DRepeater)

    void requestItems();
    void applyDelegateChange();
    QQmlDelegateModel *createDelegateModel();

    void connectModel(QQmlDelegateModelPointer *model);
    void disconnectModel(QQmlDelegateModelPointer *model);

    QPointer<QQmlInstanceModel> m_model;
    int m_itemCount;
    bool m_ownModel : 1;
    bool m_delegateValidated : 1;
    bool m_explicitDelegate: 1;
    bool m_explicitDelegateModelAccess : 1;

    QVector<QPointer<QQuick3DNode> > m_deletables;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QQuick3DRepeater)

#endif // QQUICK3DREPEATER_P_H
