// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qcansignaldescription.h"
#include "qcansignaldescription_p.h"

QT_BEGIN_NAMESPACE

/*!
    \class QCanSignalDescription
    \inmodule QtSerialBus
    \since 6.5
    \preliminary

    \brief The QCanSignalDescription class describes the rules to extract one
    value out of the CAN frame and represent it in an application-defined
    format.

    The QCanSignalDescription class can be used to provide a signal description
    and later use it to decode a received \l QCanBusFrame or encode the input
    data into a \l QCanBusFrame that can be sent to the receiver.

    \section2 General Description

    Each CAN frame can contain multiple values. The rules to extract the values
    from a CAN frame include the following:
    \list
        \li Data source (frame ID or payload).
        \li Data endianness. See \l {Data Endianness Processing} section for
            more details.
        \li Data format.
        \li Start bit position.
        \li Data length in bits.
        \li Multiplexing options.
    \endlist

    Start bit position is specified relative to the selected data source. The
    bits are counted starting from the LSB.

    Once the data is extracted, it might require conversion to an
    application-defined format. The following parameters can be used for that:
    \list
        \li Various parameters for converting the extracted value to a physical
            value (factor, offset, scale).
        \li Expected data range.
        \li Data units.
    \endlist

    The QCanSignalDescription class provides methods to control all those
    parameters.

    \section2 Data Endianness Processing

    Little endian and big endian data is encoded differently.
    For big endian values, start bit positions are given for the most
    significant bit. For little endian values, the start position is that of
    the least significant bit.

    Let's consider two examples. In both examples we will encode two 12-bit
    values in the 3-byte payload.

    \section3 Little Endian

    For the little endian case the data layout can be represented by the
    following image:

    \image canbus_signals_le.png

    Here the columns represent bit numbers, and the rows represent byte numbers.
    \c {LSB} marks the first (least significant) bit of the value, and \c {MSB}
    marks the last (most significant) bit of the value. The blue color marks the
    first value, and the orange color marks the second value.

    The information about these values will be encoded in QCanSignalDescription
    in the following way:

    \code
    QCanSignalDescription signal1;
    signal1.setDataEndian(QSysInfo::Endian::LittleEndian);
    signal1.setStartBit(0);
    signal1.setBitLength(12);
    // other parameters for signal1

    QCanSignalDescription signal2;
    signal2.setDataEndian(QSysInfo::Endian::LittleEndian);
    signal2.setStartBit(12);
    signal2.setBitLength(12);
    // other parameters for signal2
    \endcode

    \section3 Big Endian

    The following image represents the value layout for the big endian case:

    \image canbus_signals_be.png

    The values can be represented in QCanSignalDescription in the following
    way:

    \code
    QCanSignalDescription signal1;
    signal1.setDataEndian(QSysInfo::Endian::BigEndian);
    signal1.setStartBit(7);
    signal1.setBitLength(12);
    // other parameters for signal1

    QCanSignalDescription signal2;
    signal2.setDataEndian(QSysInfo::Endian::BigEndian);
    signal2.setStartBit(11);
    signal2.setBitLength(12);
    // other parameters for signal2
    \endcode

    Note how the start bits are different from the little endian case. Also the
    values are aligned differently.

    \section2 Multiplexed Signals Explained

    There are two common ways to encode the data in the CAN payload:
    \list
        \li Each range of bits always represents the same signal. For example,
            \c {Bytes 0-1} in a payload can represent an engine speed (in rpm),
            and \c {Bytes 2-3} can represent the vehicle speed (in km/h).
        \li The same range of bits can represent different data, depending on
            the values of some other bits in the payload. For example, if
            \c {Byte 0} has the value \c {0}, the \c {Bytes 1-2} represent an
            engine speed (in rpm), and if \c {Byte 0} has the value \c {1}, the
            same \c {Bytes 1-2} represent a vehicle speed (in km/h).
    \endlist

    The second case uses signal multiplexing. In the provided example we will
    have three signals. The first signal represents the value of \c {Byte 0} and
    acts like a multiplexor signal. The other two signals represent an engine
    speed and a vehicle speed respectively, but only one of them can be
    extracted from the CAN payload at a time. Which signal should be extracted
    is defined by the value of the multiplexor signal.

    In more complicated cases the payload can have multiple multiplexor signals.
    In such cases the signal can be extracted from the payload only when all
    multiplexors contain the expected values.

    \section2 Value Conversions

    In many cases the signals transferred over CAN bus cannot hold the full
    range of the physical values that they represent. To overcome these
    limitations, the physical values are converted to a smaller range before
    transmission, and can be restored on the receiving end.

    The following formulas are used to convert between the physical value and
    the signal's value:

    \badcode
    physicalValue = scaling * (signalValue * factor + offset);
    signalValue = (physicalValue / scaling - offset) / factor;
    \endcode

    The factor and scaling parameters cannot be equal to \c {0}.

    If any of the parameters equals to \l qQNaN(), it is not used during the
    conversion. If all of the parameters are equal to \l qQNaN() (which is the
    default), the conversion is not performed.
*/

