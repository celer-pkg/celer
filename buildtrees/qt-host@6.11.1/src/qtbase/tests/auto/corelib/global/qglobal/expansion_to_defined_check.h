// Copyright (C) 2025 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Giuseppe D'Angelo <giuseppe.dangelo@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_QGLOBAL_EXPANSION_TO_DEFINED_CHECK_H
#define TST_QGLOBAL_EXPANSION_TO_DEFINED_CHECK_H

// Check that expansion to defined works, despite being formally UB in C.
// In fact, there is implementation divergence in object-like macros that
// expand to defined. However, function-like macros seem to have the same
// behavior, so just test that.
#define QGLOBAL_TEST_HAS_FOO 0
#define QGLOBAL_TEST_HAS_BAR 1
#define QGLOBAL_TEST_HAS_BAZ 1

#define QGLOBAL_TEST_HAS(X) (defined QGLOBAL_TEST_HAS_##X && QGLOBAL_TEST_HAS_##X)

#if QGLOBAL_TEST_HAS(FOO)
# error FOO is 0, so this branch should not have been taken
#endif

#if !QGLOBAL_TEST_HAS(BAR)
# error BAR is set to 1, so this branch should not have been taken
#endif

#if !QGLOBAL_TEST_HAS(BAZ)
# error BAZ is set to 1, so this branch should not have been taken
#endif

#if QGLOBAL_TEST_HAS(FIE)
# error FIE is not defined, so this branch should not have been taken
#endif

#undef QGLOBAL_TEST_HAS_BAZ

#if QGLOBAL_TEST_HAS(BAZ)
# error BAZ is unset now, so this branch should not have been taken
#endif

#define QGLOBAL_TEST_HAS_BAZ 0

#if QGLOBAL_TEST_HAS(BAZ)
# error BAZ is set to 0 now, so this branch should not have been taken
#endif

#endif // TST_QGLOBAL_EXPANSION_TO_DEFINED_CHECK_H
