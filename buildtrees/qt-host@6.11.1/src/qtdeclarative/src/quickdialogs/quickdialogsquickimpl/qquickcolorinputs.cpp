// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickcolorinputs_p.h"

#include <functional>

#include <QtCore/qloggingcategory.h>
#include <QtCore/QRegularExpression>
#include <QtGui/QValidator>
#include <QtQuickTemplates2/private/qquickcontainer_p_p.h>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcColorInputs, "qt.quick.dialogs.colorinputs")

class QQuickColorInputsPrivate : public QQuickContainerPrivate
{
public:
    Q_DECLARE_PUBLIC(QQuickColorInputs)

    void repopulate();
    QQuickTextInput *createDelegateTextInputItem(QQmlComponent *component, const QVariantMap &initialProperties);
    void handleHexInput();
    void handleRedInput();
    void handleGreenInput();
    void handleBlueInput();
    void handleHueInput();
    void handleHsvSaturationInput();
    void handleValueInput();
    void handleHslSaturationInput();
    void handleLightnessInput();
    void handleAlphaInput();

    QQmlComponent *m_delegate = nullptr;
    QQuickColorInputs::Mode m_currentMode = QQuickColorInputs::Hex;
    HSVA m_hsva;
    bool m_showAlpha = false;
    bool m_repopulating = false;
};

QQuickColorInputs::QQuickColorInputs(QQuickItem *parent)
    : QQuickContainer(*(new QQuickColorInputsPrivate), parent)
{ }

QColor QQuickColorInputs::color() const
{
    Q_D(const QQuickColorInputs);
    return QColor::fromHsvF(d->m_hsva.h, d->m_hsva.s, d->m_hsva.v, d->m_hsva.a);
}

void QQuickColorInputs::setColor(const QColor &c)
{
    Q_D(QQuickColorInputs);
    if (color().rgba() == c.rgba())
        return;

    // If we get a QColor from an Hsv or Hsl color system,
    // we want to get the raw values without the risk of QColor converting them,
    // and possible deleting relevant information for achromatic cases.
    if (c.spec() == QColor::Spec::Hsl) {
        const auto sv = getSaturationAndValue(c.hslSaturationF(), c.lightnessF());
        d->m_hsva.h = qBound(.0, c.hslHueF(), 1.0);
        d->m_hsva.s = qBound(.0, sv.first, 1.0);
        d->m_hsva.v = qBound(.0, sv.second, 1.0);
    } else {
        d->m_hsva.h = qBound(.0, c.hsvHueF(), 1.0);
        d->m_hsva.s = qBound(.0, c.hsvSaturationF(), 1.0);
        d->m_hsva.v = qBound(.0, c.valueF(), 1.0);
    }

    d->m_hsva.a = c.alphaF();

    emit colorChanged(color());
}

int QQuickColorInputs::red() const
{
    return color().red();
}

int QQuickColorInputs::green() const
{
    return color().green();
}

int QQuickColorInputs::blue() const
{
    return color().blue();
}

qreal QQuickColorInputs::alpha() const
{
    Q_D(const QQuickColorInputs);
    return d->m_hsva.a;
}

qreal QQuickColorInputs::hue() const
{
    Q_D(const QQuickColorInputs);
    return d->m_hsva.h;
}

qreal QQuickColorInputs::hslSaturation() const
{
    Q_D(const QQuickColorInputs);
    return getSaturationAndLightness(d->m_hsva.s, d->m_hsva.v).first;
}

qreal QQuickColorInputs::hsvSaturation() const
{
    Q_D(const QQuickColorInputs);
    return d->m_hsva.s;
}

qreal QQuickColorInputs::value() const
{
    Q_D(const QQuickColorInputs);
    return d->m_hsva.v;
}

qreal QQuickColorInputs::lightness() const
{
    Q_D(const QQuickColorInputs);
    return getSaturationAndLightness(d->m_hsva.s, d->m_hsva.v).second;
}

bool QQuickColorInputs::showAlpha() const
{
    Q_D(const QQuickColorInputs);
    return d->m_showAlpha;
}

