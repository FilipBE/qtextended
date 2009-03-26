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
#ifndef SP_H
#define SP_H

/*
    Smart Pointer class.

    This is a rather primitive class. Really, it's just something created
    to manage deleting pointers at the end of the scope they're obtained in.
*/
template <class T>
class SP
{
public:
    // SP<T> foo;
    inline SP()
        : o(0) {}

    // SP<T> foo(bar);
    // SP<T> foo = bar;
    inline SP(T *p)
        : o(p) {}

    // Stack-based cleanup (delete the pointer)
    // You should assign 0 if you don't want the pointer deleted
    inline ~SP()
        { delete o; }

    // foo = bar;
    inline SP<T> &operator=(T *p)
        { o = p; return *this; }

    // foo->bar();
    inline T *operator->() const
        { return o; }

    // if ( !foo )
    inline bool operator!() const
        { return (o == 0); }

    // if ( foo ) // SAFE
private:
   class Tester
   {
      void operator delete(void*);
   };
public:
   operator Tester*() const
   {
      if (!o) return 0;
      static Tester test;
      return &test;
   }

private:
   T *o;
};

#endif
