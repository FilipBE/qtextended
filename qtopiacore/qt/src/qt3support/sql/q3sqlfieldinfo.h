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

#ifndef Q3SQLFIELDINFO_H
#define Q3SQLFIELDINFO_H

#ifndef QT_NO_SQL

#include <QtCore/qglobal.h>
#include <QtSql/qsqlfield.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3Support)

/* Q3SqlFieldInfo Class
   obsoleted, use QSqlField instead
*/

class Q_COMPAT_EXPORT Q3SqlFieldInfo
{
    // class is obsoleted, won't change anyways,
    // so no d pointer
    int req, len, prec, tID;
    uint gen: 1;
    uint trim: 1;
    uint calc: 1;
    QString nm;
    QVariant::Type typ;
    QVariant defValue;

public:
    Q3SqlFieldInfo(const QString& name = QString(),
                   QVariant::Type typ = QVariant::Invalid,
                   int required = -1,
                   int len = -1,
                   int prec = -1,
                   const QVariant& defValue = QVariant(),
                   int sqlType = 0,
                   bool generated = true,
                   bool trim = false,
                   bool calculated = false) :
        req(required), len(len), prec(prec), tID(sqlType),
        gen(generated), trim(trim), calc(calculated),
        nm(name), typ(typ), defValue(defValue) {}

    virtual ~Q3SqlFieldInfo() {}

    Q3SqlFieldInfo(const QSqlField & other)
    {
        nm = other.name();
        typ = other.type();
        switch (other.requiredStatus()) {
        case QSqlField::Unknown: req = -1; break;
        case QSqlField::Required: req = 1; break;
        case QSqlField::Optional: req = 0; break;
        }
        len = other.length();
        prec = other.precision();
        defValue = other.defaultValue();
        tID = other.typeID();
        gen = other.isGenerated();
        calc = false;
        trim = false;
    }

    bool operator==(const Q3SqlFieldInfo& f) const
    {
        return (nm == f.nm &&
                typ == f.typ &&
                req == f.req &&
                len == f.len &&
                prec == f.prec &&
                defValue == f.defValue &&
                tID == f.tID &&
                gen == f.gen &&
                trim == f.trim &&
                calc == f.calc);
    }

    QSqlField toField() const
    { QSqlField f(nm, typ);
      f.setRequiredStatus(QSqlField::RequiredStatus(req));
      f.setLength(len);
      f.setPrecision(prec);
      f.setDefaultValue(defValue);
      f.setSqlType(tID);
      f.setGenerated(gen);
      return f;
    }
    int isRequired() const
    { return req; }
    QVariant::Type type() const
    { return typ; }
    int length() const
    { return len; }
    int precision() const
    { return prec; }
    QVariant defaultValue() const
    { return defValue; }
    QString name() const
    { return nm; }
    int typeID() const
    { return tID; }
    bool isGenerated() const
    { return gen; }
    bool isTrim() const
    { return trim; }
    bool isCalculated() const
    { return calc; }

    virtual void setTrim(bool trim)
    { this->trim = trim; }
    virtual void setGenerated(bool generated)
    { gen = generated; }
    virtual void setCalculated(bool calculated)
    { calc = calculated; }

};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_SQL

#endif // Q3SQLFIELDINFO_H
