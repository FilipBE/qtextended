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

#ifndef QFIXEDPOINTNUMBER_P_H
#define QFIXEDPOINTNUMBER_P_H

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

#include <QString>
#include <QtGlobal>

const int MAX_FIXED_PRECISION = 15;
const int MAX_FIXED_LENGTH = 28;



typedef qint64 QFixedPointNumberType;

inline int qfixedpointnumber_cast_to_integer(QFixedPointNumberType x) {return (int) x;}

class qfixedpointnumber_scale
{
  public:
      QFixedPointNumberType x[MAX_FIXED_PRECISION+1];
    qfixedpointnumber_scale();
};
static const qfixedpointnumber_scale qfixedpointnumber_scale_inst;

class QFixedPointNumber
{
  private:
  public:
    QFixedPointNumberType value;
    unsigned char precision;
    void equalize_precision(QFixedPointNumber &);

  public:
    QFixedPointNumber(int x, int p) {value = x; precision = p;}
    explicit QFixedPointNumber(const char *);
    QFixedPointNumber();
    QFixedPointNumber operator [] (int) const;
    QFixedPointNumber operator = (QFixedPointNumber);
    QFixedPointNumber operator = (int);
    QFixedPointNumber operator = (const char*);
    friend QFixedPointNumber operator + (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator - (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator * (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator / (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator - (QFixedPointNumber);
    friend bool operator == (QFixedPointNumber, QFixedPointNumber);
    friend bool operator < (QFixedPointNumber, QFixedPointNumber);
    friend QFixedPointNumber operator - (int, QFixedPointNumber);
    friend QFixedPointNumber operator / (int, QFixedPointNumber);
    friend bool operator == (int, QFixedPointNumber);
    friend bool operator < (int, QFixedPointNumber);
    friend QFixedPointNumber operator + (QFixedPointNumber, int);
    friend QFixedPointNumber operator - (QFixedPointNumber, int);
    friend QFixedPointNumber operator * (QFixedPointNumber, int);
    friend QFixedPointNumber operator / (QFixedPointNumber, int);
    friend bool operator == (QFixedPointNumber, int);
    friend bool operator < (QFixedPointNumber, int);
    operator bool ();
    bool operator ! ();
    bool operator == ( bool t );
    enum {ALIGN=1, COMMAS=2, DECIMAL=4};
    QString toString(int = COMMAS) const;
    int whole(void) const;
};

inline bool operator != (QFixedPointNumber x, QFixedPointNumber y) {return !(x == y);}
inline bool operator != (QFixedPointNumber x, int y) {return !(x == y);}
inline bool operator != (int x, QFixedPointNumber y) {return !(x == y);}
inline bool operator >= (QFixedPointNumber x, QFixedPointNumber y) {return !(x < y);}
inline bool operator >= (QFixedPointNumber x, int y) {return !(x < y);}
inline bool operator >= (int x, QFixedPointNumber y) {return !(x < y);}
inline bool operator > (QFixedPointNumber x, QFixedPointNumber y) {return y < x;}
inline bool operator > (QFixedPointNumber x, int y) {return y < x;}
inline bool operator > (int x, QFixedPointNumber y) {return y < x;}
inline bool operator <= (QFixedPointNumber x, QFixedPointNumber y) {return !(y < x);}
inline bool operator <= (QFixedPointNumber x, int y) {return !(y < x);}
inline bool operator <= (int x, QFixedPointNumber y) {return !(y < x);}
inline QFixedPointNumber::operator bool () { return value != 0; }
inline bool QFixedPointNumber::operator ! () { return value == 0; }
inline bool QFixedPointNumber::operator == ( bool t ) { return (value != 0) == t; }

#endif
