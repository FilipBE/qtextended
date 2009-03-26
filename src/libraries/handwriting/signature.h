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

#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <qobject.h>
#include <qbytearray.h>
#include <qvector.h>
#include <qlist.h>
#include <qtopiaglobal.h>

struct QTOPIAHW_EXPORT Q_PACKED QIMPenGlyphLink
{
    signed char dx;
    signed char dy;
};

class QIMPenStroke;

class QTOPIAHW_EXPORT QIMPenSignature : public QVector<int>
{
public:
    QIMPenSignature();
    virtual ~QIMPenSignature(){}
    explicit QIMPenSignature(const QVector<int> &o);
    QIMPenSignature(const QIMPenSignature &o);
    void setStroke(const QIMPenStroke &links);
    virtual QByteArray name() const = 0;
    virtual int maxError() const = 0;
    virtual int weight() const = 0;

    virtual int calcError(const QIMPenSignature &other, const QVector<int> &weight = QVector<int>()) const;

protected:
    virtual void calcSignature(const QIMPenStroke &links) = 0;
    // assist if using normal calc error.
    void scale(unsigned int, bool);
    static QVector<int> createBase(const QVector<int>&, int e);
    static int calcError(const QVector<int> &, const QVector<int> &, int offset, bool t, const QVector<int> &weight = QVector<int>());
    virtual bool loops() const { return true; }
    virtual bool slides() const { return true; }
};

class QTOPIAHW_EXPORT TanSignature : public QIMPenSignature
{
public:
    TanSignature() : QIMPenSignature() {}
    explicit TanSignature(const QIMPenStroke &);
    virtual ~TanSignature();

    QByteArray name() const { return "tan"; }
    int maxError() const { return 40; }
    int weight() const { return 1; }

protected:
    void calcSignature(const QIMPenStroke &links);
};

class QTOPIAHW_EXPORT AngleSignature : public QIMPenSignature
{
public:
    AngleSignature() : QIMPenSignature() {}
    explicit AngleSignature(const QIMPenStroke &);
    virtual ~AngleSignature();

    QByteArray name() const { return "tan"; }
    int maxError() const { return 60; }
    int weight() const { return 40; }

protected:
    void calcSignature(const QIMPenStroke &links);
};

class QTOPIAHW_EXPORT DistSignature : public QIMPenSignature
{
public:
    DistSignature() : QIMPenSignature() {}
    explicit DistSignature(const QIMPenStroke &);
    virtual ~DistSignature();

    QByteArray name() const { return "tan"; }
    int maxError() const { return 100; }
    int weight() const { return 60; }

protected:
    void calcSignature(const QIMPenStroke &links);
    bool loops() const { return false; }
};

#endif

