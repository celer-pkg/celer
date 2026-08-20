// Copyright (C) 2015 Jolla Ltd.
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOMAPITEMVIEW_H
#define QDECLARATIVEGEOMAPITEMVIEW_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <map>
#include <QtCore/QModelIndex>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlIncubator>
#include <QtQml/qqml.h>
#include <private/qqmldelegatemodel_p.h>
#include <QtQuick/private/qquicktransition_p.h>
#include <QtLocation/private/qdeclarativegeomapitemgroup_p.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QQmlComponent;
class QQuickItem;
class QDeclarativeGeoMap;
class QDeclarativeGeoMapItemBase;
class QQmlOpenMetaObject;
class QQmlOpenMetaObjectType;
class MapItemViewDelegateIncubator;
class QDeclarativeGeoMapItemViewItemData;
class QDeclarativeGeoMapItemView;
class QDeclarativeGeoMapItemGroup;

class Q_LOCATION_EXPORT QDeclarativeGeoMapItemView : public QDeclarativeGeoMapItemGroup
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MapItemView)
    QML_ADDED_IN_VERSION(5, 0)
    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_PROPERTY(bool autoFitViewport READ autoFitViewport WRITE setAutoFitViewport NOTIFY autoFitViewportChanged)
    Q_PROPERTY(QQuickTransition *add MEMBER m_enter REVISION(5, 12))
    Q_PROPERTY(QQuickTransition *remove MEMBER m_exit REVISION(5, 12))
    Q_PROPERTY(QList<QQuickItem *> mapItems READ mapItems REVISION(5, 12))
    Q_PROPERTY(bool incubateDelegates READ incubateDelegates WRITE setIncubateDelegates NOTIFY incubateDelegatesChanged REVISION(5, 12))
    Q_PROPERTY(QQmlDelegateModel::DelegateModelAccess delegateModelAccess READ delegateModelAccess
            WRITE setDelegateModelAccess NOTIFY delegateModelAccessChanged REVISION(6, 10) FINAL)


public:
    explicit QDeclarativeGeoMapItemView(QQuickItem *parent = nullptr);
    ~QDeclarativeGeoMapItemView();

    QVariant model() const;
    void setModel(const QVariant &);

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *);

    bool autoFitViewport() const;
    void setAutoFitViewport(const bool &fit);

    void setMap(QDeclarativeGeoMap *);
    void removeInstantiatedItems(bool transition = true);
    void instantiateAllItems();

    void setIncubateDelegates(bool useIncubators);
    bool incubateDelegates() const;

    QQmlDelegateModel::DelegateModelAccess delegateModelAccess() const;
    void setDelegateModelAccess(QQmlDelegateModel::DelegateModelAccess delegateModelAccess);

    QList<QQuickItem *> mapItems();

    // From QQmlParserStatus
    void componentComplete() override;
    void classBegin() override;

Q_SIGNALS:
    void modelChanged();
    void delegateChanged();
    void autoFitViewportChanged();
    void incubateDelegatesChanged();
    Q_REVISION(6, 10) void delegateModelAccessChanged();

private Q_SLOTS:
    void destroyingItem(QObject *object);
    void initItem(int index, QObject *object);
    void createdItem(int index, QObject *object);
    void modelUpdated(const QQmlChangeSet &changeSet, bool reset);
    void exitTransitionFinished();

private:
    void fitViewport();
    void removeDelegateFromMap(int index, bool transition = true);
    void removeDelegateFromMap(QQuickItem *o);
    void transitionItemOut(QQuickItem *o);
    void terminateExitTransition(QQuickItem *o);
    QQmlInstanceModel::ReleaseFlags disposeDelegate(QQuickItem *item);

    void insertInstantiatedItem(int index, QQuickItem *o, bool createdItem);
    void addItemToMap(QDeclarativeGeoMapItemBase *item, int index, bool createdItem);
    void addItemViewToMap(QDeclarativeGeoMapItemView *item, int index, bool createdItem);
    void addItemGroupToMap(QDeclarativeGeoMapItemGroup *item, int index, bool createdItem);
    void addDelegateToMap(QQuickItem *object, int index, bool createdItem = false);

    QQmlIncubator::IncubationMode m_incubationMode = QQmlIncubator::Asynchronous;
    QQmlComponent *m_delegate = nullptr;
    QVariant m_itemModel;
    QDeclarativeGeoMap *m_map = nullptr;
    QList<QQuickItem *> m_instantiatedItems;
    QQmlDelegateModel *m_delegateModel = nullptr;
    QQuickTransition *m_enter = nullptr;
    QQuickTransition *m_exit = nullptr;

    QQmlDelegateModel::DelegateModelAccess m_delegateModelAccess
            = QQmlDelegateModel::Qt5ReadWrite;

    bool m_componentCompleted = false;
    bool m_fitViewport = false;
    bool m_creatingObject = false;

    friend class QDeclarativeGeoMap;
    friend class QDeclarativeGeoMapItemBase;
    friend class QDeclarativeGeoMapItemTransitionManager;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeGeoMapItemView)

#endif
