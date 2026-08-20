// Copyright (C) 2020 Mikhail Svetkin <mikhail.svetkin@gmail.com>
// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QHTTPSERVERROUTERVIEWTRAITS_H
#define QHTTPSERVERROUTERVIEWTRAITS_H

#include <QtHttpServer/qhttpserverviewtraits_impl.h>

QT_BEGIN_NAMESPACE

class QHttpServerRequest;
class QHttpServerResponder;

namespace QtPrivate {

template<typename ViewHandler, bool DisableStaticAssert>
struct RouterViewTraitsHelper : ViewTraits<ViewHandler, DisableStaticAssert> {
    using VTraits = ViewTraits<ViewHandler, DisableStaticAssert>;
    using FunctionTraits = typename VTraits::FTraits;

    template<int I>
    struct ArgumentChecker : FunctionTraits::template Arg<I> {
        using IsRequestCLvalue = typename VTraits::template Special<I, const QHttpServerRequest &>;
        using IsRequestValue = typename VTraits::template Special<I, QHttpServerRequest>;
        using IsRequest = CheckAny<IsRequestCLvalue, IsRequestValue>;
        static_assert(IsRequest::StaticAssert,
                      "ViewHandler arguments error: "
                      "QHttpServerRequest can only be passed by value or as a const Lvalue");

        using IsResponderLvalue = typename VTraits::template Special<I, QHttpServerResponder &>;
        using IsResponderRvalue = typename VTraits::template Special<I, QHttpServerResponder &&>;
        using IsResponder = CheckAny<IsResponderLvalue, IsResponderRvalue>;
        static_assert(IsResponder::StaticAssert,
                      "ViewHandler arguments error: "
                      "QHttpServerResponder can only be passed as a reference or Rvalue "
                      "reference");

        using IsSpecial = CheckAny<IsRequest, IsResponder>;

        struct IsSimple {
            static constexpr bool TypeMatched = !IsSpecial::TypeMatched &&
                                           I < FunctionTraits::ArgumentCount &&
                                           FunctionTraits::ArgumentIndexMax != -1;
            static constexpr bool Value =
                    !IsSpecial::Value && FunctionTraits::template Arg<I>::CopyConstructible;

            static constexpr bool StaticAssert =
                DisableStaticAssert || Value || !TypeMatched;


            static_assert(StaticAssert,
                          "ViewHandler arguments error: "
                          "Type is not copy constructible");
        };

        using CheckOk = CheckAny<IsSimple, IsSpecial>;

        static constexpr bool Value = CheckOk::Value;
        static constexpr bool StaticAssert = CheckOk::StaticAssert;
    };


    struct Arguments {
        template<size_t ... I>
        struct ArgumentsReturn {
            template<int Idx>
            using Arg = ArgumentChecker<Idx>;

            template<int Idx>
            static constexpr QMetaType metaType() noexcept
            {
                using Type = typename FunctionTraits::template Arg<Idx>::CleanType;
                constexpr bool Simple = Arg<Idx>::IsSimple::Value;

                if constexpr (Simple && std::is_copy_assignable_v<Type>)
                    return QMetaType::fromType<Type>();
                else
                    return QMetaType::fromType<void>();
            }

            static constexpr std::size_t Count = FunctionTraits::ArgumentCount;
            static constexpr std::size_t CapturableCount =
                    (0 + ... + static_cast<std::size_t>(Arg<I>::IsSimple::Value));

            static constexpr std::size_t SpecialsCount = Count - CapturableCount;

            static constexpr bool Value = (Arg<I>::Value && ...);
            static constexpr bool StaticAssert = (Arg<I>::StaticAssert && ...);

            using Indexes = std::index_sequence<I...>;

            using CapturableIndexes = std::make_index_sequence<CapturableCount>;

            using SpecialIndexes = std::make_index_sequence<SpecialsCount>;

            using Last = Arg<FunctionTraits::ArgumentIndexMax>;
            using SecondLast = Arg<FunctionTraits::ArgumentIndexMax - 1>;
        };

        template<size_t ... I>
        static constexpr ArgumentsReturn<I...> eval(std::index_sequence<I...>) noexcept;
    };

    template<int CaptureOffset>
    struct BindType {
        template<typename ... Args>
        struct FunctionWrapper {
            using Type = std::function<typename FunctionTraits::ReturnType (Args...)>;
        };

        template<int Id>
        using OffsetArg = typename FunctionTraits::template Arg<CaptureOffset + Id>::Type;

        template<size_t ... Idx>
        static constexpr typename FunctionWrapper<OffsetArg<Idx>...>::Type
                eval(std::index_sequence<Idx...>) noexcept;
    };
};


} // namespace QtPrivate

template <typename ViewHandler, bool DisableStaticAssert = false>
struct QHttpServerRouterViewTraits
{
    using Helpers = typename QtPrivate::RouterViewTraitsHelper<ViewHandler, DisableStaticAssert>;
    using ReturnType = typename Helpers::FunctionTraits::ReturnType;
    using Arguments = decltype(Helpers::Arguments::eval(typename Helpers::ArgumentIndexes{}));
    using BindableType = decltype(Helpers::template BindType<Arguments::CapturableCount>::eval(
            typename Arguments::SpecialIndexes{}));
};


QT_END_NAMESPACE

#endif  // QHTTPSERVERROUTERVIEWTRAITS_H
