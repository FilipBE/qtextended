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

/* Released originally in the public domain by Philip J. Erdelsky */

#include "qfixedpointnumber_p.h"

#include <QtGlobal>

QFixedPointNumber::QFixedPointNumber() {
    value = 0;
    precision = 0;
}

QFixedPointNumber::QFixedPointNumber( const char *s ) {
    (*this) = s;
}

QFixedPointNumber QFixedPointNumber::operator = (const char *s )
{
  value = 0;
  precision = 0;
  int c;
  while ((c = *s++) == ' ' || c == '\t');
  bool negative;
  if (c == '-')
  {
    negative = true;
    c = *s++;
  }
  else
    negative = false;
  bool decimal = false;
  while (precision < MAX_FIXED_PRECISION)
  {
    if ('0' <= c && c <= '9')
    {
      value = value * 10 + (c - '0');
      if (decimal)
        precision++;
    }
    else if (c == '.')
    {
      if (decimal)
        break;
      decimal = true;
    }
    else if (c != ',')
      break;
    c = *s++;
  }
  if (negative)
    value = - value;
  return *this;
}

bool operator < (QFixedPointNumber x, QFixedPointNumber y)
{
  x.equalize_precision(y);
  return x.value < y.value;
}

bool operator < (QFixedPointNumber x, int y)
{
  QFixedPointNumber r;
  r.value = y;
  r.precision = 0;
  return x < r;
}

bool operator < (int x, QFixedPointNumber y)
{
  QFixedPointNumber l;
  l.value = x;
  l.precision = 0;
  return l < y;
}

QFixedPointNumber operator - (QFixedPointNumber x)
{
  x.value = -x.value;
  return x;
}

QFixedPointNumber operator * (QFixedPointNumber x, int y)
{
  QFixedPointNumber r;
  r.value = y;
  r.precision = 0;
  return x * r;
}

QFixedPointNumber operator * (QFixedPointNumber x, QFixedPointNumber y)
{
  x.value = x.value * y.value;
  x.precision = x.precision + y.precision;
  return x;
}

QFixedPointNumber QFixedPointNumber::operator [] (int p) const
{
  QFixedPointNumber x;
  x.precision = p;
  x.value =
    p < precision ?
      value / qfixedpointnumber_scale_inst.x[precision - p] :
    p > precision ?
      value * qfixedpointnumber_scale_inst.x[p - precision] :
    value;
  return x;
}

//QFixedPointNumber::qfixedpointnumber_scale QFixedPointNumber::qfixedpointnumber_scale_inst;

qfixedpointnumber_scale::qfixedpointnumber_scale(void)
{
  int i;
  QFixedPointNumberType n;
  n = 1;
  for (i = 0; i <= MAX_FIXED_PRECISION; i++)
  {
    x[i] = n;
    n = 10 * n;
  }
}

QFixedPointNumber operator - (QFixedPointNumber x, QFixedPointNumber y)
{
  x.equalize_precision(y);
  x.value = x.value - y.value;
  return x;
}

QFixedPointNumber operator - (QFixedPointNumber x, int y)
{
  QFixedPointNumber r;
  r.value = y;
  r.precision = 0;
  return x - r;
}

QFixedPointNumber operator - (int x, QFixedPointNumber y)
{
  QFixedPointNumber l;
  l.value = x;
  l.precision = 0;
  return l - y;
}

int QFixedPointNumber::whole(void) const
{
  return qfixedpointnumber_cast_to_integer(value / qfixedpointnumber_scale_inst.x[precision]);
}

QFixedPointNumber operator + (QFixedPointNumber x, QFixedPointNumber y)
{
  x.equalize_precision(y);
  x.value = x.value + y.value;
  return x;
}

QFixedPointNumber operator + (QFixedPointNumber x, int y)
{
  QFixedPointNumber r;
  r.value = y;
  r.precision = 0;
  return x + r;
}

QFixedPointNumber QFixedPointNumber::operator = (QFixedPointNumber x)
{
    if( precision == 0 && value == 0 ) {
        value = x.value;
        precision = x.precision;
    } else if( precision > x.precision )
        value = x.value * qfixedpointnumber_scale_inst.x[precision - x.precision];
    else if( precision < x.precision )
        value = x.value / qfixedpointnumber_scale_inst.x[x.precision - precision];
    else
        value = x.value;
  return *this;
}

QFixedPointNumber QFixedPointNumber::operator = (int x)
{
  value = x;
  precision = 0;
  return *this;
}

QFixedPointNumber operator / (QFixedPointNumber x, QFixedPointNumber y)
{
  x.value = (x.value * qfixedpointnumber_scale_inst.x[y.precision]) / y.value;
  return x;
}

QFixedPointNumber operator / (QFixedPointNumber x, int y)
{
  QFixedPointNumber r;
  r.value = y;
  r.precision = 0;
  return x / r;
}

QFixedPointNumber operator / (int x, QFixedPointNumber y)
{
  QFixedPointNumber l;
  l.value = x;
  l.precision = 0;
  return l / y;
}

QString QFixedPointNumber::toString(int options) const
{
    QFixedPointNumberType x = value;
  bool negative;
  if (x < 0)
  {
    x = -x;
    // prevent buffer overflow if result is still negative
    if (x < 0)
      x = x - 1;
    negative = true;
  }
  else
    negative = false;
  int n = 0;
  int units = 0;
  const int bufsize = MAX_FIXED_LENGTH + MAX_FIXED_PRECISION;
  char buffer[bufsize+1];
  buffer[bufsize] = '\0';
  do
  {
    if (n == precision)
    {
      if (n > 0 || options & DECIMAL)
        buffer[bufsize - ++n] = '.';
      units = n;
    }
    else if (options & COMMAS && n > precision && (n - units) % 4 == 3)
      buffer[bufsize - ++n] = ',';
    QFixedPointNumberType y;
    y = x / 10;
    buffer[bufsize - ++n] = qfixedpointnumber_cast_to_integer(x - y*10) + '0';
    x = y;
  } while (n <= precision || x != 0);
  if (negative)
    buffer[bufsize - ++n] = '-';
  if (options & ALIGN)
  {
    while (n - units < MAX_FIXED_LENGTH-2)
      buffer[bufsize - ++n] = ' ';
  }
  QString str((const char *) (buffer + bufsize - n));
  return str;
}

bool operator == (QFixedPointNumber x, QFixedPointNumber y)
{
  x.equalize_precision(y);
  return x.value == y.value;
}

bool operator == (QFixedPointNumber x, int y)
{
  QFixedPointNumber r;
  r.value = y;
  r.precision = 0;
  return x == r;
}

bool operator == (int x, QFixedPointNumber y)
{
  QFixedPointNumber l;
  l.value = x;
  l.precision = 0;
  return l == y;
}

void QFixedPointNumber::equalize_precision(QFixedPointNumber &x)
{
    if( precision == 0 && value == 0 ) {
        precision = x.precision;
        value = x.value;
        return;
    }
  if (precision < x.precision)
  {
    value = value * qfixedpointnumber_scale_inst.x[x.precision - precision];
    precision = x.precision;
  }
  else if (x.precision < precision)
  {
    x.value = x.value * qfixedpointnumber_scale_inst.x[precision - x.precision];
    x.precision = precision;
  }
}

