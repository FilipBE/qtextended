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

#include "q3toolbar/q3toolbar_plugin.h"
#include "q3iconview/q3iconview_plugin.h"
#include "q3groupbox/q3groupbox_plugin.h"
#include "q3frame/q3frame_plugin.h"
#include "q3wizard/q3wizard_plugin.h"
#include "q3mainwindow/q3mainwindow_plugin.h"
#include "q3widgetstack/q3widgetstack_plugin.h"
#include "q3buttongroup/q3buttongroup_plugin.h"
#include "q3listview/q3listview_plugin.h"
#include "q3table/q3table_plugin.h"
#include "q3listbox/q3listbox_plugin.h"
#include "q3listview/q3listview_plugin.h"
#include "q3textedit/q3textedit_plugin.h"
#include "q3dateedit/q3dateedit_plugin.h"
#include "q3timeedit/q3timeedit_plugin.h"
#include "q3datetimeedit/q3datetimeedit_plugin.h"
#include "q3progressbar/q3progressbar_plugin.h"
#include "q3textbrowser/q3textbrowser_plugin.h"

#include <QtDesigner/QDesignerCustomWidgetCollectionInterface>
#include <QtCore/qplugin.h>
#include <QtCore/qdebug.h>
#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE

class Qt3SupportWidgets: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
public:
    Qt3SupportWidgets(QObject *parent = 0);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> m_plugins;
};

Qt3SupportWidgets::Qt3SupportWidgets(QObject *parent)
    : QObject(parent)
{
    const QIcon qt3Icon(QLatin1String(":/trolltech/formeditor/images/qt3logo.png"));
    m_plugins.append(new Q3ToolBarPlugin(qt3Icon, this));
    m_plugins.append(new Q3IconViewPlugin(qt3Icon, this));
    m_plugins.append(new Q3GroupBoxPlugin(qt3Icon, this));
    m_plugins.append(new Q3FramePlugin(qt3Icon, this));
    m_plugins.append(new Q3WizardPlugin(qt3Icon, this));
    m_plugins.append(new Q3MainWindowPlugin(qt3Icon, this));
    m_plugins.append(new Q3WidgetStackPlugin(qt3Icon, this));
    m_plugins.append(new Q3ButtonGroupPlugin(qt3Icon, this));
    m_plugins.append(new Q3TablePlugin(qt3Icon, this));
    m_plugins.append(new Q3ListBoxPlugin(qt3Icon, this));
    m_plugins.append(new Q3ListViewPlugin(qt3Icon, this));
    m_plugins.append(new Q3TextEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3DateEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3TimeEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3DateTimeEditPlugin(qt3Icon, this));
    m_plugins.append(new Q3ProgressBarPlugin(qt3Icon, this));
    m_plugins.append(new Q3TextBrowserPlugin(qt3Icon, this));
}

QList<QDesignerCustomWidgetInterface*> Qt3SupportWidgets::customWidgets() const
{
    return m_plugins;
}

Q_EXPORT_PLUGIN(Qt3SupportWidgets)

QT_END_NAMESPACE

#include "qt3supportwidgets.moc"
