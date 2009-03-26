/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QPRIVATEIMPLEMENTATION_H
#define QPRIVATEIMPLEMENTATION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

/* Rationale:

QSharedDataPointer has some deficiencies when used for implementation hiding,
which are exposed by its use in the messaging library:
1. It cannot easily be used with incomplete classes, requiring client classes
   to unnecessarily define destructors, copy constructors and assignment
   operators.
2. It is not polymorphic, and must be reused redundantly where a class with
   a hidden implementation is derived from another class with a hidden
   implementation.
3. Type-bridging functions are required to provide a supertype with access
   to the supertype data allocated within a subtype object.

The QPrivateImplementation class stores a pointer to the correct destructor
and copy-constructor, given the type of the actual object instantiated.
This allows it to be copied and deleted in contexts where the true type of
the implementation object is not recorded.

The QPrivateImplementationPointer provides QSharedDataPointer semantics,
providing the pointee type with necessary derived-type information. The
pointee type must derive from QPrivateImplementation.

The QPrivatelyImplemented<> template provides correct copy and assignment
functions, and allows the shared implementation object to be cast to the
different object types in the implementation type hierarchy.

*/

#include "qtopiaglobal.h"
#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>

class QPrivateImplementationBase
{
public:
    template<typename Subclass>
    inline QPrivateImplementationBase(Subclass* p)
        : ref_count(0),
          self(p),
          delete_function(&QPrivateImplementationBase::typed_delete<Subclass>),
          copy_function(&QPrivateImplementationBase::typed_copy_construct<Subclass>)
    {
    }

    inline QPrivateImplementationBase(const QPrivateImplementationBase& other)
        : ref_count(0),
          self(other.self),
          delete_function(other.delete_function),
          copy_function(other.copy_function)
    {
    }

    inline void ref()
    {
        ref_count.ref();
    }

    inline bool deref()
    {
        if (ref_count.deref() == 0 && delete_function && self) {
            (*delete_function)(self);
            return true;
        } else  {
            return false;
        }
    }

    inline void* detach()
    {
        if (copy_function && self && ref_count != 1) {
            void* copy = (*copy_function)(self);
            reinterpret_cast<QPrivateImplementationBase*>(copy)->self = copy;
            return copy;
        } else {
            return 0;
        }
    }

private:
    QAtomicInt ref_count;

    void *self;
    void (*delete_function)(void *p);
    void *(*copy_function)(const void *p);

    template<class T>
    static inline void typed_delete(void *p)
    {
        delete static_cast<T*>(p);
    }

    template<class T>
    static inline void* typed_copy_construct(const void *p)
    {
        return new T(*static_cast<const T*>(p));
    }

    // using the assignment operator would lead to corruption in the ref-counting
    QPrivateImplementationBase &operator=(const QPrivateImplementationBase &);
};

template <class T> class QPrivateImplementationPointer
{
public:
    inline T &operator*() { return *detach(); }
    inline const T &operator*() const { return *d; }

    inline T *operator->() { return detach(); }
    inline const T *operator->() const { return d; }

    inline operator T *() { return detach(); }
    inline operator const T *() const { return d; }

    inline T *data() { return detach(); }
    inline const T *data() const { return d; }

    inline const T *constData() const { return d; }

    inline bool operator==(const QPrivateImplementationPointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const QPrivateImplementationPointer<T> &other) const { return d != other.d; }

    inline QPrivateImplementationPointer()
        : d(0)
    {
    }

    inline explicit QPrivateImplementationPointer(T *p)
        : d(p)
    {
        increment(d);
    }

    template<typename U>
    inline explicit QPrivateImplementationPointer(U *p)
        : d(static_cast<T*>(p))
    {
        increment(d);
    }

    inline QPrivateImplementationPointer(const QPrivateImplementationPointer<T> &o)
        : d(o.d)
    {
        increment(d);
    }

    /* Not necessary?
    template<typename U>
    inline QPrivateImplementationPointer(const QPrivateImplementationPointer<U> &o)
        : d(static_cast<T*>(o.d))
    {
        increment(d);
    }
    */

    inline ~QPrivateImplementationPointer()
    {
        decrement(d);
    }

    inline QPrivateImplementationPointer<T> &operator=(T *p)
    {
        assign_helper(p);
        return *this;
    }

    inline QPrivateImplementationPointer<T> &operator=(const QPrivateImplementationPointer<T> &o)
    {
        assign_helper(o.d);
        return *this;
    }

    /* Not necessary?
    template<typename U>
    inline QPrivateImplementationPointer<T> &operator=(const QPrivateImplementationPointer<U> &o)
    {
        assign_helper(o.d);
        return *this;
    }
    */

    inline bool operator!() const { return !d; }

private:
    inline void increment(T*& p)
    {
        if (p) p->ref();
    }

    inline void decrement(T*& p)
    {
        if (p) {
            if (p->deref())  {
                p = reinterpret_cast<T*>(~0);
            }
        }
    }

    inline T* assign_helper(T *p)
    {
        if (p != d) {
            increment(p);
            decrement(d);
            d = p;
        }
        return d;
    }

    inline T* detach()
    {
        if (!d) return 0;

        if (T* detached = static_cast<T*>(d->detach())) {
            return assign_helper(detached);
        } else {
            return d;
        }
    }

public:
    T *d;
};

