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

#ifndef QEXPRESSIONEVALUATOR_H
#define QEXPRESSIONEVALUATOR_H

/* Qt includes */
#include <QByteArray>
#include <QVariant>
#include <QObject>

#include "qtopiailglobal.h"

struct QExpressionEvaluatorPrivate;

//========================================
//= Expression Declaration
//=======================================
class ExpressionTokenizer;
class QTOPIAIL_EXPORT QExpressionEvaluator : public QObject 
{
    Q_OBJECT
public:
    /* Public Data */
    enum FloatingPointFormat {
        Double,
        FixedPoint
    };

    /* Public Methods */
    /* Expression Ctors */
    explicit QExpressionEvaluator( QObject* parent = 0 );
    explicit QExpressionEvaluator( const QByteArray&, QObject* parent = 0 );
    /* Expression Dtor */
    ~QExpressionEvaluator();

    bool isValid() const;
    bool evaluate();
    QVariant result();

    void setFloatingPointFormat( const FloatingPointFormat& fmt );
    FloatingPointFormat floatingPointFormat() const;

    QByteArray expression() const;
public slots:
    bool setExpression( const QByteArray& expr );
    void clear();

signals:
    void termsChanged();

private:
    /* Private Data */
    QExpressionEvaluatorPrivate* d;
};

#endif
