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

#ifndef INHERITEDSHAREDDATA_H
#define INHERITEDSHAREDDATA_H

#include <QSharedData>

template <class T> class InheritedSharedDataPointer
{
public:
    inline void detach() { if (d && d->ref != 1) detach_helper(); }
    inline T &operator*() { detach(); return *d; }
    inline const T &operator*() const { return *d; }
    inline T *operator->() { detach(); return d; }
    inline const T *operator->() const { return d; }
    inline operator T *() { detach(); return d; }
    inline operator const T *() const { return d; }
    inline T *data() { detach(); return d; }
    inline const T *data() const { return d; }
    inline const T *constData() const { return d; }

    inline bool operator==(const InheritedSharedDataPointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const InheritedSharedDataPointer<T> &other) const { return d != other.d; }

    inline InheritedSharedDataPointer() { d = 0; }
    inline ~InheritedSharedDataPointer() { if (d && !d->ref.deref()) delete d; }

    explicit InheritedSharedDataPointer(T *data);
    inline InheritedSharedDataPointer(const InheritedSharedDataPointer<T> &o) : d(o.d) { if (d) d->ref.ref(); }
    inline InheritedSharedDataPointer<T> & operator=(const InheritedSharedDataPointer<T> &o) {
        if (o.d != d) {
            if (o.d)
                o.d->ref.ref();
            if (d && !d->ref.deref())
                delete d;
            d = o.d;
        }
        return *this;
    }
    inline InheritedSharedDataPointer &operator=(T *o) {
        if (o != d) {
            if (o)
                o->ref.ref();
            if (d && !d->ref.deref())
                delete d;
            d = o;
        }
        return *this;
    }

    inline bool operator!() const { return !d; }

private:
    void detach_helper();

    T *d;
};

template <class T>
Q_INLINE_TEMPLATE InheritedSharedDataPointer<T>::InheritedSharedDataPointer(T *adata) : d(adata)
{ if (d) d->ref.ref(); }

template <class T>
Q_OUTOFLINE_TEMPLATE void InheritedSharedDataPointer<T>::detach_helper()
{
    T *x = d->createCopy();
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

#endif
