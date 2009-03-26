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

#ifndef PKIMPL_H
#define PKIMPL_H

#include <inputmethodinterface.h>
#include <QIcon>

class PkIM;
class QPixmap;
class QLabel;
class QUuid;
class InputMatcherGuessList;

class PkImpl : public QtopiaInputMethod
{

public:
    PkImpl(QObject *parent = 0);
    virtual ~PkImpl();

#ifndef QT_NO_COMPONENT
//    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
//    Q_REFCOUNT
#endif
    QString name() const;
    QString identifier() const;
    QString version() const;
    QIcon icon() const;

    virtual void reset();
    State state() const;

    virtual QWSInputMethod *inputModifier( );

    virtual QWidget *statusWidget( QWidget *parent );

    virtual int properties() const;

    // hint done by qcop
    virtual void setHint(const QString &, bool);
    bool restrictedToHint() const;


private:
    PkIM *input;
    mutable QIcon icn;
    QLabel *statWid;
    ulong ref;
};

#endif
