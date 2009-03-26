/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef UILIBPROPERTIES_H
#define UILIBPROPERTIES_H

#include <QtDesigner/uilib_global.h>

#include <QtCore/QObject>
#include <QtCore/QMetaProperty>
#include <QtCore/QLocale>
#include <QtGui/QWidget>

#include "formbuilderextra_p.h"

QT_BEGIN_NAMESPACE

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class QAbstractFormBuilder;
class DomProperty;

QDESIGNER_UILIB_EXPORT DomProperty *variantToDomProperty(QAbstractFormBuilder *abstractFormBuilder, QObject *object, const QString &propertyName, const QVariant &value);


QDESIGNER_UILIB_EXPORT QVariant domPropertyToVariant(const DomProperty *property);
QDESIGNER_UILIB_EXPORT QVariant domPropertyToVariant(QAbstractFormBuilder *abstractFormBuilder, const QMetaObject *meta, const  DomProperty *property);

// This class exists to provide meta information
// for enumerations only.
class QAbstractFormBuilderGadget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ fakeOrientation)
    Q_PROPERTY(QSizePolicy::Policy sizeType READ fakeSizeType)
    Q_PROPERTY(QPalette::ColorRole colorRole READ fakeColorRole)
    Q_PROPERTY(QPalette::ColorGroup colorGroup READ fakeColorGroup)
    Q_PROPERTY(QFont::StyleStrategy styleStrategy READ fakeStyleStrategy)
    Q_PROPERTY(Qt::CursorShape cursorShape READ fakeCursorShape)
    Q_PROPERTY(Qt::BrushStyle brushStyle READ fakeBrushStyle)
    Q_PROPERTY(Qt::ToolBarArea toolBarArea READ fakeToolBarArea)
    Q_PROPERTY(QGradient::Type gradientType READ fakeGradientType)
    Q_PROPERTY(QGradient::Spread gradientSpread READ fakeGradientSpread)
    Q_PROPERTY(QGradient::CoordinateMode gradientCoordinate READ fakeGradientCoordinate)
    Q_PROPERTY(QLocale::Language language READ fakeLanguage)
    Q_PROPERTY(QLocale::Country country READ fakeCountry)
public:
    QAbstractFormBuilderGadget() { Q_ASSERT(0); }

    Qt::Orientation fakeOrientation() const     { Q_ASSERT(0); return Qt::Horizontal; }
    QSizePolicy::Policy fakeSizeType() const    { Q_ASSERT(0); return QSizePolicy::Expanding; }
    QPalette::ColorGroup fakeColorGroup() const { Q_ASSERT(0); return static_cast<QPalette::ColorGroup>(0); }
    QPalette::ColorRole fakeColorRole() const   { Q_ASSERT(0); return static_cast<QPalette::ColorRole>(0); }
    QFont::StyleStrategy fakeStyleStrategy() const     { Q_ASSERT(0); return QFont::PreferDefault; }
    Qt::CursorShape fakeCursorShape() const     { Q_ASSERT(0); return Qt::ArrowCursor; }
    Qt::BrushStyle fakeBrushStyle() const       { Q_ASSERT(0); return Qt::NoBrush; }
    Qt::ToolBarArea fakeToolBarArea() const {  Q_ASSERT(0); return Qt::NoToolBarArea; }
    QGradient::Type fakeGradientType() const    { Q_ASSERT(0); return QGradient::NoGradient; }
    QGradient::Spread fakeGradientSpread() const  { Q_ASSERT(0); return QGradient::PadSpread; }
    QGradient::CoordinateMode fakeGradientCoordinate() const  { Q_ASSERT(0); return QGradient::LogicalMode; }
    QLocale::Language fakeLanguage() const  { Q_ASSERT(0); return QLocale::C; }
    QLocale::Country fakeCountry() const  { Q_ASSERT(0); return QLocale::AnyCountry; }
};

// Convert key to value for a given QMetaEnum
template <class EnumType>
inline EnumType enumKeyToValue(const QMetaEnum &metaEnum,const char *key, const EnumType* = 0)
{
    int val = metaEnum.keyToValue(key);
    if (val == -1) {

        uiLibWarning(QObject::tr("The enumeration-value \"%1\" is invalid. The default value \"%2\" will be used instead.")
                    .arg(QString::fromUtf8(key)).arg(QString::fromUtf8(metaEnum.key(0))));
        val = metaEnum.value(0);
    }
    return static_cast<EnumType>(val);
}

// Access meta enumeration object of a qobject
template <class QObjectType>
inline QMetaEnum metaEnum(const char *name, const QObjectType* = 0)
{
    const int e_index = QObjectType::staticMetaObject.indexOfProperty(name);
    Q_ASSERT(e_index != -1);
    return QObjectType::staticMetaObject.property(e_index).enumerator();
}

// Convert key to value for enumeration by name
template <class QObjectType, class EnumType>
inline EnumType enumKeyOfObjectToValue(const char *enumName, const char *key, const QObjectType* = 0, const EnumType* = 0)
{
    const QMetaEnum me = metaEnum<QObjectType>(enumName);
    return enumKeyToValue<EnumType>(me, key);
}

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // UILIBPROPERTIES_H