void QQuickColorInputs::setShowAlpha(bool showAlpha)
{
    Q_D(QQuickColorInputs);
    if (d->m_showAlpha == showAlpha)
        return;

    d->m_showAlpha = showAlpha;
    d->repopulate();

    emit showAlphaChanged(d->m_showAlpha);
}

QQuickColorInputs::Mode QQuickColorInputs::currentMode() const
{
    Q_D(const QQuickColorInputs);
    return d->m_currentMode;
}

void QQuickColorInputs::setCurrentMode(Mode mode)
{
    Q_D(QQuickColorInputs);
    if (d->m_currentMode == mode)
        return;

    d->m_currentMode = mode;
    d->repopulate();

    emit currentModeChanged();
}

QQmlComponent *QQuickColorInputs::delegate() const
{
    Q_D(const QQuickColorInputs);
    return d->m_delegate;
}

void QQuickColorInputs::setDelegate(QQmlComponent *delegate)
{
    Q_D(QQuickColorInputs);
    if (d->m_delegate == delegate)
        return;
    d->m_delegate = delegate;
    emit delegateChanged();
}


void QQuickColorInputs::componentComplete()
{
    Q_D(QQuickColorInputs);
    QQuickContainer::componentComplete();
    d->repopulate();
}

QQuickTextInput *QQuickColorInputsPrivate::createDelegateTextInputItem(QQmlComponent *component, const QVariantMap &initialProperties)
{
    Q_Q(QQuickColorInputs);
    QQmlContext *context = component->creationContext();
    if (!context)
        context = qmlContext(q);

    if (!component->isBound() && initialProperties.isEmpty()) {
        context = new QQmlContext(context, q);
        context->setContextObject(q);
    }

    QQuickTextInput *textInput = qobject_cast<QQuickTextInput*>(component->createWithInitialProperties(initialProperties, context));
    if (textInput)
        QQml_setParent_noEvent(textInput, q);
    return textInput;
}

static const QString s_percentage_pattern = QString::fromUtf8("^(\\d+)%?$");
static const QString s_degree_pattern = QString::fromUtf8("(\\d+)°?$");
static const QString s_rgba_pattern = QString::fromUtf8("^#[0-9A-f]{6}(?:[0-9A-f]{2})?$");
static const QString s_rgb_pattern = QString::fromUtf8("^#[0-9A-f]{6}$");