/*!
    \struct QCanSignalDescription::MultiplexValueRange
    \inmodule QtSerialBus
    \since 6.5

    \brief Defines a range of values for a multiplexor signal.

    Each multiplexor signal can have several ranges of values assigned to it.
    This type represents one range. Both minimum and maximum values are
    included in the range. Minimum and maximum values can be equal, so a
    MultiplexValueRange is never empty. If maximum is less than minimum, the
    values will be swapped while doing the range check.

    \sa {Multiplexed Signals Explained}
*/

/*!
    \variable QCanSignalDescription::MultiplexValueRange::minimum
    \brief the minimum value of the range.
*/

/*!
    \variable QCanSignalDescription::MultiplexValueRange::maximum
    \brief the maximum value of the range.
*/

/*!
    \typealias QCanSignalDescription::MultiplexValues
*/

/*!
    \typealias QCanSignalDescription::MultiplexSignalValues
*/

/*!
    Creates an empty signal description.
*/
QCanSignalDescription::QCanSignalDescription() : d(new QCanSignalDescriptionPrivate)
{
}

/*!
    Creates a signal description with the values copied from \a other.
*/
QCanSignalDescription::QCanSignalDescription(const QCanSignalDescription &other) : d(other.d)
{
}

/*!
    \fn QCanSignalDescription::QCanSignalDescription(QCanSignalDescription &&other) noexcept

    Creates a signal description by moving from \a other.

    \note The moved-from QCanSignalDescription object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor or
    one of the assignment operators is undefined.
*/

/*!
    \fn QCanSignalDescription::~QCanSignalDescription()

    Destroys this signal description.
*/

QT_DEFINE_QESDP_SPECIALIZATION_DTOR(QCanSignalDescriptionPrivate)

/*!
    Assigns the values from \a other to this signal description.
*/
QCanSignalDescription &QCanSignalDescription::operator=(const QCanSignalDescription &other)
{
    d = other.d;
    return *this;
}

/*!
    \fn QCanSignalDescription &QCanSignalDescription::operator=(QCanSignalDescription &&other) noexcept

    Move-assigns the values from \a other to this signal description.

    \note The moved-from QCanSignalDescription object can only be destroyed or
    assigned to. The effect of calling other functions than the destructor or
    one of the assignment operators is undefined.
*/

