// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include "qaxbindable.h"
#include "qaxfactory.h"

#include <QtAxBase/private/qaxutils_p.h>

#include <qapplication.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmetaobject.h>
#include <qscopedpointer.h>
#include <qfileinfo.h>
#include <qsettings.h>
#include <qvariant.h>
#include <qtextstream.h>
#include <qloggingcategory.h>
#include <qdebug.h>
#include <QScopeGuard>

#include <qt_windows.h>
#include <olectl.h>

QT_BEGIN_NAMESPACE

#define Q_REQUIRED_RPCNDR_H_VERSION 475

// Some global variables to store module information
bool qAxIsServer = false;
ITypeLib *qAxTypeLibrary = nullptr;
wchar_t qAxModuleFilename[MAX_PATH];
bool qAxOutProcServer = false;

// The QAxFactory instance
static QAxFactory* qax_factory = nullptr;
extern CLSID CLSID_QRect;
extern CLSID CLSID_QSize;
extern CLSID CLSID_QPoint;
extern void qax_shutDown();
extern bool qax_ownQApp;


extern QAxFactory *qax_instantiate();

QAxFactory *qAxFactory()
{
    if (!qax_factory) {
        bool hadQApp = qApp != nullptr;
        qax_factory = qax_instantiate();
        // QAxFactory created a QApplication
        if (!hadQApp && qApp)
            qax_ownQApp = true;

        // register all types with metatype system as pointers
        QStringList keys(qax_factory->featureList());
        for (int i = 0; i < keys.size(); ++i) {
            QByteArray pointerType = keys.at(i).toLatin1() + '*';
            if (QMetaType::fromName(pointerType).id() == QMetaType::UnknownType)
                qRegisterMetaType<void *>(pointerType);
        }
    }
    return qax_factory;
}

// Some local variables to handle module lifetime
static unsigned long qAxModuleRef = 0;
static CRITICAL_SECTION qAxModuleSection;


/////////////////////////////////////////////////////////////////////////////
// Server control
/////////////////////////////////////////////////////////////////////////////

static int initCount = 0;

Q_STATIC_LOGGING_CATEGORY(lcAxRegistration, "qt.activeqt.registration")

