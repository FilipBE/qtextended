/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtXMLPatterns module of the Qt Toolkit.
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

#include "qcommonnamespaces_p.h"
#include "qnamepool_p.h"

#include "qexternalenvironment_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

/* The goal is to support serialization, backwards compatibility, import feature
 * and full axis featur, but not schema awareness. */

const xsDecimal ExternalEnvironment::XSLVersion                        (xsDecimal(1.20));
const QString   ExternalEnvironment::Vendor                            (QLatin1String("The Patternist Team"));
const QUrl      ExternalEnvironment::VendorURL                         (QLatin1String("http://svg.kde.org/"));
const QString   ExternalEnvironment::ProductName                       (QLatin1String("Patternist"));
const QString   ExternalEnvironment::ProductVersion                    (QLatin1String("0.1"));
const bool      ExternalEnvironment::IsSchemaAware                     (false);
const bool      ExternalEnvironment::SupportsSerialization             (false);
const bool      ExternalEnvironment::SupportsBackwardsCompatibility    (false);

QString ExternalEnvironment::toString(const bool value)
{
    return value ? QLatin1String("yes") : QLatin1String("no");
}

QString ExternalEnvironment::retrieveProperty(const QXmlName name)
{
    if(name.namespaceURI() != StandardNamespaces::xslt)
        return QString();

    switch(name.localName())
    {
        case StandardLocalNames::version:
            return QString::number(ExternalEnvironment::XSLVersion);
        case StandardLocalNames::vendor:
            return ExternalEnvironment::Vendor;
        case StandardLocalNames::vendor_url:
            return QString(ExternalEnvironment::VendorURL.toString());
        case StandardLocalNames::product_name:
            return ExternalEnvironment::ProductName;
        case StandardLocalNames::product_version:
            return ExternalEnvironment::ProductVersion;
        case StandardLocalNames::is_schema_aware:
            return toString(ExternalEnvironment::IsSchemaAware);
        case StandardLocalNames::supports_serialization:
            return toString(ExternalEnvironment::SupportsSerialization);
        case StandardLocalNames::supports_backwards_compatibility:
            return toString(ExternalEnvironment::SupportsBackwardsCompatibility);
        default:
            return QString();
    }
}

QT_END_NAMESPACE
