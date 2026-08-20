// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "comutil_p.h"
#include "testutil_p.h"

#include <qaxtypes_p.h>
#include <ActiveQt/qaxobject.h>

#include <QtTest/QtTest>
#include <QtCore/QVariant>
#include <QtCore/QDateTime>
#include <QtCore/QMetaType>
#include <QtCore/private/qcomptr_p.h>

#include <qt_windows.h>

using namespace Qt::StringLiterals;

class tst_Conversion : public QObject
{
    Q_OBJECT

private slots:
    void conversion_data();
    void conversion();

    void VARIANTToQVariant_ReturnsBool_WhenCalledWithVariantBool();
    void QVariantToVARIANT_ReturnsVariantBool_WhenCalledWithBool();

    void VARIANTToQVariant_ReturnsString_WhenCalledWithString_data();
    void VARIANTToQVariant_ReturnsString_WhenCalledWithString();

    void QVariantToVARIANT_ReturnsString_WhenCalledWithString_data();
    void QVariantToVARIANT_ReturnsString_WhenCalledWithString();

    void VARIANTToQVariant_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValueOrPointer_data();
    void VARIANTToQVariant_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValueOrPointer();

    void QVariantToVARIANT_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValue_data();
    void QVariantToVARIANT_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValue();

    void VARIANTToQVariant_DoesNotIncreaseRefCount_WhenGivenAnIUnknown();
    void QVariantToVARIANT_RecoversIUnknown_WhenQVariantHasIUnknown();

    void VARIANTToQVariant_DoesNotIncreaseRefCount_WhenGivenAnIDispatch();
    void QVariantToVARIANT_RecoversIDispatch_WhenQVariantHasIDispatch();

    void VARIANTToQVariant_IncreasesRefCount_WhenCalledWithQVariantTypeName();

    void ObserveThat_VARIANTToQVariant_ReturnsEmptyQVariant_WhenWrappingIDispatchInQAxObjectPtr();
    void VARIANTToQVariant_CreatesQAxObject_WhenCalledWithMetaTypeId();

private:
    template<typename T>
    static void addScalarMaxValueRow();
};

enum Mode {
    ByValue,
    ByReference, // Allocate new value
    OutParameter // Pre-allocated out-parameter by reference (test works only for types < qint64)
};

Q_DECLARE_METATYPE(Mode)