/*!
    Returns \c true when the signal description is valid and \c false otherwise.

    A valid signal description \e must fulfill the following conditions:
    \list
        \li have a non-empty \l name()
        \li have \l bitLength() \c {== 32} if the \l dataFormat() is
            \l {QtCanBus::DataFormat::}{Float}
        \li have \l bitLength() \c {== 64} if the \l dataFormat() is
            \l {QtCanBus::DataFormat::}{Double}
        \li the \l bitLength() \e must be a multiple of \c 8 if the
            \l dataFormat() is \l {QtCanBus::DataFormat::}{AsciiString}
        \li the \l bitLength() \e must be greater than \c 0 and less than or
            equal to \c {64}.
    \endlist

    \sa bitLength(), dataFormat(), name()
*/
bool QCanSignalDescription::isValid() const
{
    const bool formatMatch = [this]() {
        if (d->format == QtCanBus::DataFormat::Float)
            return d->dataLength == 32;
        if (d->format == QtCanBus::DataFormat::Double)
            return d->dataLength == 64;
        if (d->format == QtCanBus::DataFormat::AsciiString)
            return d->dataLength % 8 == 0;
        return d->dataLength > 0 && d->dataLength <= 64;
    }();
    return !d->name.isEmpty() && formatMatch;
}

/*!
    Returns the name of the signal.

    \sa setName(), isValid()
*/
QString QCanSignalDescription::name() const
{
    return d->name;
}

/*!
    Sets the name of the signal to \a name.

    The signal's name must be unique within a CAN message.

    \sa name()
*/
void QCanSignalDescription::setName(const QString &name)
{
    d.detach();
    d->name = name;
}

/*!
    Returns the physical unit (e.g. km/h) of the signal's value or an empty
    string if the unit is not set.

//! [qcansignaldesc-aux-parameter]
    This parameter is introduced only for extra description. It's not used
    during signal processing.
//! [qcansignaldesc-aux-parameter]

    \sa setPhysicalUnit()
*/
QString QCanSignalDescription::physicalUnit() const
{
    return d->unit;
}

/*!
    Sets the physical \a unit (e.g. km/h) of the signal's value.

    \include qcansignaldescription.cpp qcansignaldesc-aux-parameter

    \sa physicalUnit()
*/
void QCanSignalDescription::setPhysicalUnit(const QString &unit)
{
    d.detach();
    d->unit = unit;
}

/*!
    Returns the receiver node for this signal.

    \include qcansignaldescription.cpp qcansignaldesc-aux-parameter

    \sa setReceiver()
*/
QString QCanSignalDescription::receiver() const
{
    return d->receiver;
}

/*!
    Sets the \a receiver node for this signal.

    \include qcansignaldescription.cpp qcansignaldesc-aux-parameter

    \sa receiver()
*/
void QCanSignalDescription::setReceiver(const QString &receiver)
{
    d.detach();
    d->receiver = receiver;
}

/*!
    Returns the comment for the signal.

    \include qcansignaldescription.cpp qcansignaldesc-aux-parameter

    \sa setComment()
*/
QString QCanSignalDescription::comment() const
{
    return d->comment;
}

/*!
    Sets the comment for the signal to \a text.

    \include qcansignaldescription.cpp qcansignaldesc-aux-parameter

    \sa comment()
*/
void QCanSignalDescription::setComment(const QString &text)
{
    d.detach();
    d->comment = text;
}

/*!
    Returns the data source of the signal's value.

    By default, \l {QtCanBus::DataSource::}{Payload} is used.

    \sa setDataSource(), QtCanBus::DataSource
*/
QtCanBus::DataSource QCanSignalDescription::dataSource() const
{
    return d->source;
}

/*!
    Sets the data source of the signal's value to \a source.

    \sa dataSource(), QtCanBus::DataSource
*/
void QCanSignalDescription::setDataSource(QtCanBus::DataSource source)
{
    d.detach();
    d->source = source;
}

/*!
    Returns the data endian of the signal's value.

    By default, \l {QSysInfo::}{BigEndian} is used.

    \note The data endian is ignored if the \l dataFormat() is set to
    \l {QtCanBus::DataFormat::}{AsciiString}.

    \sa setDataEndian(), QSysInfo::Endian
*/
QSysInfo::Endian QCanSignalDescription::dataEndian() const
{
    return d->endian;
}

/*!
    Sets the data endian of the signal's value to \a endian.

    \sa dataEndian(), QSysInfo::Endian
*/
void QCanSignalDescription::setDataEndian(QSysInfo::Endian endian)
{
    d.detach();
    d->endian = endian;
}

