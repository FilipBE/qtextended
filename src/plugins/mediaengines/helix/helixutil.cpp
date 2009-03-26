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

#include "helixutil.h"

#include <qtopianamespace.h>

QString helix_library_path()
{
    static QString library_path;
    static bool resolve = true;

    if( resolve ) {
        static const QString CORELIB_PATH = "lib/helix/hxmedpltfm.so";

        resolve = false;
        foreach( QString preface, Qtopia::installPaths() ) {
            if( QFile::exists( preface + CORELIB_PATH ) ) {
                return library_path = preface + QString( "lib/helix" );
            }
        }
    }

    return library_path;
}

GenericContext::GenericContext( const QList<IUnknown*>& classes )
    : m_refCount( 0 ), m_classes( classes )
{
    foreach( IUnknown* unknown, m_classes ) {
        HX_ADDREF( unknown );
    }
}

GenericContext::~GenericContext()
{
    foreach( IUnknown* unknown, m_classes ) {
        HX_RELEASE( unknown );
    }
}

STDMETHODIMP_(ULONG32) GenericContext::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

STDMETHODIMP_(ULONG32) GenericContext::Release()
{
    if( InterlockedDecrement( &m_refCount ) > 0 )
    {
        return m_refCount;
    }

    delete this;
    return 0;
}

STDMETHODIMP GenericContext::QueryInterface( REFIID riid, void** object )
{
    if( IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *object = (IUnknown*)this;
        return HXR_OK;
    } else {
        foreach( IUnknown* unknown, m_classes ) {
            if( unknown->QueryInterface( riid, object ) == HXR_OK ) {
                return HXR_OK;
            }
        }
    }

    *object = NULL;
    return HXR_NOINTERFACE;
}
