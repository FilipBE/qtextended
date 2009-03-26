/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3PTRQUEUE_H
#define Q3PTRQUEUE_H

#include <Qt3Support/q3glist.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

template<class type>
class Q3PtrQueue : protected Q3GList
{
public:
    Q3PtrQueue()				{}
    Q3PtrQueue( const Q3PtrQueue<type> &q ) : Q3GList(q) {}
    ~Q3PtrQueue()			{ clear(); }
    Q3PtrQueue<type>& operator=(const Q3PtrQueue<type> &q)
			{ return (Q3PtrQueue<type>&)Q3GList::operator=(q); }
    bool  autoDelete() const		{ return Q3PtrCollection::autoDelete(); }
    void  setAutoDelete( bool del )	{ Q3PtrCollection::setAutoDelete(del); }
    uint  count()   const		{ return Q3GList::count(); }
    bool  isEmpty() const		{ return Q3GList::count() == 0; }
    void  enqueue( const type *d )	{ Q3GList::append(Item(d)); }
    type *dequeue()			{ return (type *)Q3GList::takeFirst();}
    bool  remove()			{ return Q3GList::removeFirst(); }
    void  clear()			{ Q3GList::clear(); }
    type *head()    const		{ return (type *)Q3GList::cfirst(); }
	  operator type *() const	{ return (type *)Q3GList::cfirst(); }
    type *current() const		{ return (type *)Q3GList::cfirst(); }

#ifdef qdoc
protected:
    virtual QDataStream& read( QDataStream&, Q3PtrCollection::Item& );
    virtual QDataStream& write( QDataStream&, Q3PtrCollection::Item ) const;
#endif

private:
    void  deleteItem( Item d );
};

#if !defined(Q_BROKEN_TEMPLATE_SPECIALIZATION)
template<> inline void Q3PtrQueue<void>::deleteItem( Q3PtrCollection::Item )
{
}
#endif

template<class type> inline void Q3PtrQueue<type>::deleteItem( Q3PtrCollection::Item d )
{
    if ( del_item ) delete (type *)d;
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3PTRQUEUE_H
