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

#ifndef HELIXUTIL_H
#define HELIXUTIL_H

#include <QtCore>

#include <config.h>
#include <hxcom.h>

// Return path to Helix libraries
QString helix_library_path();

class GenericContext : public IUnknown
{
public:
    GenericContext( const QList<IUnknown*>& classes = QList<IUnknown*>() );
    virtual ~GenericContext();

    // IUnknown
    STDMETHOD(QueryInterface) (THIS_
        REFIID ID,
        void **object);
    STDMETHOD_(UINT32, AddRef) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

private:
    INT32 m_refCount;

    QList<IUnknown*> m_classes;
};


template <typename creator, typename klass>
inline HX_RESULT queryInterface(creator* c, REFIID iid, klass*& object)
{
    void* tmp = 0;
    HX_RESULT rc = c->QueryInterface(iid, &tmp);
    if (rc == HXR_OK)
        object = reinterpret_cast<klass*>(tmp);

    return rc;
}

template <typename creator, typename klass>
inline HX_RESULT createInstance(creator* c, REFCLSID iid, klass*& object)
{
    void* tmp = 0;
    HX_RESULT rc = c->CreateInstance(iid, &tmp);
    if (rc == HXR_OK)
        object = reinterpret_cast<klass*>(tmp);

    return rc;
}


#endif
