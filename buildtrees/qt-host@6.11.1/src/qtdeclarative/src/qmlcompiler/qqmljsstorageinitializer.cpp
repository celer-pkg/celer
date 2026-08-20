// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
// Qt-Security score:significant

#include "qqmljsstorageinitializer_p.h"

QT_BEGIN_NAMESPACE

/*!
 * \internal
 * \class QQmlJSStorageInitializer
 *
 * The QQmlJSStorageInitializer is a compile pass that initializes the storage
 * for all register contents.
 *
 * QQmlJSStorageInitializer does not have to use the byte code at all but
 * operates only on the annotations and the function description.
 */

QQmlJSCompilePass::BlocksAndAnnotations QQmlJSStorageInitializer::run(Function *function)
{
    m_function = function;

    if (QQmlJSRegisterContent &returnType = function->returnType; returnType.isValid()) {
        if (const QQmlJSScope::ConstPtr stored
                = m_typeResolver->storedType(returnType.containedType())) {
            m_pool->storeType(returnType, stored);
        } else {
            addError(QStringLiteral("Cannot store the return type %1.")
                             .arg(returnType.containedType()->internalName()));
            return {};
        }
    }

    const auto storeRegister = [&](QQmlJSRegisterContent &content) {
        if (!content.isValid() || !content.storage().isNull())
            return;

        const QQmlJSScope::ConstPtr original = m_typeResolver->originalContainedType(content);
        if (const QQmlJSScope::ConstPtr originalStored = m_typeResolver->storedType(original)) {
            m_pool->storeType(content, originalStored);
        } else {
            addError(QStringLiteral("Cannot store type %1.").arg(original->internalName()));
            return;
        }

        const QQmlJSScope::ConstPtr contentContained = content.containedType();
        const QQmlJSScope::ConstPtr adjustedStored = m_typeResolver->storedType(contentContained);
        if (!adjustedStored) {
            addError(QStringLiteral("Cannot store type %1.")
                             .arg(contentContained->internalName()));
            return;
        }

        if (!m_typeResolver->adjustTrackedType(content.storage(), adjustedStored)) {
            addError(QStringLiteral("Cannot adjust stored type for %1 to %2.")
                             .arg(contentContained->internalName(), adjustedStored->internalName()));
        }
    };

    const auto storeRegisters = [&](VirtualRegisters &registers) {
        for (auto j = registers.begin(), jEnd = registers.end(); j != jEnd; ++j)
            storeRegister(j.value().content);
    };

    storeRegister(function->qmlScope);

    for (QQmlJSRegisterContent &argument : function->argumentTypes) {
        Q_ASSERT(argument.isValid());
        storeRegister(argument);
    }

    for (QQmlJSRegisterContent &argument : function->registerTypes) {
        Q_ASSERT(argument.isValid());
        storeRegister(argument);
    }

    for (auto i = m_annotations.begin(), iEnd = m_annotations.end(); i != iEnd; ++i) {
        storeRegister(i->second.changedRegister);
        storeRegisters(i->second.typeConversions);
        storeRegisters(i->second.readRegisters);
    }

    return { std::move(m_basicBlocks), std::move(m_annotations) };
}

QT_END_NAMESPACE