/*!
    Returns the data format of the signal's value.

    By default, \l {QtCanBus::DataFormat::}{SignedInteger} is used.

    \sa setDataFormat(), QtCanBus::DataFormat
*/
QtCanBus::DataFormat QCanSignalDescription::dataFormat() const
{
    return d->format;
}

/*!
    Sets the data format of the signal's value to \a format.

    \sa dataFormat(), QtCanBus::DataFormat
*/
void QCanSignalDescription::setDataFormat(QtCanBus::DataFormat format)
{
    d.detach();
    d->format = format;
}

/*!
    Returns the start bit of the signal's value in the \l dataSource().

    \sa setStartBit(), bitLength(), setBitLength()
*/
quint16 QCanSignalDescription::startBit() const
{
    return d->startBit;
}

/*!
    Sets the start bit of the signal's value in the \l dataSource() to \a bit.

    \sa startBit(), bitLength(), setBitLength()
*/
void QCanSignalDescription::setStartBit(quint16 bit)
{
    d.detach();
    d->startBit = bit;
}

/*!
    Returns the bit length of the signal's value.

    \sa setBitLength(), startBit(), setStartBit()
*/
quint16 QCanSignalDescription::bitLength() const
{
    return d->dataLength;
}

/*!
    Sets the bit length of the signal's value to \a length.

    \sa bitLength(), startBit(), setStartBit()
*/
void QCanSignalDescription::setBitLength(quint16 length)
{
    d.detach();
    d->dataLength = length;
}

/*!
    Returns the factor that is used to convert the signal's value to a physical
    value and back.

    By default the function returns \l qQNaN(), which means that a factor is not
    used.

    The \l {Value Conversions} section explains how this parameter is used.

    \sa setFactor(), offset(), scaling()
*/
double QCanSignalDescription::factor() const
{
    return d->factor;
}

/*!
    Sets the factor that is used to convert the signal's value to a physical
    value and back to \a factor.

    Pass \l qQNaN() to this method to skip this parameter during the conversion.

    The factor cannot be 0. An attempt to set a zero factor is equivalent to
    setting it to \l qQNaN().

    The \l {Value Conversions} section explains how this parameter is used.

    \sa factor(), setOffset(), setScaling()
*/
void QCanSignalDescription::setFactor(double factor)
{
    d.detach();
    if (qFuzzyIsNull(factor))
        d->factor = qQNaN();
    else
        d->factor = factor;
}

/*!
    Returns the offset that is used to convert the signal's value to a physical
    value and back.

    By default the function returns \l qQNaN(), which means that an offset is
    not used.

    The \l {Value Conversions} section explains how this parameter is used.

    \sa setOffset(), factor(), scaling()
*/
double QCanSignalDescription::offset() const
{
    return d->offset;
}

/*!
    Sets the offset that is used to convert the signal's value to a physical
    value and back to \a offset.

    Pass \l qQNaN() to this method to skip this parameter during the conversion.

    The \l {Value Conversions} section explains how this parameter is used.

    \sa offset(), setFactor(), setScaling()
*/
void QCanSignalDescription::setOffset(double offset)
{
    d.detach();
    d->offset = offset;
}

/*!
    Returns the scaling that is used to convert the signal's value to a physical
    value and back.

    By default the function returns \l qQNaN(), which means that scaling is not
    used.

    The \l {Value Conversions} section explains how this parameter is used.

    \sa setScaling(), offset(), factor()
*/
double QCanSignalDescription::scaling() const
{
    return d->scaling;
}

/*!
    Sets the scaling that is used to convert the signal's value to a physical
    value and back to \a scaling.

    Pass \l qQNaN() to this method to skip this parameter during the conversion.

    The scaling cannot be 0. An attempt to set zero scaling is equivalent to
    setting it to \l qQNaN().

    The \l {Value Conversions} section explains how this parameter is used.

    \sa scaling(), setOffset(), setFactor()
*/
void QCanSignalDescription::setScaling(double scaling)
{
    d.detach();
    if (qFuzzyIsNull(scaling))
        d->scaling = qQNaN();
    else
        d->scaling = scaling;
}

