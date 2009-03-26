/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QATOMIC_AVR32_H
#define QATOMIC_AVR32_H

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

#define Q_ATOMIC_INT_REFERENCE_COUNTING_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isReferenceCountingNative()
{ return true; }
inline bool QBasicAtomicInt::isReferenceCountingWaitFree()
{ return false; }

#define Q_ATOMIC_INT_TEST_AND_SET_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isTestAndSetNative()
{ return true; }
inline bool QBasicAtomicInt::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_INT_FETCH_AND_STORE_IS_WAIT_FREE

inline bool QBasicAtomicInt::isFetchAndStoreNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndStoreWaitFree()
{ return true; }

#define Q_ATOMIC_INT_FETCH_AND_ADD_IS_ALWAYS_NATIVE

inline bool QBasicAtomicInt::isFetchAndAddNative()
{ return true; }
inline bool QBasicAtomicInt::isFetchAndAddWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_TEST_AND_SET_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isTestAndSetWaitFree()
{ return false; }

#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_ALWAYS_NATIVE
#define Q_ATOMIC_POINTER_FETCH_AND_STORE_IS_WAIT_FREE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndStoreWaitFree()
{ return true; }

#define Q_ATOMIC_POINTER_FETCH_AND_ADD_IS_ALWAYS_NATIVE

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddNative()
{ return true; }
template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::isFetchAndAddWaitFree()
{ return false; }

#if defined(QT_BUILD_QREGION_CPP)
#warning "workaround!"
__attribute__((noinline))
#else
inline
#endif
bool QBasicAtomicInt::ref()
{
    register int newValue;
    asm volatile("0:\n"
                 "ssrf %[LOCK]\n"
                 "ld.w %[newValue], %[_q_value]\n"
                 "add %[newValue], %[ONE]\n"
                 "stcond %[_q_value], %[newValue]\n"
                 "brne 0b\n"
                 : [newValue] "=&r"(newValue),
                   [_q_value] "+RKs16"(_q_value)
                 : [LOCK] "i"(0x5),
                   [ONE] "r"(1)
                 : "cc", "memory");
    return newValue;
}

inline bool QBasicAtomicInt::deref()
{
    register int newValue;
    asm volatile("0:\n"
                 "ssrf %[LOCK]\n"
                 "ld.w %[newValue], %[_q_value]\n"
                 "sub %[newValue], %[ONE]\n"
                 "stcond %[_q_value], %[newValue]\n"
                 "brne 0b\n"
                 : [newValue] "=&r"(newValue),
                   [_q_value] "+RKs16"(_q_value)
                 : [LOCK] "i"(0x5),
                   [ONE] "i"(1)
                 : "cc", "memory");
    return newValue;
}

inline bool QBasicAtomicInt::testAndSetOrdered(int expectedValue, int newValue)
{
    register int originalValue;
    asm volatile("0:\n"
                 "ssrf %[LOCK]\n"
                 "ld.w %[originalValue], %[_q_value]\n"
                 "cp.w %[originalValue], %[expectedValue]\n"
                 "brne 0f\n"
                 "stcond %[_q_value], %[newValue]\n"
                 "brne 0b\n"
                 "0:\n"
                 : [originalValue] "=&r"(originalValue),
                   [_q_value] "+RKs16"(_q_value)
                 : [LOCK] "i"(0x5),
                   [expectedValue] "r"(expectedValue),
                   [newValue] "r"(newValue)
                 : "cc", "memory");
    return originalValue == expectedValue;
}

