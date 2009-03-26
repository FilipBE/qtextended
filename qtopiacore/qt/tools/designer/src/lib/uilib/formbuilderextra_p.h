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

#ifndef ABSTRACTFORMBUILDERPRIVATE_H
#define ABSTRACTFORMBUILDERPRIVATE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "uilib_global.h"

#ifndef QT_FORMBUILDER_NO_SCRIPT
#    include "formscriptrunner_p.h"
#endif

#include <QtCore/QHash>
#include <QtCore/QPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QObject;
class QVariant;
class QWidget;
class QObject;
class QLabel;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class QAbstractFormBuilder;
class QResourceBuilder;

class QDESIGNER_UILIB_EXPORT QFormBuilderExtra
{
    QFormBuilderExtra();
    ~QFormBuilderExtra();
public:
    void clear();

    bool applyPropertyInternally(QObject *o, const QString &propertyName, const QVariant &value);

    enum BuddyMode { BuddyApplyAll, BuddyApplyVisibleOnly };

    void applyInternalProperties() const;
    static bool applyBuddy(const QString &buddyName, BuddyMode applyMode, QLabel *label);

    const QPointer<QWidget> &rootWidget() const;
    void setRootWidget(const QPointer<QWidget> &w);

#ifndef QT_FORMBUILDER_NO_SCRIPT
    QFormScriptRunner &formScriptRunner();
    void storeCustomWidgetScript(const QString &className, const QString &script);
    QString customWidgetScript(const QString &className) const;
#endif

    void setProcessingLayoutWidget(bool processing);
    bool processingLayoutWidget() const;

    void setResourceBuilder(QResourceBuilder *builder);
    QResourceBuilder *resourceBuilder() const;

    static QFormBuilderExtra *instance(const QAbstractFormBuilder *afb);
    static void removeInstance(const QAbstractFormBuilder *afb);

    void storeCustomWidgetAddPageMethod(const QString &className, const QString &ct);
    QString customWidgetAddPageMethod(const QString &className) const;

    void storeCustomWidgetBaseClass(const QString &className, const QString &baseClassName);
    QString customWidgetBaseClass(const QString &className) const;

private:
    void clearResourceBuilder();

    typedef QHash<QLabel*, QString> BuddyHash;
    BuddyHash m_buddies;

#ifndef QT_FORMBUILDER_NO_SCRIPT
    QFormScriptRunner m_FormScriptRunner;

    typedef QHash<QString, QString> CustomWidgetScriptHash;
    CustomWidgetScriptHash m_customWidgetScriptHash;
#endif

    QHash<QString, QString> m_customWidgetAddPageMethodHash;
    QHash<QString, QString> m_customWidgetBaseClassHash;

    bool m_layoutWidget;
    QResourceBuilder *m_resourceBuilder;

    QPointer<QWidget> m_rootWidget;
};

void uiLibWarning(const QString &message);

// Struct with static accessor that provides most strings used in the form builder.
struct QDESIGNER_UILIB_EXPORT QFormBuilderStrings {
    QFormBuilderStrings();

    static const QFormBuilderStrings &instance();

    const QString buddyProperty;
    const QString cursorProperty;
    const QString objectNameProperty;
    const QString trueValue;
    const QString falseValue;
    const QString horizontalPostFix;
    const QString separator;
    const QString defaultTitle;
    const QString titleAttribute;
    const QString labelAttribute;
    const QString toolTipAttribute;
    const QString iconAttribute;
    const QString pixmapAttribute;
    const QString textAttribute;
    const QString currentIndexProperty;
    const QString toolBarAreaAttribute;
    const QString toolBarBreakAttribute;
    const QString dockWidgetAreaAttribute;
    const QString marginProperty;
    const QString spacingProperty;
    const QString leftMarginProperty;
    const QString topMarginProperty;
    const QString rightMarginProperty;
    const QString bottomMarginProperty;
    const QString horizontalSpacingProperty;
    const QString verticalSpacingProperty;
    const QString sizeHintProperty;
    const QString sizeTypeProperty;
    const QString orientationProperty;
    const QString styleSheetProperty;
    const QString qtHorizontal;
    const QString qtVertical;
    const QString currentRowProperty;
    const QString tabSpacingProperty;
    const QString qWidgetClass;
    const QString lineClass;
    const QString geometryProperty;
    const QString scriptWidgetVariable;
    const QString scriptChildWidgetsVariable;
};
#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // ABSTRACTFORMBUILDERPRIVATE_H
