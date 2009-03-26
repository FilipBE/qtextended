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

#include "qmaildatacomparator.h"

/*!
    \enum QMailDataComparator::Comparator

    Defines the comparison operations that can be used to compare data elements 
    of QMailStore objects.  Not all operations are applicable to all data elements.

    \value LessThan represents the '<' operator.
    \value LessThanEqual represents the '<=' operator.
    \value GreaterThan represents the '>' operator.
    \value GreaterThanEqual represents the '>= operator'.
    \value Equal represents the '=' operator.
    \value NotEqual represents the '!=' operator.
    \value Includes represents an operation in which an associated property is 
           tested to determine if it contains a provided value.  For most property 
           types this will perform a string based check. For status type properties 
           this will perform a check to see if a status bit value is set.
    \value Excludes represents an operation in which an associated property is
           tested to determine that it does not contains a provided value. Excludes 
           is the inverse relationship to \c Includes.
*/