inline bool QBasicAtomicInt::testAndSetRelaxed(int expectedValue, int newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

inline bool QBasicAtomicInt::testAndSetAcquire(int expectedValue, int newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

inline bool QBasicAtomicInt::testAndSetRelease(int expectedValue, int newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

inline int QBasicAtomicInt::fetchAndStoreOrdered(int newValue)
{
    register int originalValue;
    asm volatile("xchg %[originalValue], %[_q_value], %[newValue]"
                 : [originalValue] "=&r"(originalValue),
                   "+m" (_q_value)
                 : [_q_value] "r"(&_q_value),
                   [newValue] "r"(newValue)
                 : "cc", "memory");
    return originalValue;
}

inline int QBasicAtomicInt::fetchAndStoreRelaxed(int newValue)
{
    return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndStoreAcquire(int newValue)
{
    return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndStoreRelease(int newValue)
{
    return fetchAndStoreOrdered(newValue);
}

inline int QBasicAtomicInt::fetchAndAddOrdered(int valueToAdd)
{
    register int originalValue, newValue;
    asm volatile("0:\n"
                 "ssrf %[LOCK]\n"
                 "ld.w %[originalValue], %[_q_value]\n"
                 "add %[newValue], %[originalValue], %[valueToAdd]\n"
                 "stcond %[_q_value], %[newValue]\n"
                 "brne 0b\n"
                 : [originalValue] "=&r"(originalValue),
                   [newValue] "=&r"(newValue),
                   [_q_value] "+RKs16"(_q_value)
                 : [LOCK] "i"(0x5),
                   [valueToAdd] "r"(valueToAdd)
                 : "cc", "memory");
    return originalValue;
}

inline int QBasicAtomicInt::fetchAndAddRelaxed(int valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddAcquire(int valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

inline int QBasicAtomicInt::fetchAndAddRelease(int valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetOrdered(T *expectedValue, T *newValue)
{
    register T *originalValue;
    asm volatile("0:\n"
                 "ssrf %[LOCK]\n"
                 "ld.w %[originalValue], %[_q_value]\n"
                 "cp.w %[originalValue], %[expectedValue]\n"
                 "brne 0f\n"
                 "stcond %[_q_value], %[newValue]\n"
                 "brne 0b\n"
                 "0:\n"
                 : [originalValue] "=&r"(originalValue),
                   [_q_value] "+RKs16"(_q_value)
                 : [LOCK] "i"(0x5),
                   [expectedValue] "r"(expectedValue),
                   [newValue] "r"(newValue)
                 : "cc", "memory");
    return originalValue == expectedValue;
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelaxed(T *expectedValue, T *newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetAcquire(T *expectedValue, T *newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE bool QBasicAtomicPointer<T>::testAndSetRelease(T *expectedValue, T *newValue)
{
    return testAndSetOrdered(expectedValue, newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreOrdered(T *newValue)
{
    register T *originalValue;
    asm volatile("xchg %[originalValue], %[_q_value], %[newValue]"
                 : [originalValue] "=&r"(originalValue),
                   "+m" (_q_value)
                 : [_q_value] "r"(&_q_value),
                   [newValue] "r"(newValue)
                 : "cc", "memory");
    return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelaxed(T *newValue)
{
    return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreAcquire(T *newValue)
{
    return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndStoreRelease(T *newValue)
{
    return fetchAndStoreOrdered(newValue);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddOrdered(qptrdiff valueToAdd)
{
    register T *originalValue, *newValue;
    asm volatile("0:\n"
                 "ssrf %[LOCK]\n"
                 "ld.w %[originalValue], %[_q_value]\n"
                 "add %[newValue], %[originalValue], %[valueToAdd]\n"
                 "stcond %[_q_value], %[newValue]\n"
                 "brne 0b\n"
                 : [originalValue] "=&r"(originalValue),
                   [newValue] "=&r"(newValue),
                   [_q_value] "+RKs16"(_q_value)
                 : [LOCK] "i"(0x5),
                   [valueToAdd] "r"(valueToAdd * sizeof(T))
                 : "cc", "memory");
    return originalValue;
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelaxed(qptrdiff valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddAcquire(qptrdiff valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

template <typename T>
Q_INLINE_TEMPLATE T *QBasicAtomicPointer<T>::fetchAndAddRelease(qptrdiff valueToAdd)
{
    return fetchAndAddOrdered(valueToAdd);
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QATOMIC_AVR32_H