/*!
    Returns the minimum supported value for the signal.

    By default the function returns \l qQNaN(), which means that there is no
    minimum value.

    \sa setRange(), maximum()
*/
double QCanSignalDescription::minimum() const
{
    return d->minimum;
}

/*!
    Returns the maximum supported value for the signal.

    By default the function returns \l qQNaN(), which means that there is no
    maximum value.

    \sa setRange(), minimum()
*/
double QCanSignalDescription::maximum() const
{
    return d->maximum;
}

/*!
    Sets the \a minimum and \a maximum for the signal's value.

    Setting one or both of the parameters to \l qQNaN() means that the
    corresponding limit will not be used.

    \sa minimum(), maximum()
*/
void QCanSignalDescription::setRange(double minimum, double maximum)
{
    d.detach();
    if (qIsNaN(minimum) || qIsNaN(maximum) || minimum <= maximum) {
        d->minimum = minimum;
        d->maximum = maximum;
    } else {
        qWarning("Minimum value is greater than maximum. The values will be swapped.");
        d->minimum = maximum;
        d->maximum = minimum;
    }
}

/*!
    Returns the multiplex state of the signal.

    See the \l {Multiplexed Signals Explained} section for more details on
    multiplexed signals.

    By default this method returns \l {QtCanBus::MultiplexState::}{None}.

    \sa setMultiplexState(), QtCanBus::MultiplexState
*/
QtCanBus::MultiplexState QCanSignalDescription::multiplexState() const
{
    return d->muxState;
}

/*!
    Sets the multiplex state of the signal to \a state.

    See the \l {Multiplexed Signals Explained} section for more details on
    multiplexed signals.

    \sa multiplexState(), QtCanBus::MultiplexState
*/
void QCanSignalDescription::setMultiplexState(QtCanBus::MultiplexState state)
{
    d.detach();
    d->muxState = state;
}

/*!
    Returns the \l {Multiplexed Signals Explained}{multiplexor signals} and
    their desired values that are used to properly identify this signal.

    The returned hash contains signal names as keys and respective desired
    ranges of values as values.

    This signal's value can be extracted from the payload only when all the
    signals from the hash have the expected values.

    \sa multiplexState(), clearMultiplexSignals(), setMultiplexSignals(),
    addMultiplexSignal()
*/
QCanSignalDescription::MultiplexSignalValues QCanSignalDescription::multiplexSignals() const
{
    return d->muxSignals;
}

/*!
    Removes all \l {Multiplexed Signals Explained}{multiplexor signals} for
    this signal.

    \sa multiplexSignals(), setMultiplexSignals(), addMultiplexSignal()
*/
void QCanSignalDescription::clearMultiplexSignals()
{
    d.detach();
    d->muxSignals.clear();
}

/*!
    Sets the \l {Multiplexed Signals Explained}{multiplexor signals} for this
    signal to \a multiplexorSignals.

    The \a multiplexorSignals hash \e must contain signal names as keys and
    respective desired value ranges as values.

    \sa multiplexState(), multiplexSignals(), clearMultiplexSignals(),
    addMultiplexSignal()
*/
void QCanSignalDescription::setMultiplexSignals(const MultiplexSignalValues &multiplexorSignals)
{
    d.detach();
    d->muxSignals = multiplexorSignals;
}

/*!
    Adds a new \l {Multiplexed Signals Explained}{multiplexor signal} for this
    signal. The \a name parameter contains the name of the multiplexor signal,
    and the \a ranges parameter contains the desired value ranges.

    If this signal already has desired value ranges for the multiplexor signal
    \a name, the ranges are overwritten.

    \sa multiplexState(), multiplexSignals(), clearMultiplexSignals(),
    setMultiplexSignals()
*/
void QCanSignalDescription::addMultiplexSignal(const QString &name, const MultiplexValues &ranges)
{
    d.detach();
    d->muxSignals.insert(name, ranges);
}