template<typename ImplementationType>
class QTOPIAMAIL_EXPORT QPrivatelyImplemented
{
public:
    QPrivatelyImplemented(ImplementationType* p);
    QPrivatelyImplemented(const QPrivatelyImplemented& other);

    template<typename A1>
    QPrivatelyImplemented(ImplementationType* p, A1 a1);

    virtual ~QPrivatelyImplemented();

    const QPrivatelyImplemented<ImplementationType>& operator=(const QPrivatelyImplemented<ImplementationType>& other);

    template<typename ImplementationSubclass>
    ImplementationSubclass* impl();

    template<typename InterfaceType>
    typename InterfaceType::ImplementationType* impl(InterfaceType*);

    template<typename ImplementationSubclass>
    const ImplementationSubclass* impl() const;

    template<typename InterfaceType>
    const typename InterfaceType::ImplementationType* impl(const InterfaceType*) const;

    /* Comparison functions passed through to the implementation type?
    bool operator== (const QPrivatelyImplemented<ImplementationType>& other) const;
    bool operator!= (const QPrivatelyImplemented<ImplementationType>& other) const;
    bool operator< (const QPrivatelyImplemented<ImplementationType>& other) const;
    */

protected:
    QPrivateImplementationPointer<ImplementationType> d;
};


class QPrivateNoncopyableBase
{
public:
    template<typename Subclass>
    inline QPrivateNoncopyableBase(Subclass* p)
        : self(p),
          delete_function(&QPrivateNoncopyableBase::typed_delete<Subclass>)
    {
    }

    inline void delete_self()
    {
        if (delete_function && self) {
            (*delete_function)(self);
        }
    }

private:
    void *self;
    void (*delete_function)(void *p);

    template<class T>
    static inline void typed_delete(void *p)
    {
        delete static_cast<T*>(p);
    }

    // do not permit copying
    QPrivateNoncopyableBase(const QPrivateNoncopyableBase &);

    QPrivateNoncopyableBase &operator=(const QPrivateNoncopyableBase &);
};

template <class T> class QPrivateNoncopyablePointer
{
public:
    inline T &operator*() { return *d; }
    inline const T &operator*() const { return *d; }

    inline T *operator->() { return d; }
    inline const T *operator->() const { return d; }

    inline operator T *() { return d; }
    inline operator const T *() const { return d; }

    inline T *data() { return d; }
    inline const T *data() const { return d; }

    inline const T *constData() const { return d; }

    inline bool operator==(const QPrivateNoncopyablePointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const QPrivateNoncopyablePointer<T> &other) const { return d != other.d; }

    inline QPrivateNoncopyablePointer()
        : d(0)
    {
    }

    inline explicit QPrivateNoncopyablePointer(T *p)
        : d(p)
    {
    }

    template<typename U>
    inline explicit QPrivateNoncopyablePointer(U *p)
        : d(static_cast<T*>(p))
    {
    }

    inline ~QPrivateNoncopyablePointer()
    {
        d->delete_self();
    }

    inline bool operator!() const { return !d; }

private:
    inline QPrivateNoncopyablePointer<T> &operator=(T *p);

    inline QPrivateNoncopyablePointer<T> &operator=(const QPrivateNoncopyablePointer<T> &o);

    /* Not necessary?
    template<typename U>
    inline QPrivateNoncopyablePointer<T> &operator=(const QPrivateNoncopyablePointer<U> &o);
    */

public:
    T *d;
};

template<typename ImplementationType>
class QTOPIAMAIL_EXPORT QPrivatelyNoncopyable
{
public:
    QPrivatelyNoncopyable(ImplementationType* p);

    template<typename A1>
    QPrivatelyNoncopyable(ImplementationType* p, A1 a1);

    virtual ~QPrivatelyNoncopyable();

    template<typename ImplementationSubclass>
    ImplementationSubclass* impl();

    template<typename InterfaceType>
    typename InterfaceType::ImplementationType* impl(InterfaceType*);

    template<typename ImplementationSubclass>
    const ImplementationSubclass* impl() const;

    template<typename InterfaceType>
    const typename InterfaceType::ImplementationType* impl(const InterfaceType*) const;

    /* Comparison functions passed through to the implementation type?
    bool operator== (const QPrivatelyNoncopyable<ImplementationType>& other) const;
    bool operator!= (const QPrivatelyNoncopyable<ImplementationType>& other) const;
    bool operator< (const QPrivatelyNoncopyable<ImplementationType>& other) const;
    */

protected:
    QPrivateNoncopyablePointer<ImplementationType> d;
};

#endif