void QQuickColorInputsPrivate::repopulate()
{
    Q_Q(QQuickColorInputs);

    if (m_repopulating)
        return;

    if (!q->delegate() || !q->contentItem()) {
        qmlWarning(q) << "Both delegate and contentItem must be set before repopulating";
        return;
    }

    QScopedValueRollback<bool> repopulateGuard(m_repopulating, true);

    auto removeAllItems = [q](){
        while (q->count() > 0)
            q->removeItem(q->itemAt(0));
    };

    removeAllItems();

    static const QRegularExpressionValidator rgba_validator = QRegularExpressionValidator(QRegularExpression(s_rgba_pattern));
    static const QRegularExpressionValidator rgb_validator = QRegularExpressionValidator(QRegularExpression(s_rgb_pattern));
    static const QRegularExpressionValidator percentage_validator = QRegularExpressionValidator(QRegularExpression(s_percentage_pattern));
    static const QRegularExpressionValidator degree_validator = QRegularExpressionValidator(QRegularExpression(s_degree_pattern));
    static const QIntValidator intValdator = QIntValidator(0, 255);

    auto addInputField = [this, q, removeAllItems](const QString &name, const QValidator *validator,
                                                   void (QQuickColorInputsPrivate::*handler)(),
                                                   std::function<QString()> textConverter) {
        const int maxLen = m_currentMode == QQuickColorInputs::Hex ? 9 : 4;
        const QVariantMap properties = {
            { QStringLiteral("objectName"), QVariant::fromValue(name) },
            { QStringLiteral("validator"), QVariant::fromValue(validator) },
            { QStringLiteral("horizontalAlignment"), QVariant::fromValue(
                                                             m_currentMode == QQuickColorInputs::Hex ? Qt::AlignLeft : Qt::AlignHCenter) },
            { QStringLiteral("maximumLength"), QVariant::fromValue(maxLen) },
            { QStringLiteral("text"), QVariant::fromValue(textConverter()) }
        };
        if (QQuickTextInput *item = createDelegateTextInputItem(q->delegate(), properties)) {
            connect(item, &QQuickTextInput::editingFinished, this, handler);
            QObject::connect(q, &QQuickColorInputs::colorChanged, item, [item, textConverter](const QColor &){ item->setText(textConverter()); });

            insertItem(q->count(), item);
        } else {
            qCWarning(lcColorInputs) << "Failed to create delegate for " << name;
            removeAllItems();
        }
    };

    switch (m_currentMode) {
    case QQuickColorInputs::Hex:
        addInputField(QStringLiteral("hex"), m_showAlpha ? &rgba_validator : &rgb_validator, &QQuickColorInputsPrivate::handleHexInput,
                      [q](){ return q->color().name(); });
        break;
    case QQuickColorInputs::Rgb:
        addInputField(QStringLiteral("red"), &intValdator, &QQuickColorInputsPrivate::handleRedInput, [q](){ return QString::number(q->red()); });
        addInputField(QStringLiteral("green"), &intValdator, &QQuickColorInputsPrivate::handleGreenInput, [q](){ return QString::number(q->green()); });
        addInputField(QStringLiteral("blue"), &intValdator, &QQuickColorInputsPrivate::handleBlueInput, [q](){ return QString::number(q->blue()); });
        if (m_showAlpha)
            addInputField(QStringLiteral("alpha"), &percentage_validator, &QQuickColorInputsPrivate::handleAlphaInput,
                          [q](){ return QString::number(qRound(q->alpha() * 100)).append(QStringLiteral("%")); });
        break;
    case QQuickColorInputs::Hsv:
        addInputField(QStringLiteral("hsvHue"), &degree_validator, &QQuickColorInputsPrivate::handleHueInput,
                      [q](){ return QString::number(qRound(q->hue() * 360)).append(QStringLiteral("°")); });
        addInputField(QStringLiteral("hsvSaturation"), &percentage_validator, &QQuickColorInputsPrivate::handleHsvSaturationInput,
                      [q](){ return QString::number(qRound(q->hsvSaturation() * 100)).append(QStringLiteral("%")); });
        addInputField(QStringLiteral("value"), &percentage_validator, &QQuickColorInputsPrivate::handleValueInput,
                      [q](){ return QString::number(qRound(q->value() * 100)).append(QStringLiteral("%")); });
        if (m_showAlpha)
            addInputField(QStringLiteral("alpha"), &percentage_validator, &QQuickColorInputsPrivate::handleAlphaInput,
                          [q](){ return QString::number(qRound(q->alpha() * 100)).append(QStringLiteral("%")); });
        break;
    case QQuickColorInputs::Hsl:
        addInputField(QStringLiteral("hslHue"), &degree_validator, &QQuickColorInputsPrivate::handleHueInput,
                      [q](){ return QString::number(qRound(q->hue() * 360)).append(QStringLiteral("°")); });
        addInputField(QStringLiteral("hslSaturation"), &percentage_validator, &QQuickColorInputsPrivate::handleHslSaturationInput,
                      [q](){ return QString::number(qRound(q->hslSaturation() * 100)).append(QStringLiteral("%")); });
        addInputField(QStringLiteral("lightness"), &percentage_validator, &QQuickColorInputsPrivate::handleLightnessInput,
                      [q](){ return QString::number(qRound(q->lightness() * 100)).append(QStringLiteral("%")); });
        if (m_showAlpha)
            addInputField(QStringLiteral("alpha"), &percentage_validator, &QQuickColorInputsPrivate::handleAlphaInput,
                          [q](){ return QString::number(qRound(q->alpha() * 100)).append(QStringLiteral("%")); });
        break;
    default:
        qCDebug(lcColorInputs) << "Unrecognised mode " << m_currentMode;
        break;
    }

    updateImplicitContentSize();
}

