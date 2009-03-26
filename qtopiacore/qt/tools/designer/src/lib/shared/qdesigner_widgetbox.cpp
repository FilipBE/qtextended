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

/*
TRANSLATOR qdesigner_internal::QDesignerWidgetBox
*/

#include "qdesigner_widgetbox_p.h"
#include "qdesigner_utils_p.h"

#include <ui4_p.h>

#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtXml/QDomDocument>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
QDesignerWidgetBox::QDesignerWidgetBox(QWidget *parent, Qt::WindowFlags flags)
    : QDesignerWidgetBoxInterface(parent, flags),
      m_loadMode(LoadMerge)
{

}

QDesignerWidgetBox::LoadMode QDesignerWidgetBox::loadMode() const
{
    return m_loadMode;
}

void QDesignerWidgetBox::setLoadMode(LoadMode lm)
{
     m_loadMode = lm;
}

// Convenience to find a widget by class name
bool QDesignerWidgetBox::findWidget(const QDesignerWidgetBoxInterface *wbox, const QString &className, Widget *widgetData)
{
    // Note that entry names do not necessarily match the class name
    // (at least, not for the standard widgets), so,
    // look in the XML for the class name of the first widget to appear
    const QString widgetTag = QLatin1String("<widget");
    QString pattern = QLatin1String("^<widget\\s+class\\s*=\\s*\"");
    pattern += className;
    pattern += QLatin1String("\".*$");
    const QRegExp regexp(pattern);
    Q_ASSERT(regexp.isValid());
    const int catCount = wbox->categoryCount();
    for (int c = 0; c < catCount; c++) {
        const Category cat = wbox->category(c);
        if (const int widgetCount =  cat.widgetCount())
            for (int w = 0; w < widgetCount; w++) {
                const Widget widget = cat.widget(w);
                QString xml = widget.domXml(); // Erase the <ui> tag that can be present starting from 4.4
                const int widgetTagIndex = xml.indexOf(widgetTag);
                if (widgetTagIndex != -1) {
                    xml.remove(0, widgetTagIndex);
                    if (regexp.exactMatch(xml)) {
                        *widgetData = widget;
                        return true;
                    }
                }
            }
    }
    return false;
}

// Convenience to create a Dom Widget from widget box xml code.
DomUI *QDesignerWidgetBox::xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel, QString *errorMessage)
{
    QDomDocument doc;
    int errorLine, errorColumn;
    if (!doc.setContent(xml, errorMessage, &errorLine, &errorColumn)) {
        *errorMessage = QObject::tr("A parse error occurred at line %1, column %2 of the XML code specified for the widget %3: %4\n%5").
                                 arg(errorLine).arg(errorColumn).arg(name).arg(*errorMessage).arg(xml);
        return 0;
    }

    if (!doc.hasChildNodes()) {
        *errorMessage = QObject::tr("The XML code specified for the widget %1 does not contain any widget elements.\n%2").arg(name).arg(xml);
        return 0;
    }

    QDomElement rootElement = doc.firstChildElement();
    const QString rootNode = rootElement.nodeName();

    const QString widgetTag = QLatin1String("widget");
    if (rootNode == widgetTag) { // 4.3 legacy ,wrap into DomUI
        DomUI *rc = new DomUI;
        DomWidget *widget = new DomWidget;
        widget->read(rootElement);
        if (insertFakeTopLevel)  {
            DomWidget *fakeTopLevel = new DomWidget;
            QList<DomWidget *> children;
            children.push_back(widget);
            fakeTopLevel->setElementWidget(children);
            rc->setElementWidget(fakeTopLevel);
        } else {
            rc->setElementWidget(widget);
        }
        return rc;
    }

    if (rootNode == QLatin1String("ui")) { // 4.4
        QDomElement widgetChild = rootElement.firstChildElement(widgetTag);
        if (widgetChild.isNull()) {
            *errorMessage = QObject::tr("The XML code specified for the widget %1 does not contain valid widget element\n%2").arg(name).arg(xml);
            return 0;
        }
        if (insertFakeTopLevel)  {
            QDomElement fakeTopLevel = doc.createElement(widgetTag);
            rootElement.replaceChild(fakeTopLevel, widgetChild);
            fakeTopLevel.appendChild(widgetChild);
        }
        DomUI *rc = new DomUI;
        rc->read(rootElement);
        return rc;
    }

    *errorMessage = QObject::tr("The XML code specified for the widget %1 contains an invalid root element %2.\n%3").arg(name).arg(rootNode).arg(xml);
    return 0;
}

// Convenience to create a Dom Widget from widget box xml code.
DomUI *QDesignerWidgetBox::xmlToUi(const QString &name, const QString &xml, bool insertFakeTopLevel)
{
    QString errorMessage;
    DomUI *rc = xmlToUi(name, xml, insertFakeTopLevel, &errorMessage);
    if (!rc)
        qdesigner_internal::designerWarning(errorMessage);
    return rc;
}

}  // namespace qdesigner_internal

QT_END_NAMESPACE