QString qAxInit()
{
    static QString libFile;

    if (initCount++)
        return libFile;

    InitializeCriticalSection(&qAxModuleSection);

    libFile = QString::fromWCharArray(qAxModuleFilename);
    libFile = libFile.toLower();
    if (LoadTypeLibEx(reinterpret_cast<const wchar_t *>(libFile.utf16()),
                      REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    const int lastDot = libFile.lastIndexOf(QLatin1Char('.'));
    libFile.truncate(lastDot);
    libFile += QLatin1String(".tlb");
    if (LoadTypeLibEx(reinterpret_cast<const wchar_t *>(libFile.utf16()),
                      REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    libFile.truncate(lastDot);
    libFile += QLatin1String(".olb");
    if (LoadTypeLibEx(reinterpret_cast<const wchar_t *>(libFile.utf16()),
                      REGKIND_NONE, &qAxTypeLibrary) == S_OK)
        return libFile;

    return QString();
}

void qAxCleanup()
{
    if (!initCount)
        qWarning("qAxInit/qAxCleanup mismatch");

    if (--initCount)
        return;

    delete qax_factory;
    qax_factory = nullptr;

    if (qAxTypeLibrary) {
        qAxTypeLibrary->Release();
        qAxTypeLibrary = nullptr;
    }

    DeleteCriticalSection(&qAxModuleSection);
}

unsigned long qAxLock()
{
    EnterCriticalSection(&qAxModuleSection);
    unsigned long ref = ++qAxModuleRef;
    LeaveCriticalSection(&qAxModuleSection);
    return ref;
}

unsigned long qAxUnlock()
{
    if (!initCount) // cleaned up already
        return 0;

    EnterCriticalSection(&qAxModuleSection);
    unsigned long ref = --qAxModuleRef;
    LeaveCriticalSection(&qAxModuleSection);

    if (!ref)
        qax_shutDown();
    return ref;
}

unsigned long qAxLockCount()
{
    return qAxModuleRef;
}

/////////////////////////////////////////////////////////////////////////////
// Registry
/////////////////////////////////////////////////////////////////////////////

extern bool qax_disable_inplaceframe;

QString qax_clean_type(const QString &type, const QMetaObject *mo)
{
    if (mo) {
        int classInfoIdx = mo->indexOfClassInfo("CoClassAlias");
        if (classInfoIdx != -1) {
            const QMetaClassInfo classInfo = mo->classInfo(classInfoIdx);
            return QLatin1String(classInfo.value());
        }
    }

    QString alias(type);
    alias.remove(QLatin1String("::"));
    return alias;
}

static void UpdateRegistryKeys(bool bRegister, const QString keyPath, QScopedPointer<QSettings> & settings)
{
    const QChar dot(QLatin1Char('.'));
    const QChar slash(QLatin1Char('/'));
    QString file = QString::fromWCharArray(qAxModuleFilename);
    const QString module = QFileInfo(file).baseName();

    const QString appId = qAxFactory()->appID().toString().toUpper();
    const QString libId = qAxFactory()->typeLibID().toString().toUpper();

    if (bRegister) {
        settings->setValue(QLatin1String("/AppID/") + appId + QLatin1String("/."), module);
        if (qAxOutProcServer)
            settings->setValue(QLatin1String("/AppID/") + module + QLatin1String(".EXE/AppID"), appId);
        else
            settings->setValue(QLatin1String("/AppID/") + appId + QLatin1String("/DllSurrogate"), QString());

        const QStringList keys = qAxFactory()->featureList();
        for (const QString &classNameIn : keys) {
            QObject *object = qAxFactory()->createObject(classNameIn);
            const QMetaObject *mo = qAxFactory()->metaObject(classNameIn);
            const QString classId = qAxFactory()->classID(classNameIn).toString().toUpper();

            const QString className = qax_clean_type(classNameIn, mo);

            if (object) { // don't register subobject classes
                QString classVersion = mo ? QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString();
                if (classVersion.isNull())
                    classVersion = QLatin1String("1.0");
                bool insertable = mo && !qstricmp(mo->classInfo(mo->indexOfClassInfo("Insertable")).value(), "yes");
                bool control = object->isWidgetType();
                QString classMajorVersion = classVersion.left(classVersion.indexOf(dot));
                uint olemisc = OLEMISC_SETCLIENTSITEFIRST
                    |OLEMISC_ACTIVATEWHENVISIBLE
                    |OLEMISC_INSIDEOUT
                    |OLEMISC_CANTLINKINSIDE
                    |OLEMISC_RECOMPOSEONRESIZE;
                if (!control)
                    olemisc |= OLEMISC_INVISIBLEATRUNTIME;
                else if (object->findChild<QMenuBar*>() && !qax_disable_inplaceframe)
                    olemisc |= OLEMISC_WANTSTOMENUMERGE;

                const QString versionLessProgId = module + dot + className;
                const QString progId = versionLessProgId + dot + classMajorVersion;
                QString key = slash + progId;
                settings->setValue(key + QLatin1String("/."),
                                   QString(className + QLatin1String(" Class")));
                settings->setValue(key + QLatin1String("/CLSID/."), classId);
                if (insertable)
                    settings->setValue(key + QLatin1String("/Insertable/."), QVariant(QLatin1String("")));

                key = slash + module + dot + className;
                settings->setValue(key + QLatin1String("/."),
                                   QString(className + QLatin1String(" Class")));
                settings->setValue(key + QLatin1String("/CLSID/."), classId);
                settings->setValue(key + QLatin1String("/CurVer/."), progId);

                key = QLatin1String("/CLSID/") + classId;
                settings->setValue(key + QLatin1String("/."),
                                   QString(className + QLatin1String(" Class")));
                settings->setValue(key + QLatin1String("/AppID"), appId);
                if (control)
                    settings->setValue(key + QLatin1String("/Control/."), QVariant(QLatin1String("")));
                if (insertable)
                    settings->setValue(key + QLatin1String("/Insertable/."), QVariant(QLatin1String("")));
                if (file.endsWith(QLatin1String("dll"), Qt::CaseInsensitive))
                    settings->setValue(key + QLatin1String("/InProcServer32/."), file);
                else
                    settings->setValue(key + QLatin1String("/LocalServer32/."),
                                       QString(QLatin1Char('\"') + file + QLatin1String("\" -activex")));
                settings->setValue(key + QLatin1String("/MiscStatus/."), control ? QLatin1String("1") : QLatin1String("0"));
                settings->setValue(key + QLatin1String("/MiscStatus/1/."), QString::number(olemisc));
                settings->setValue(key + QLatin1String("/Programmable/."), QVariant(QLatin1String("")));
                settings->setValue(key + QLatin1String("/ToolboxBitmap32/."),
                                   QString(QLatin1Char('\"') + file + QLatin1String("\", 101")));
                settings->setValue(key + QLatin1String("/TypeLib/."), libId);
                settings->setValue(key + QLatin1String("/Version/."), classVersion);
                settings->setValue(key + QLatin1String("/VersionIndependentProgID/."), versionLessProgId);
                settings->setValue(key + QLatin1String("/ProgID/."), progId);

                QString mime = QLatin1String(mo->classInfo(mo->indexOfClassInfo("MIME")).value());
                if (!mime.isEmpty()) {
                    QStringList mimeTypes = mime.split(QLatin1Char(';'));
                    for (qsizetype m = 0; m < mimeTypes.size(); ++m) {
                        mime = mimeTypes.at(m);
                        if (mime.isEmpty())
                            continue;
                        QString extension;
                        while (mime.contains(QLatin1Char(':'))) {
                            extension = mime.mid(mime.lastIndexOf(QLatin1Char(':')) + 1);
                            mime.chop(extension.length() + 1);
                            // Prepend '.' before extension, if required.
                            extension = extension.trimmed();
                            if (!extension.startsWith(dot))
                                extension.prepend(dot);
                        }

                        if (!extension.isEmpty()) {
                            key = slash + extension;
                            settings->setValue(key + QLatin1String("/."),
                                               QString(module + dot + className));
                            settings->setValue(key + QLatin1String("/Content Type"), mime);

                            mime.replace(slash, QLatin1Char('\\'));
                            key = QLatin1String("/MIME/Database/Content Type/") + mime;
                            settings->setValue(key + QLatin1String("/CLSID"), classId);
                            settings->setValue(key + QLatin1String("/Extension"), extension);
                        }
                    }
                }

                delete object;
                qCDebug(lcAxRegistration).nospace().noquote() << "Registered \"" << progId
                    << "\"/" << classId << ", \"" << file << "\" at \"" << keyPath
                    << "\", insertable=" << insertable << ", control=" << control
                    << ", olemisc=" << Qt::hex << Qt::showbase << olemisc
                    << ", mime=" << mime;
            }

            qAxFactory()->registerClass(classNameIn, settings.data());
        }
    } else {
        settings->remove(QLatin1String("/AppID/") + appId + QLatin1String("/."));
        if (qAxOutProcServer)
            settings->remove(QLatin1String("/AppID/") + module + QLatin1String(".EXE"));
        else
            settings->remove(QLatin1String("/AppID/") + appId + QLatin1String("/DllSurrogate"));

        const QStringList keys = qAxFactory()->featureList();
        for (const QString &classNameIn : keys) {
            const QMetaObject *mo = qAxFactory()->metaObject(classNameIn);
            const QString classId = qAxFactory()->classID(classNameIn).toString().toUpper();
            const QString className = qax_clean_type(classNameIn, mo);

            QString classVersion = mo ? QString::fromLatin1(mo->classInfo(mo->indexOfClassInfo("Version")).value()) : QString();
            if (classVersion.isNull())
                classVersion = QLatin1String("1.0");
            const QString classMajorVersion = classVersion.left(classVersion.indexOf(dot));

            qAxFactory()->unregisterClass(classNameIn, settings.data());
            const QString progId = module + dot + className + dot + classMajorVersion;
            QString key = slash + progId;
            settings->remove(key + QLatin1String("/CLSID/."));
            settings->remove(key + QLatin1String("/Insertable/."));
            settings->remove(key + QLatin1String("/."));
            settings->remove(key);

            key = slash + module + dot + className;
            settings->remove(key + QLatin1String("/CLSID/."));
            settings->remove(key + QLatin1String("/CurVer/."));
            settings->remove(key + QLatin1String("/."));
            settings->remove(key);

            key = QLatin1String("/CLSID/") + classId;
            settings->remove(key + QLatin1String("/AppID"));
            settings->remove(key + QLatin1String("/Control/."));
            settings->remove(key + QLatin1String("/Insertable/."));
            settings->remove(key + QLatin1String("/InProcServer32/."));
            settings->remove(key + QLatin1String("/LocalServer32/."));
            settings->remove(key + QLatin1String("/MiscStatus/1/."));
            settings->remove(key + QLatin1String("/MiscStatus/."));
            settings->remove(key + QLatin1String("/Programmable/."));
            settings->remove(key + QLatin1String("/ToolboxBitmap32/."));
            settings->remove(key + QLatin1String("/TypeLib/."));
            settings->remove(key + QLatin1String("/Version/."));
            settings->remove(key + QLatin1String("/VersionIndependentProgID/."));
            settings->remove(key + QLatin1String("/ProgID/."));
            settings->remove(key + QLatin1String("/."));
            settings->remove(key);

            QString mime = QLatin1String(mo->classInfo(mo->indexOfClassInfo("MIME")).value());
            if (!mime.isEmpty()) {
                QStringList mimeTypes = mime.split(QLatin1Char(';'));
                for (qsizetype m = 0; m < mimeTypes.size(); ++m) {
                    mime = mimeTypes.at(m);
                    if (mime.isEmpty())
                        continue;
                    QString extension;
                    while (mime.contains(QLatin1Char(':'))) {
                        extension = mime.mid(mime.lastIndexOf(QLatin1Char(':')) + 1);
                        mime.chop(extension.length() + 1);
                        // Prepend '.' before extension, if required.
                        extension = extension.trimmed();
                        if (extension[0] != dot)
                            extension.prepend(dot);
                    }
                    if (!extension.isEmpty()) {
                        key = slash + extension;
                        settings->remove(key + QLatin1String("/Content Type"));
                        settings->remove(key + QLatin1String("/."));
                        settings->remove(key);
                        mime.replace(slash, QLatin1Char('\\'));
                        key = QLatin1String("/MIME/Database/Content Type/") + mime;
                        settings->remove(key + QLatin1String("/Extension"));
                        settings->remove(key + QLatin1String("/CLSID"));
                        settings->remove(key + QLatin1String("/."));
                        settings->remove(key);
                    }
                }
            }
            qCDebug(lcAxRegistration).nospace().noquote() << "Unregistered \""
                << progId << "\"/" << classId << ", \"" << file << "\" from \""
                << keyPath << '"';
        }
    }
}

// (Un)Register the ActiveX server in the registry.
// The QAxFactory implementation provides the information.
HRESULT UpdateRegistry(bool bRegister, bool perUser)
{
    qAxIsServer = false;
    QString file = QString::fromWCharArray(qAxModuleFilename);
    const QString module = QFileInfo(file).baseName();

    QString libFile = qAxInit();
    auto libFile_cleanup = qScopeGuard([] { qAxCleanup(); });

    TLIBATTR *libAttr = nullptr;
    if (qAxTypeLibrary)
        qAxTypeLibrary->GetLibAttr(&libAttr);
    if (!libAttr)
        return SELFREG_E_TYPELIB;
    auto libAttr_cleanup = qScopeGuard([libAttr] { qAxTypeLibrary->ReleaseTLibAttr(libAttr); });

    if (bRegister) {
        if (!perUser) {
            HRESULT hr = RegisterTypeLib(qAxTypeLibrary, qaxQString2MutableOleChars(libFile), nullptr);
            if (FAILED(hr)) {
                qWarning("Failing to register %s due to insufficient permission.", qPrintable(module));
                return hr;
            }
        } else {
#ifndef Q_CC_MINGW
            // MinGW does not have RegisterTypeLibForUser() implemented so we cannot fallback in this case
            RegisterTypeLibForUser(qAxTypeLibrary, qaxQString2MutableOleChars(libFile), nullptr);
#endif
        }
    } else {
        if (!perUser) {
            HRESULT hr = UnRegisterTypeLib(libAttr->guid, libAttr->wMajorVerNum, libAttr->wMinorVerNum, libAttr->lcid, libAttr->syskind);
            if (FAILED(hr)) {
                qWarning("Failing to register %s due to insufficient permission.", qPrintable(module));
                return hr;
            }
        } else {
#ifndef Q_CC_MINGW
            // MinGW does not have RegisterTypeLibForUser() implemented so we cannot fallback in this case
            UnRegisterTypeLibForUser(libAttr->guid, libAttr->wMajorVerNum, libAttr->wMinorVerNum, libAttr->lcid, libAttr->syskind);
#endif
        }
    }

    QString keyPath(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"));
    if (perUser)
        keyPath = QLatin1String("HKEY_CURRENT_USER\\Software\\Classes");
    QScopedPointer<QSettings> settings(new QSettings(keyPath, QSettings::NativeFormat));

    // we try to create the ActiveX widgets later on...
    bool delete_qApp = false;
    if (!qApp) {
        (void)new QApplication(__argc, __argv);
        delete_qApp = true;
    }

    UpdateRegistryKeys(bRegister, keyPath, settings);

    if (delete_qApp)
        delete qApp;

    if (settings->status() == QSettings::NoError)
        return S_OK;
    qWarning() << module << ": Error writing to " << keyPath;
    return SELFREG_E_CLASS;
}

/////////////////////////////////////////////////////////////////////////////
// IDL generator
/////////////////////////////////////////////////////////////////////////////

static QByteArrayList enums;
static QByteArrayList enumValues;
static QByteArrayList subtypes;

static const char* const type_map[][2] =
{
    // QVariant/Qt Value data types
    { "QString",        "BSTR" },
    { "QCString",       "BSTR" },
    { "bool",           "VARIANT_BOOL" },
    { "char",           "char" },
    { "unsigned char",  "unsigned char" },
    { "uchar",          "unsigned char" },
    { "short",          "short" },
    { "unsigned short", "unsigned short" },
    { "ushort",         "unsigned short" },
    { "int",            "int" },
    { "void",           "void" },
    { "uint",           "unsigned int" },
    { "float",          "float" },
    { "double",         "double" },
    { "QColor",         "OLE_COLOR" },
    { "QDate",          "DATE" },
    { "QTime",          "DATE" },
    { "QDateTime",      "DATE" },
    { "QFont",          "IFontDisp*" },
    { "QPixmap",        "IPictureDisp*" },
    { "QVariant",       "VARIANT" },
    { "QVariantList",    "SAFEARRAY(VARIANT)" },
    { "QList<QVariant>", "SAFEARRAY(VARIANT)" },
    { "quint64",        "unsigned hyper" },
    { "qint64",         "hyper" },
    { "qulonglong",     "unsigned hyper" },
    { "qlonglong",      "hyper" },
    { "QByteArray",     "SAFEARRAY(BYTE)" },
    { "QStringList",    "SAFEARRAY(BSTR)" },
    // Userdefined Qt datatypes - some not on Borland though
    { "QCursor",         "enum MousePointer" },
    { "Qt::FocusPolicy", "enum FocusPolicy" },
#if __REQUIRED_RPCNDR_H_VERSION__ >= Q_REQUIRED_RPCNDR_H_VERSION
    { "QRect",          "struct QRect" },
    { "QSize",          "struct QSize" },
    { "QPoint",         "struct QPoint" },
#endif // __REQUIRED_RPCNDR_H_VERSION__ >= Q_REQUIRED_RPCNDR_H_VERSION
    // And we support COM data types
    { "BOOL",           "BOOL" },
    { "BSTR",           "BSTR" },
    { "OLE_COLOR",      "OLE_COLOR" },
    { "DATE",           "DATE" },
    { "VARIANT",        "VARIANT" },
    { "IDispatch",      "IDispatch*" },
    { "IUnknown",       "IUnknown*" },
    { "IDispatch*",     "IDispatch*" },
    { "IUnknown*",      "IUnknown*" },
    { nullptr,                nullptr }
};

static QByteArray convertTypes(const QByteArray &qtype, bool *ok)
{
    qRegisterMetaType<IDispatch *>("IDispatch*");
    qRegisterMetaType<IUnknown *>("IUnknown*");

    *ok = false;

    int i = 0;
    while (type_map[i][0]) {
        if (qtype == type_map[i][0] && type_map[i][1]) {
            *ok = true;
            return type_map[i][1];
        }
        ++i;
    }
    if (enums.contains(qtype)) {
        *ok = true;
        return "enum " + qtype;
    }
    if (subtypes.contains(qtype)) {
        *ok = true;
    } else if (qtype.endsWith('*')) {
        QByteArray cleanType = qtype.left(qtype.length() - 1);
        const QMetaObject *mo = qAxFactory()->metaObject(QString::fromLatin1(cleanType.constData()));
        if (mo) {
            cleanType = qax_clean_type(QString::fromLatin1(cleanType), mo).toLatin1();
            if (subtypes.contains(cleanType)) {
                *ok = true;
                return cleanType + '*';
            }
        }
    }
    return qtype;
}

static const char* const keyword_map[][2] =
{
    { "aggregatable",   "aggregating"       },
    { "allocate",       "alloc"             },
    { "appobject",      "appObject"         },
    { "arrays",         "array"             },
    { "async",          "asynchronous"      },
    { "bindable",       "binding"           },
    { "Boolean",        "boolval"           },
    { "boolean",        "boolval"           },
    { "broadcast",      "broadCast"         },
    { "callback",       "callBack"          },
    { "decode",         "deCode"            },
    { "default",        "defaulted"         },
    { "defaultbind",    "defaultBind"       },
    { "defaultvalue",   "defaultValue"      },
    { "encode"          "enCode"            },
    { "endpoint",       "endPoint"          },
    { "hidden",         "isHidden"          },
    { "ignore",         "ignore_"           },
    { "local",          "local_"            },
    { "notify",         "notify_"           },
    { "object",         "object_"           },
    { "optimize",       "optimize_"         },
    { "optional",       "optional_"         },
    { "out",            "out_"              },
    { "pipe",           "pipe_"             },
    { "proxy",          "proxy_"            },
    { "ptr",            "pointer"           },
    { "readonly",       "readOnly"          },
    { "small",          "small_"            },
    { "source",         "source_"           },
    { "string",         "string_"           },
    { "uuid",           "uuid_"             },
    { nullptr,                nullptr                   }
};

static QByteArray replaceKeyword(const QByteArray &name)
{
    int i = 0;
    while (keyword_map[i][0]) {
        if (name == keyword_map[i][0] && keyword_map[i][1])
            return keyword_map[i][1];
        ++i;
    }
    return name;
}

static QMap<QByteArray, int> mapping;

static QByteArray renameOverloads(const QByteArray &name)
{
    QByteArray newName = name;

    if (mapping.contains(name)) {
        int n = mapping.value(name);
        newName = name + '_' + QByteArray::number(n);
        mapping.insert(name, n+1);
    } else {
        mapping.insert(name, 1);
    }

    return newName;
}

// filter out some properties
static const char* const ignore_props[] =
{
    "name",
    "objectName",
    "isTopLevel",
    "isDialog",
    "isModal",
    "isPopup",
    "isDesktop",
    "geometry",
    "pos",
    "frameSize",
    "frameGeometry",
    "size",
    "sizeHint",
    "minimumSizeHint",
    "microFocusHint",
    "rect",
    "childrenRect",
    "childrenRegion",
    "minimumSize",
    "maximumSize",
    "sizeIncrement",
    "baseSize",
    "ownPalette",
    "ownFont",
    "ownCursor",
    "visibleRect",
    "isActiveWindow",
    "underMouse",
    "visible",
    "hidden",
    "minimized",
    "focus",
    "focusEnabled",
    "customWhatsThis",
    "shown",
    "windowOpacity",
    nullptr
};

// filter out some slots
static const char* const ignore_slots[] =
{
    "deleteLater",
    "setMouseTracking",
    "update",
    "repaint",
    "iconify",
    "showMinimized",
    "showMaximized",
    "showFullScreen",
    "showNormal",
    "polish",
    "constPolish",
    "stackUnder",
    "setShown",
    "setHidden",
    "move_1",
    "resize_1",
    "setGeometry_1",
    nullptr
};

static bool ignore(const char *test, const char *const *table)
{
    if (!test)
        return true;
    int i = 0;
    while (table[i]) {
        if (!strcmp(test, table[i]))
            return true;
        ++i;
    }
    return false;
}

bool ignoreSlots(const char *test)
{
    return ignore(test, ignore_slots);
}

bool ignoreProps(const char *test)
{
    return ignore(test, ignore_props);
}

static QString stripCurlyBraces(const QUuid &uuid)
{
    if (uuid.isNull())
        return QString();
    QString result = uuid.toString().toUpper();
    result.chop(1);
    result.remove(0, 1);
    return result;
}

static QByteArray prototype(const QByteArrayList &parameterTypes, const QByteArrayList &parameterNames, bool *ok)
{
    QByteArray prototype;

    for (qsizetype p = 0; p < parameterTypes.size() && *ok; ++p) {
        bool out = false;
        QByteArray type(parameterTypes.at(p));
        const QByteArray &name = parameterNames.at(p);

        if (type.endsWith('&')) {
            out = true;
            type.truncate(type.length() - 1);
        } else if (type.endsWith("**")) {
            out = true;
            type.truncate(type.length() - 1);
        } else if (type.endsWith('*') && !subtypes.contains(type)) {
            type.truncate(type.length() - 1);
        }
        if (type.isEmpty()) {
            *ok = false;
            break;
        }
        type = convertTypes(type, ok);
        if (!out)
            prototype += "[in] " + type + ' ';
        else
            prototype += "[in,out] " + type + ' ';

        if (out)
            prototype += '*';
        if (name.isEmpty())
            prototype += 'p' + QByteArray::number(p);
        else
            prototype += "p_" + replaceKeyword(name);

        if (p < parameterTypes.size() - 1)
            prototype += ", ";
    }

    return prototype;
}

static QByteArray addDefaultArguments(const QByteArray &prototype, int numDefArgs)
{
    // nothing to do, or unsupported anyway
    if (!numDefArgs || prototype.contains("/**"))
        return prototype;

    QByteArray ptype(prototype);
    int in = -1;
    while (numDefArgs) {
        in = ptype.lastIndexOf(']', in);
        ptype.replace(in, 1, ",optional]");
        in = ptype.indexOf(' ', in) + 1;
        QByteArray type = ptype.mid(in, ptype.indexOf(' ', in) - in);
        if (type == "enum")
            type += ' ' + ptype.mid(in + 5, ptype.indexOf(' ', in + 5) - in - 5);
        if (type == "struct")
            type += ' ' + ptype.mid(in + 7, ptype.indexOf(' ', in + 7) - in - 7);
        const QByteArray replacement = QByteArray("VARIANT /*was: ") + type + "*/";
        ptype.replace(in, type.length(), replacement);
        --numDefArgs;
    }

    return ptype;
}

static HRESULT classIDL(QObject *o, const QMetaObject *mo, const QString &className, bool isBindable, QTextStream &out)
{
    int id = 1;
    int i = 0;
    if (!mo)
        return 3;

    QString topclass = qAxFactory()->exposeToSuperClass(className);
    if (topclass.isEmpty())
        topclass = QLatin1String("QObject");
    bool hasStockEvents = qAxFactory()->hasStockEvents(className);

    const QMetaObject *pmo = mo;
    do {
        pmo = pmo->superClass();
    } while (pmo && topclass != QString::fromLatin1(pmo->className()));

    int enumoff = pmo ? pmo->enumeratorOffset() : mo->enumeratorOffset();
    int methodoff = pmo ? pmo->methodOffset() : mo->methodOffset();
    int propoff = pmo ? pmo->propertyOffset() : mo->propertyOffset();

    int qtProps = 0;
    int qtSlots = 0;

    bool control = false;

    if (o && o->isWidgetType()) {
        qtProps = QWidget::staticMetaObject.propertyCount();
        qtSlots = QWidget::staticMetaObject.methodCount();
        control = true;
    }

    const QString classID = stripCurlyBraces(qAxFactory()->classID(className));
    if (classID.isEmpty())
        return 4;
    const QString interfaceID = stripCurlyBraces(qAxFactory()->interfaceID(className));
    if (interfaceID.isEmpty())
        return 5;
    const QString eventsID = stripCurlyBraces(qAxFactory()->eventsID(className));
    const bool hasEvents = !eventsID.isEmpty();

    QString cleanClassName = qax_clean_type(className, mo);
    QString defProp(QLatin1String(mo->classInfo(mo->indexOfClassInfo("DefaultProperty")).value()));
    QString defSignal(QLatin1String(mo->classInfo(mo->indexOfClassInfo("DefaultSignal")).value()));

    for (i = enumoff; i < mo->enumeratorCount(); ++i) {
        const QMetaEnum enumerator = mo->enumerator(i);
        if (enums.contains(enumerator.name()))
            continue;

        enums.append(enumerator.name());

        out << "\tenum " << enumerator.name() << " {" << Qt::endl;

        for (int j = 0; j < enumerator.keyCount(); ++j) {
            QByteArray key(enumerator.key(j));
            while (enumValues.contains(key)) {
                key += '_';
            }
            enumValues.append(key);
            const uint value = uint(enumerator.value(j));
            key = key.leftJustified(20);
            out << "\t\t" << key << "\t= ";
            if (enumerator.isFlag())
                out << "0x" << QByteArray::number(value, 16).rightJustified(8, '0');
            else
                out << value;
            if (j < enumerator.keyCount()-1)
                out << ", ";
            out << Qt::endl;
        }
        out << "\t};" << Qt::endl << Qt::endl;
    }

    // mouse cursor enum for QCursor support
    if (!enums.contains("MousePointer")) {
        enums.append("MousePointer");
        out << "\tenum MousePointer {" << Qt::endl;
        out << "\t\tArrowCursor             = " << Qt::ArrowCursor << ',' << Qt::endl;
        out << "\t\tUpArrowCursor           = " << Qt::UpArrowCursor << ',' << Qt::endl;
        out << "\t\tCrossCursor             = " << Qt::CrossCursor << ',' << Qt::endl;
        out << "\t\tWaitCursor              = " << Qt::WaitCursor << ',' << Qt::endl;
        out << "\t\tIBeamCursor             = " << Qt::IBeamCursor << ',' << Qt::endl;
        out << "\t\tSizeVerCursor           = " << Qt::SizeVerCursor << ',' << Qt::endl;
        out << "\t\tSizeHorCursor           = " << Qt::SizeHorCursor << ',' << Qt::endl;
        out << "\t\tSizeBDiagCursor         = " << Qt::SizeBDiagCursor << ',' << Qt::endl;
        out << "\t\tSizeFDiagCursor         = " << Qt::SizeFDiagCursor << ',' << Qt::endl;
        out << "\t\tSizeAllCursor           = " << Qt::SizeAllCursor << ',' << Qt::endl;
        out << "\t\tBlankCursor             = " << Qt::BlankCursor << ',' << Qt::endl;
        out << "\t\tSplitVCursor            = " << Qt::SplitVCursor << ',' << Qt::endl;
        out << "\t\tSplitHCursor            = " << Qt::SplitHCursor << ',' << Qt::endl;
        out << "\t\tPointingHandCursor      = " << Qt::PointingHandCursor << ',' << Qt::endl;
        out << "\t\tForbiddenCursor         = " << Qt::ForbiddenCursor << ',' << Qt::endl;
        out << "\t\tWhatsThisCursor         = " << Qt::WhatsThisCursor << ',' << Qt::endl;
        out << "\t\tBusyCursor\t= " << Qt::BusyCursor << Qt::endl;
        out << "\t};" << Qt::endl << Qt::endl;
    }
    if (!enums.contains("FocusPolicy")) {
        enums.append("FocusPolicy");
        out << "\tenum FocusPolicy {" << Qt::endl;
        out << "\t\tNoFocus             = " << Qt::NoFocus << ',' << Qt::endl;
        out << "\t\tTabFocus            = " << Qt::TabFocus << ',' << Qt::endl;
        out << "\t\tClickFocus          = " << Qt::ClickFocus << ',' << Qt::endl;
        out << "\t\tStrongFocus         = " << Qt::StrongFocus << ',' << Qt::endl;
        out << "\t\tWheelFocus          = " << Qt::WheelFocus << Qt::endl;
        out << "\t};" << Qt::endl << Qt::endl;
    }

    out << Qt::endl;
    out << "\t[" << Qt::endl;
    out << "\t\tuuid(" << interfaceID << ")," << Qt::endl;
    out << "\t\thelpstring(\"" << cleanClassName << " Interface\")" << Qt::endl;
    out << "\t]" << Qt::endl;
    out << "\tdispinterface I" << cleanClassName  << Qt::endl;
    out << "\t{" << Qt::endl;

    out << "\tproperties:" << Qt::endl;
    for (i = propoff; i < mo->propertyCount(); ++i) {
        const QMetaProperty property = mo->property(i);
        /* if (property.testFlags(QMetaProperty::Override))
            continue;*/
        if (i <= qtProps && ignoreProps(property.name()))
            continue;
        if (!property.name() || mo->indexOfProperty(property.name()) > i)
            continue;

        bool ok = true;
        QByteArray type(convertTypes(property.typeName(), &ok));
        QByteArray name(replaceKeyword(property.name()));

        if (!ok)
            out << "\t/****** Property is of unsupported datatype" << Qt::endl;

        out << "\t\t[id(" << id << ')';
        if (!property.isWritable())
            out << ", readonly";
        if (isBindable && property.isScriptable())
            out << ", bindable";
        if (!property.isDesignable())
            out << ", nonbrowsable";
        if (isBindable)
            out << ", requestedit";
        if (defProp == QLatin1String(name))
            out << ", uidefault";
        out << "] " << type << ' ' << name << ';' << Qt::endl;

        if (!ok)
            out << "\t******/" << Qt::endl;
        ++id;
    }
    out << Qt::endl;
    out << "\tmethods:" << Qt::endl;
    int numDefArgs = 0;
    QByteArray outBuffer;
    for (i = methodoff; i < mo->methodCount(); ++i) {
        const QMetaMethod slot = mo->method(i);
        if (slot.access() != QMetaMethod::Public || slot.methodType() == QMetaMethod::Signal)
            continue;

        if (slot.attributes() & QMetaMethod::Cloned) {
            ++numDefArgs;
            continue;
        }
        if (!outBuffer.isEmpty()) {
            outBuffer = addDefaultArguments(outBuffer, numDefArgs);
            numDefArgs = 0;
            out << outBuffer;
            outBuffer = QByteArray();
        }

        QByteArray signature(slot.methodSignature());
        QByteArray name(signature.left(signature.indexOf('(')));
        if (i <= qtSlots && ignoreSlots(name))
            continue;

        signature.remove(0, name.length() + 1);
        signature.truncate(signature.length() - 1);
        name = renameOverloads(replaceKeyword(name));
        if (ignoreSlots(name))
            continue;

        const auto parameterTypes = slot.parameterTypes();
        const auto parameterNames = slot.parameterNames();

        bool ok = true;
        QByteArray type = slot.typeName();
        if (type.isEmpty())
            type = "void";
        else
            type = convertTypes(type, &ok);

        QByteArray ptype(prototype(parameterTypes, parameterNames, &ok));
        if (!ok)
            outBuffer += "\t/****** Slot parameter uses unsupported datatype\n";

        outBuffer += "\t\t[id(" + QByteArray::number(id) + ")] " + type + ' '
            + name + '(' + ptype + ");\n";

        if (!ok)
            outBuffer += "\t******/\n";
        ++id;
    }
    if (!outBuffer.isEmpty()) {
        outBuffer = addDefaultArguments(outBuffer, numDefArgs);
        numDefArgs = 0;
        out << outBuffer;
        outBuffer = QByteArray();
    }
    out << "\t};" << Qt::endl << Qt::endl;

    mapping.clear();
    id = 1;

    if (hasEvents) {
        out << "\t[" << Qt::endl;
        out << "\t\tuuid(" << eventsID << ")," << Qt::endl;
        out << "\t\thelpstring(\"" << cleanClassName << " Events Interface\")" << Qt::endl;
        out << "\t]" << Qt::endl;
        out << "\tdispinterface I" << cleanClassName << "Events" << Qt::endl;
        out << "\t{" << Qt::endl;
        out << "\tproperties:" << Qt::endl;
        out << "\tmethods:" << Qt::endl;

        if (hasStockEvents) {
            out << "\t/****** Stock events ******/" << Qt::endl;
            out << "\t\t[id(DISPID_CLICK)] void Click();" << Qt::endl;
            out << "\t\t[id(DISPID_DBLCLICK)] void DblClick();" << Qt::endl;
            out << "\t\t[id(DISPID_KEYDOWN)] void KeyDown(short* KeyCode, short Shift);" << Qt::endl;
            out << "\t\t[id(DISPID_KEYPRESS)] void KeyPress(short* KeyAscii);" << Qt::endl;
            out << "\t\t[id(DISPID_KEYUP)] void KeyUp(short* KeyCode, short Shift);" << Qt::endl;
            out << "\t\t[id(DISPID_MOUSEDOWN)] void MouseDown(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << Qt::endl;
            out << "\t\t[id(DISPID_MOUSEMOVE)] void MouseMove(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << Qt::endl;
            out << "\t\t[id(DISPID_MOUSEUP)] void MouseUp(short Button, short Shift, OLE_XPOS_PIXELS x, OLE_YPOS_PIXELS y);" << Qt::endl << Qt::endl;
        }

        for (i = methodoff; i < mo->methodCount(); ++i) {
            const QMetaMethod signal = mo->method(i);
            if (signal.methodType() != QMetaMethod::Signal)
                continue;

            QByteArray signature(signal.methodSignature());
            QByteArray name(signature.left(signature.indexOf('(')));
            signature.remove(0, name.length() + 1);
            signature.truncate(signature.length() - 1);

            const auto parameterTypes = signal.parameterTypes();
            const auto parameterNames = signal.parameterNames();

            bool isDefault = defSignal == QLatin1String(name);
            name = renameOverloads(replaceKeyword(name));
            bool ok = true;

            QByteArray type = signal.typeName();
            if (!type.isEmpty() && type != "void") // signals with return value not supported
                continue;

            QByteArray ptype(prototype(parameterTypes, parameterNames, &ok));
            if (!ok)
                out << "\t/****** Signal parameter uses unsupported datatype" << Qt::endl;

            out << "\t\t[id(" << id << ')';
            if (isDefault)
                out << ", uidefault";
            out << "] void " << name << '(' << ptype << ");" << Qt::endl;

            if (!ok)
                out << "\t******/" << Qt::endl;
            ++id;
        }
        out << "\t};" << Qt::endl << Qt::endl;
    }

    out << "\t[" << Qt::endl;

    if (qstricmp(mo->classInfo(mo->indexOfClassInfo("Aggregatable")).value(), "no"))
        out << "\t\taggregatable," << Qt::endl;
    if (!qstricmp(mo->classInfo(mo->indexOfClassInfo("RegisterObject")).value(), "yes"))
        out << "\t\tappobject," << Qt::endl;
    if (mo->classInfo(mo->indexOfClassInfo("LicenseKey")).value())
        out << "\t\tlicensed," << Qt::endl;
    const char *helpString = mo->classInfo(mo->indexOfClassInfo("Description")).value();
    if (helpString)
        out << "\t\thelpstring(\"" << helpString << "\")," << Qt::endl;
    else
        out << "\t\thelpstring(\"" << cleanClassName << " Class\")," << Qt::endl;
    const char *classVersion = mo->classInfo(mo->indexOfClassInfo("Version")).value();
    if (classVersion)
        out << "\t\tversion(" << classVersion << ")," << Qt::endl;
    out << "\t\tuuid(" << classID << ')';
    if (control) {
        out << ", " << Qt::endl;
        out << "\t\tcontrol";
    } else if (!o) {
        out << ", " << Qt::endl;
        out << "\t\tnoncreatable";
    }
    out << Qt::endl;
    out << "\t]" << Qt::endl;
    out << "\tcoclass " << cleanClassName << Qt::endl;
    out << "\t{" << Qt::endl;
    out << "\t\t[default] dispinterface I" << cleanClassName << ';' << Qt::endl;
    if (hasEvents)
        out << "\t\t[default, source] dispinterface I" << cleanClassName << "Events;" << Qt::endl;
    out << "\t};" << Qt::endl;

    return S_OK;
}

extern "C" HRESULT __stdcall DumpIDL(const QString &outfile, const QString &ver)
{
    qAxIsServer = false;
    QTextStream out;
    if (outfile.contains(QLatin1String("\\"))) {
        QString outpath = outfile.left(outfile.lastIndexOf(QLatin1String("\\")));
        QDir dir;
        dir.mkpath(outpath);
    }
    QFile file(outfile);
    file.remove();

    QString filebase = QString::fromWCharArray(qAxModuleFilename);
    filebase.truncate(filebase.lastIndexOf(QLatin1Char('.')));

    const QString appID = stripCurlyBraces(qAxFactory()->appID());
    if (appID.isEmpty())
        return 1;
    const QString typeLibID = stripCurlyBraces(qAxFactory()->typeLibID());
    if (typeLibID.isEmpty())
        return 2;
    QString typelib = filebase.right(filebase.length() - filebase.lastIndexOf(QLatin1String("\\"))-1);

    if (!file.open(QIODevice::WriteOnly))
        return -1;

    out.setDevice(&file);

    QString version(ver.unicode(), ver.length());
    // truncate "major.minor.patch.build" version string to "major.minor"
    while (version.count(QLatin1Char('.')) > 1) {
        int lastdot = version.lastIndexOf(QLatin1Char('.'));
        version.truncate(lastdot);
    }
    if (version.isEmpty())
        version = QLatin1String("1.0");

    const QString idQRect = stripCurlyBraces(QUuid(CLSID_QRect));
    const QString idQSize = stripCurlyBraces(QUuid(CLSID_QSize));
    const QString idQPoint = stripCurlyBraces(QUuid(CLSID_QPoint));

    out << "/****************************************************************************" << Qt::endl;
    out << "** Interface definition generated for ActiveQt project" << Qt::endl;
    out << "**" << Qt::endl;
    out << "**     '" << QString::fromWCharArray(qAxModuleFilename) << '\'' << Qt::endl;
    out << "**" << Qt::endl;
    out << "** Created:  " << QDateTime::currentDateTime().toString() << Qt::endl;
    out << "**" << Qt::endl;
    out << "** WARNING! All changes made in this file will be lost!" << Qt::endl;
    out << "****************************************************************************/" << Qt::endl << Qt::endl;

    out << "import \"ocidl.idl\";" << Qt::endl;
    out << "#include <olectl.h>" << Qt::endl << Qt::endl;

    // dummy application to create widgets
    bool delete_qApp = false;
    if (!qApp) {
        (void)new QApplication(__argc, __argv);
        delete_qApp = true;
    }

    out << '[' << Qt::endl;
    out << "\tuuid(" << typeLibID << ")," << Qt::endl;
    out << "\tversion(" << version << ")," << Qt::endl;
    out << "\thelpstring(\"" << typelib << ' ' << version << " Type Library\")" << Qt::endl;
    out << ']' << Qt::endl;
    out << "library " << typelib << "Lib" << Qt::endl;
    out << '{' << Qt::endl;
    out << "\timportlib(\"stdole32.tlb\");" << Qt::endl;
    out << "\timportlib(\"stdole2.tlb\");" << Qt::endl << Qt::endl;

    const QStringList keys = qAxFactory()->featureList();

    out << "\t/************************************************************************" << Qt::endl;
    out << "\t** If this causes a compile error in MIDL you need to upgrade the" << Qt::endl;
    out << "\t** Platform SDK you are using. Download the SDK from msdn.microsoft.com" << Qt::endl;
    out << "\t** and make sure that both the system and the Visual Studio environment" << Qt::endl;
    out << "\t** use the correct files." << Qt::endl;
    out << "\t**" << Qt::endl;

#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t** Required version of MIDL could not be verified. QRect, QSize and QPoint" << Qt::endl;
    out << "\t** support needs an updated Platform SDK to be installed." << Qt::endl;
    out << "\t*************************************************************************" << Qt::endl;
#else
    out << "\t************************************************************************/" << Qt::endl;
#endif

    out << Qt::endl;
    out << "\t[uuid(" << idQRect << ")]" << Qt::endl;
    out << "\tstruct QRect {" << Qt::endl;
    out << "\t\tint left;" << Qt::endl;
    out << "\t\tint top;" << Qt::endl;
    out << "\t\tint right;" << Qt::endl;
    out << "\t\tint bottom;" << Qt::endl;
    out << "\t};" << Qt::endl << Qt::endl;

    out << "\t[uuid(" << idQSize << ")]" << Qt::endl;
    out << "\tstruct QSize {" << Qt::endl;
    out << "\t\tint width;" << Qt::endl;
    out << "\t\tint height;" << Qt::endl;
    out << "\t};" << Qt::endl << Qt::endl;

    out << "\t[uuid(" << idQPoint << ")]" << Qt::endl;
    out << "\tstruct QPoint {" << Qt::endl;
    out << "\t\tint x;" << Qt::endl;
    out << "\t\tint y;" << Qt::endl;
    out << "\t};" << Qt::endl;
#if __REQUIRED_RPCNDR_H_VERSION__ < Q_REQUIRED_RPCNDR_H_VERSION
    out << "\t*/" << Qt::endl;
#endif
    out << Qt::endl;

    out << "\t/* Forward declaration of classes that might be used as parameters */" << Qt::endl << Qt::endl;

    int res = S_OK;
    for (const QString &className : keys) {
        const QMetaObject *mo = qAxFactory()->metaObject(className);
        // We have meta object information for this type. Forward declare it.
        if (mo) {
            QByteArray cleanType = qax_clean_type(className, mo).toLatin1();
            out << "\tcoclass " << cleanType << ';' << Qt::endl;
            subtypes.append(cleanType);
            if (QMetaType::fromName(cleanType).id() == QMetaType::UnknownType)
                qRegisterMetaType<void *>(cleanType);
            cleanType += '*';
            subtypes.append(cleanType);
            if (QMetaType::fromName(cleanType).id() == QMetaType::UnknownType)
                qRegisterMetaType<void *>(cleanType);
        }
    }
    out << Qt::endl;

    for (const QString &className : keys) {
        const QMetaObject *mo = qAxFactory()->metaObject(className);
        // We have meta object information for this type. Define it.
        if (mo) {
            QObject *o = qAxFactory()->createObject(className);
            // It's not a control class, so it is actually a subtype. Define it.
            if (!o)
                res = classIDL(nullptr, mo, className, false, out);
            delete o;
        }
    }

    out << Qt::endl;
    if (res != S_OK)
        goto ErrorInClass;

    for (const QString &className : keys) {
        QObject *o = qAxFactory()->createObject(className);
        if (!o)
            continue;
        const QMetaObject *mo = o->metaObject();
        QAxBindable *bind = static_cast<QAxBindable *>(o->qt_metacast("QAxBindable"));
        bool isBindable =  bind != nullptr;

        const QByteArray cleanType = qax_clean_type(className, mo).toLatin1();
        subtypes.append(cleanType);
        subtypes.append(cleanType + '*');
        res = classIDL(o, mo, className, isBindable, out);
        delete o;
        if (res != S_OK)
            break;
    }

    out << "};" << Qt::endl;
    out.flush();

ErrorInClass:
    if (delete_qApp)
        delete qApp;

    if (res != S_OK) {
        file.close();
        file.remove();
    }

    return res;
}

QT_END_NAMESPACE