/*!
    \overload

    This is a convenience overload for the case when the multiplexor signal is
    expected to have only one specific value, not a range of values.

    The \a name parameter contains the name of the multiplexor signal,
    and the \a value parameter contains the desired value.

    If this signal already has desired value ranges for the multiplexor signal
    \a name, the ranges are overwritten.

    \sa multiplexState(), multiplexSignals(), clearMultiplexSignals(),
    setMultiplexSignals()
*/
void QCanSignalDescription::addMultiplexSignal(const QString &name, const QVariant &value)
{
    d.detach();
    d->muxSignals.insert(name, { {value, value} });
}

#ifndef QT_NO_DEBUG_STREAM
QDebug QCanSignalDescription::debugStreaming(QDebug dbg, const QCanSignalDescription &sig)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "QCanSignalDescription(" << sig.name() << ", Source = " << sig.dataSource()
                  << ", Format = " << sig.dataFormat() << ", Endian = " << sig.dataEndian()
                  << ", StartBit = " << sig.startBit() << ", BitLength = " << sig.bitLength();
    if (!sig.physicalUnit().isEmpty())
        dbg << ", Units = " << sig.physicalUnit();
    if (!sig.receiver().isEmpty())
        dbg << ", Receiver = " << sig.receiver();
    if (!sig.comment().isEmpty())
        dbg << ", Comment = " << sig.comment();
    dbg << ", Factor = " << sig.factor() << ", Offset = " << sig.offset()
        << ", Scaling = " << sig.scaling();
    dbg << ", Minimum = " << sig.minimum() << ", Maximum = " << sig.maximum();
    dbg << ", Multiplex State = " << sig.multiplexState();
    const auto muxSignals = sig.multiplexSignals();
    if (!muxSignals.isEmpty()) {
        dbg << ", Multiplexor Signals: {";
        for (auto it = muxSignals.cbegin(); it != muxSignals.cend(); ++it) {
            if (it != muxSignals.cbegin())
                dbg << ", ";
            dbg << "(" << it.key() << ", " << it.value() << ")";
        }
        dbg << "}";
    }
    dbg << ")";
    return dbg;
}

QDebug QCanSignalDescription::MultiplexValueRange::debugStreaming(QDebug dbg,
                                                                  const MultiplexValueRange &range)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "MultiplexValueRange(" << range.minimum << ", " << range.maximum << ")";
    return dbg;
}
#endif // QT_NO_DEBUG_STREAM

template <typename T>
static bool checkValue(const QVariant &valueVar,
                       const QCanSignalDescription::MultiplexValues &ranges)
{
    const T val = valueVar.value<T>();
    for (const auto &pair : ranges) {
        T min = pair.minimum.value<T>();
        T max = pair.maximum.value<T>();
        if (min > max)
            max = std::exchange(min, max);
        if (val >= min && val <= max)
            return true;
    }
    return false;
}

bool QCanSignalDescriptionPrivate::muxValueInRange(
        const QVariant &value, const QCanSignalDescription::MultiplexValues &ranges) const
{
    // Use the current data format to convert QVariant values.
    // Do we really need it for Float, Double and Ascii?
    switch (format) {
    case QtCanBus::DataFormat::SignedInteger:
        return checkValue<qint64>(value, ranges);
    case QtCanBus::DataFormat::UnsignedInteger:
        return checkValue<quint64>(value, ranges);
    case QtCanBus::DataFormat::Float:
        return checkValue<float>(value, ranges);
    case QtCanBus::DataFormat::Double:
        return checkValue<double>(value, ranges);
    case QtCanBus::DataFormat::AsciiString:
        return checkValue<QByteArray>(value, ranges);
    }

    Q_UNREACHABLE_RETURN(false);
}

QCanSignalDescriptionPrivate *QCanSignalDescriptionPrivate::get(const QCanSignalDescription &desc)
{
    return desc.d.data();
}

QT_END_NAMESPACE