void QQuickColorInputsPrivate::handleHexInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender()))
        emit q->colorModified(QColor::fromString(textInput->text()));
}

void QQuickColorInputsPrivate::handleRedInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        QColor c = q->color();
        c.setRed(textInput->text().toInt());
        emit q->colorModified(c);
    }
}

void QQuickColorInputsPrivate::handleGreenInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        QColor c = q->color();
        c.setGreen(textInput->text().toInt());
        emit q->colorModified(c);
    }
}

void QQuickColorInputsPrivate::handleBlueInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        QColor c = q->color();
        c.setBlue(textInput->text().toInt());
        emit q->colorModified(c);
    }
}

void QQuickColorInputsPrivate::handleHueInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        static const QRegularExpression pattern(s_degree_pattern);
        const auto match = pattern.match(textInput->text());
        if (match.hasMatch()) {
            const auto substr = match.captured(1);
            const qreal input = static_cast<qreal>(qBound(0, substr.toInt(), 360)) / static_cast<qreal>(360);
            const QColor c = m_currentMode == QQuickColorInputs::Hsl ? QColor::fromHslF(input, q->hslSaturation(), q->lightness(), q->alpha())
                                                               : QColor::fromHsvF(input, q->hsvSaturation(), q->value(), q->alpha());
            emit q->colorModified(c);
        }
    }
}

void QQuickColorInputsPrivate::handleHsvSaturationInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        static const QRegularExpression pattern(s_percentage_pattern);
        const auto match = pattern.match(textInput->text());
        if (match.hasMatch()) {
            const auto substr = match.captured(1);
            const qreal input = static_cast<qreal>(qBound(0, substr.toInt(), 100)) / static_cast<qreal>(100);
            emit q->colorModified(QColor::fromHsvF(q->hue(), input, q->value(), q->alpha()));
        }
    }
}

void QQuickColorInputsPrivate::handleValueInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        static const QRegularExpression pattern(s_percentage_pattern);
        const auto match = pattern.match(textInput->text());
        if (match.hasMatch()) {
            const auto substr = match.captured(1);
            const qreal input = static_cast<qreal>(qBound(0, substr.toInt(), 100)) / static_cast<qreal>(100);
            emit q->colorModified(QColor::fromHsvF(q->hue(), q->hsvSaturation(), input, q->alpha()));
        }
    }
}

void QQuickColorInputsPrivate::handleHslSaturationInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        static const QRegularExpression pattern(s_percentage_pattern);
        const auto match = pattern.match(textInput->text());
        if (match.hasMatch()) {
            const auto substr = match.captured(1);
            const qreal input = static_cast<qreal>(qBound(0, substr.toInt(), 100)) / static_cast<qreal>(100);
            emit q->colorModified(QColor::fromHslF(q->hue(), input, q->lightness(), q->alpha()));
        }
    }
}

void QQuickColorInputsPrivate::handleLightnessInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        static const QRegularExpression pattern(s_percentage_pattern);
        const auto match = pattern.match(textInput->text());
        if (match.hasMatch()) {
            const auto substr = match.captured(1);
            const qreal input = static_cast<qreal>(qBound(0, substr.toInt(), 100)) / static_cast<qreal>(100);
            emit q->colorModified(QColor::fromHslF(q->hue(), q->hslSaturation(), input, q->alpha()));
        }
    }
}

void QQuickColorInputsPrivate::handleAlphaInput()
{
    Q_Q(QQuickColorInputs);
    if (const auto textInput = qobject_cast<QQuickTextInput *>(q->QObject::sender())) {
        static const QRegularExpression pattern(s_percentage_pattern);
        const auto match = pattern.match(textInput->text());
        if (match.hasMatch()) {
            QColor c = q->color();
            const auto substr = match.captured(1);
            const qreal input = static_cast<qreal>(qBound(0, substr.toInt(), 100)) / static_cast<qreal>(100);
            c.setAlphaF(input);
            emit q->colorModified(c);
        }
    }
}

QT_END_NAMESPACE

#include "moc_qquickcolorinputs_p.cpp"