void tst_Conversion::conversion_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<uint>("expectedType");
    QTest::addColumn<QByteArray>("typeName");
    QTest::addColumn<Mode>("mode");

    QVariant qvar;
    QByteArray typeName;

    qvar = QVariant('a');
    typeName = QByteArrayLiteral("char");
    QTest::newRow("char")
        << qvar << uint(VT_I1) << typeName << ByValue;
    QTest::newRow("char-ref")
        << qvar << uint(VT_I1 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("char-out")
        << qvar << uint(VT_I1 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(uchar(197));
    typeName = QByteArrayLiteral("uchar");
    QTest::newRow("uchar")
        << qvar << uint(VT_UI1) << typeName << ByValue;
    QTest::newRow("uchar-ref")
        << qvar << uint(VT_UI1 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("uchar-out")
        << qvar << uint(VT_UI1 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(ushort(42));
    typeName = QByteArrayLiteral("ushort");
    QTest::newRow("ushort")
        << qvar << uint(VT_UI2) << typeName << ByValue;
    QTest::newRow("ushort-ref")
        << qvar << uint(VT_UI2 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("ushort-out")
        << qvar << uint(VT_UI2 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(short(42));
    typeName = QByteArrayLiteral("short");
    QTest::newRow("short")
        << qvar << uint(VT_I2) << typeName << ByValue;
    QTest::newRow("short-ref")
        << qvar << uint(VT_I2 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("short-out")
        << qvar << uint(VT_I2 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(42);
    typeName.clear();
    QTest::newRow("int")
        << qvar << uint(VT_I4) << typeName << ByValue;
    QTest::newRow("int-ref")
        << qvar << uint(VT_I4 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("int-out")
        << qvar << uint(VT_I4 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(42u);
    typeName.clear();
    QTest::newRow("uint")
        << qvar << uint(VT_UI4) << typeName << ByValue;
    QTest::newRow("uint-ref")
        << qvar << uint(VT_UI4 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("uint-out")
        << qvar << uint(VT_UI4 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(qint64(42));
    typeName.clear();
    QTest::newRow("int64")
        << qvar << uint(VT_I8) << typeName << ByValue;
    QTest::newRow("int64-ref")
        << qvar << uint(VT_I8 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("int64-out")
        << qvar << uint(VT_I8 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(quint64(42u));
    typeName.clear();
    QTest::newRow("uint64")
        << qvar << uint(VT_UI8) << typeName << ByValue;
    QTest::newRow("uint64-ref")
        << qvar << uint(VT_UI8 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("uint64-out")
        << qvar << uint(VT_UI8 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(3.141f);
    typeName = QByteArrayLiteral("float");
    QTest::newRow("float")
        << qvar << uint(VT_R4) << typeName << ByValue;
    QTest::newRow("float-ref")
        << qvar << uint(VT_R4 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("float-out")
        << qvar << uint(VT_R4 | VT_BYREF) << typeName << OutParameter;

    qvar = QVariant(3.141);
    typeName.clear();
    QTest::newRow("double")
        << qvar << uint(VT_R8) << typeName << ByValue;
    QTest::newRow("double-ref")
        << qvar << uint(VT_R8 | VT_BYREF) << typeName << ByReference;
    QTest::newRow("double-out")
        << qvar << uint(VT_R8 | VT_BYREF) << typeName << OutParameter;

    qvar = QDateTime(QDate(1968, 3, 9), QTime(10, 0));
    typeName.clear();
    QTest::newRow("datetime")
        << qvar << uint(VT_DATE) << typeName << ByValue;
    QTest::newRow("datetime-ref")
        << qvar << uint(VT_DATE | VT_BYREF) << typeName << ByReference;
    QTest::newRow("datetime-out")
        << qvar << uint(VT_DATE | VT_BYREF) << typeName << OutParameter;

    // QTBUG-122762; do not create a 2-dimensional array from a string (sequence).
    qvar = QVariant(QVariantList{QVariant(QString("test"_L1)), QVariant(42)});
    QTest::newRow("list")
            << qvar << uint(VT_VARIANT | VT_ARRAY | VT_BYREF) << typeName << OutParameter;
}

void tst_Conversion::conversion()
{
    QFETCH(QVariant, value);
    QFETCH(uint, expectedType);
    QFETCH(QByteArray, typeName);
    QFETCH(Mode, mode);

    VARIANT variant;
    VariantInit(&variant);
    if (mode == OutParameter) {
        variant.vt = expectedType | VT_BYREF;
        variant.pullVal = new ULONGLONG(0);
    }

    QVERIFY(QVariantToVARIANT_container(value, variant, typeName, mode != ByValue));
    QCOMPARE(uint(variant.vt), expectedType);
    const QVariant converted = VARIANTToQVariant_container(variant, QByteArray());
    QCOMPARE(converted, value);

    if (mode == OutParameter)
        delete variant.pullVal;
}

void tst_Conversion::VARIANTToQVariant_ReturnsBool_WhenCalledWithVariantBool()
{
    {
        const ComVariant v = true;
        const QVariant result = VARIANTToQVariant(v, "canBeAnything");
        QVERIFY(result == true);
    }

    {
        const ComVariant v = false;
        const QVariant result = VARIANTToQVariant(v, "canBeAnything");
        QVERIFY(result == false);
    }
}

void tst_Conversion::QVariantToVARIANT_ReturnsVariantBool_WhenCalledWithBool()
{
    {
        const QVariant v = true;
        ComVariant result;
        QVERIFY(QVariantToVARIANT(v, result));
        QVERIFY(result.vt == VT_BOOL);
        QVERIFY(result.boolVal == VARIANT_TRUE);
    }

    {
        const QVariant v = false;
        ComVariant result;
        QVERIFY(QVariantToVARIANT(v, result));
        QVERIFY(result.vt == VT_BOOL);
        QVERIFY(result.boolVal == VARIANT_FALSE);
    }
}

void tst_Conversion::VARIANTToQVariant_ReturnsString_WhenCalledWithString_data()
{
    QTest::addColumn<QString>("text");
    QTest::newRow("empty") << QString{ "" };
    QTest::newRow("nonempty") << QString{ "Some Latin 1 text" };
}

void tst_Conversion::VARIANTToQVariant_ReturnsString_WhenCalledWithString()
{
    QFETCH(QString, text);

    const ComVariant comVariant = text.toStdWString().c_str();
    const QVariant actual = VARIANTToQVariant(comVariant, {});

    QCOMPARE(actual, text);
}

void tst_Conversion::QVariantToVARIANT_ReturnsString_WhenCalledWithString_data()
{
    QTest::addColumn<QString>("text");
    QTest::newRow("empty") << QString{ "" };
    QTest::newRow("nonempty") << QString{ "Some Latin 1 text" };
}

void tst_Conversion::QVariantToVARIANT_ReturnsString_WhenCalledWithString()
{
    QFETCH(QString, text);

    ComVariant comVariant;
    QVERIFY(QVariantToVARIANT(text, comVariant));

    const QString actual = QString::fromWCharArray(comVariant.bstrVal);

    QCOMPARE(actual, text);
}

void tst_Conversion::
        VARIANTToQVariant_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValueOrPointer_data()
{
    QTest::addColumn<ComVariant>("comVariant");
    QTest::addColumn<QVariant>("qvariant");

     addScalarMaxValueRow<char>();
     addScalarMaxValueRow<unsigned char>();
     addScalarMaxValueRow<short>();
     addScalarMaxValueRow<unsigned short>();
     addScalarMaxValueRow<int>();
     addScalarMaxValueRow<unsigned int>();
     addScalarMaxValueRow<long long>();
     addScalarMaxValueRow<unsigned long long>();
     addScalarMaxValueRow<float>();
     addScalarMaxValueRow<double>();

     addScalarMaxValueRow<char *>();
     addScalarMaxValueRow<unsigned char *>();
     addScalarMaxValueRow<short *>();
     addScalarMaxValueRow<unsigned short *>();
     addScalarMaxValueRow<int *>();
     addScalarMaxValueRow<unsigned int *>();
     addScalarMaxValueRow<long long *>();
     addScalarMaxValueRow<unsigned long long *>();
     addScalarMaxValueRow<float *>();
     addScalarMaxValueRow<double *>();
}

void tst_Conversion::VARIANTToQVariant_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValueOrPointer()
{
    QFETCH(ComVariant, comVariant);
    QFETCH(QVariant, qvariant);

    const QVariant actual = VARIANTToQVariant(comVariant, qvariant.typeName());

    QCOMPARE(actual, qvariant);
}

void tst_Conversion::QVariantToVARIANT_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValue_data()
{
    QTest::addColumn<ComVariant>("comVariant");
    QTest::addColumn<QVariant>("qvariant");

    addScalarMaxValueRow<int>();
    addScalarMaxValueRow<unsigned int>();
    addScalarMaxValueRow<long long>();
    addScalarMaxValueRow<unsigned long long>();
    addScalarMaxValueRow<float>();
    addScalarMaxValueRow<double>();
}

void tst_Conversion::QVariantToVARIANT_ReturnsCopyOfValue_WhenCalledWithMaxPossibleValue()
{
    QFETCH(ComVariant, comVariant);
    QFETCH(QVariant, qvariant);

    ComVariant actual;
    QVERIFY(QVariantToVARIANT(qvariant, actual));

    QCOMPARE(actual, comVariant);
}

void tst_Conversion::VARIANTToQVariant_DoesNotIncreaseRefCount_WhenGivenAnIUnknown()
{
    const auto stub = makeComObject<IUnknownStub>();

    const ComVariant value = stub.Get();

    const ULONG expectedRefCount = refCount(stub);

    const QVariant qVariant = VARIANTToQVariant(value, {});

    QVERIFY(refCount(stub) == expectedRefCount);

    Q_UNUSED(qVariant);
}

void tst_Conversion::QVariantToVARIANT_RecoversIUnknown_WhenQVariantHasIUnknown()
{
    const auto stub = makeComObject<IUnknownStub>();
    const ComVariant value = stub.Get();

    const QVariant qvar = VARIANTToQVariant(value, {});

    ComVariant comVariant;

    const ULONG expectedRefCount = refCount(stub) + 1; // Add one for the VARIANT

    QVERIFY(QVariantToVARIANT(qvar, comVariant));

    QCOMPARE(refCount(stub), expectedRefCount);

    const ComPtr<IUnknown> recovered = comVariant.punkVal;

    QVERIFY(recovered == stub);
}

void tst_Conversion::VARIANTToQVariant_DoesNotIncreaseRefCount_WhenGivenAnIDispatch()
{
    const auto stub = makeComObject<IDispatchStub>();

    const ComVariant value = stub.Get();

    const ULONG expectedRefCount = refCount(stub);

    const QVariant qVariant = VARIANTToQVariant(value, "IDispatch*");

    QCOMPARE(refCount(stub), expectedRefCount);

    Q_UNUSED(qVariant);
}

struct IDispatchFixture
{
    const ComPtr<IDispatchStub> m_iDispatchStub = makeComObject<IDispatchStub>();
    const ComVariant m_comVariant = m_iDispatchStub.Get();
    const QVariant m_qVariant = VARIANTToQVariant(m_comVariant, "IDispatch*");
};

void tst_Conversion::QVariantToVARIANT_RecoversIDispatch_WhenQVariantHasIDispatch()
{
    const IDispatchFixture testFixture;
    const auto expectedRefCount = refCount(testFixture.m_iDispatchStub) + 1;

    ComVariant comVariant;
    QVERIFY(QVariantToVARIANT(testFixture.m_qVariant, comVariant));

    QCOMPARE(refCount(testFixture.m_iDispatchStub), expectedRefCount);

    const ComPtr<IUnknown> recovered = comVariant.pdispVal;

    QVERIFY(recovered == testFixture.m_iDispatchStub);
}

void tst_Conversion::VARIANTToQVariant_IncreasesRefCount_WhenCalledWithQVariantTypeName()
{
    const IDispatchFixture testFixture;
    const auto expectedRefCount = refCount(testFixture.m_iDispatchStub) + 1;

    QVariant qVariant = VARIANTToQVariant(testFixture.m_comVariant, "QVariant");
    qVariant = {};

    // Observe that IDispatch interface is leaked here, since
    // the QVariant destructor does not decrement the refcount
    QCOMPARE(refCount(testFixture.m_iDispatchStub), expectedRefCount);

    // Workaround to ensure cleanup
    testFixture.m_iDispatchStub->Release();
}

void tst_Conversion::ObserveThat_VARIANTToQVariant_ReturnsEmptyQVariant_WhenWrappingIDispatchInQAxObjectPtr()
{
    const IDispatchFixture testFixture;
    QCOMPARE(refCount(testFixture.m_iDispatchStub), 2u);

    qRegisterMetaType<QAxObject *>("QAxObject*");
    qRegisterMetaType<QAxObject>("QAxObject");

    const QVariant qVariant = VARIANTToQVariant(testFixture.m_comVariant, "QAxObject*");
    QVERIFY(qVariant.isNull());
}

void tst_Conversion::VARIANTToQVariant_CreatesQAxObject_WhenCalledWithMetaTypeId()
{
    const IDispatchFixture testFixture;
    QCOMPARE(refCount(testFixture.m_iDispatchStub), 2u);

    qRegisterMetaType<QAxObject *>("QAxObject*");
    qRegisterMetaType<QAxObject>("QAxObject");

    const QVariant qVariant = VARIANTToQVariant(testFixture.m_comVariant, "QAxObject*", QMetaType::fromType<QAxObject*>().id());

    QAxObject *recovered = qVariant.value<QAxObject *>();
    QVERIFY(recovered != nullptr);
}

template<typename T>
void tst_Conversion::addScalarMaxValueRow()
{
    using ValueType = std::remove_pointer_t<T>;
    static ValueType v = std::numeric_limits<ValueType>::max();

    ComVariant comVariant;
    if constexpr (std::is_pointer_v<T>)
        comVariant = &v;
    else
        comVariant = v;

    const char *typeName = QMetaType::fromType<T>().name();
    QTest::newRow(typeName) << comVariant << QVariant{ v };
}

QTEST_MAIN(tst_Conversion)

#include "tst_conversion.moc"
