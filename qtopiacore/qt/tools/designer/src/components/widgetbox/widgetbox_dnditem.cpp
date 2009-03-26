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

#include "widgetbox_dnditem.h"
#include "ui4_p.h"
#include <spacer_widget_p.h>
#include <qdesigner_formbuilder_p.h>
#include <qtresourcemodel_p.h>
#include <formscriptrunner_p.h>
#include <QtDesigner/QDesignerFormEditorInterface>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
/*******************************************************************************
** WidgetBoxResource
*/

class WidgetBoxResource : public QDesignerFormBuilder
{
public:
    WidgetBoxResource(QDesignerFormEditorInterface *core);

    virtual QWidget *createWidget(DomWidget *ui_widget, QWidget *parentWidget)
    { return QDesignerFormBuilder::createWidget(ui_widget, parentWidget); }

    QWidget *createWidgetWithResources(const DomUI *dom_ui, DomWidget *dom_widget, DomResources *dom_resources, QWidget *result);

protected:

    virtual QWidget *create(DomWidget *ui_widget, QWidget *parents);
    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
};

WidgetBoxResource::WidgetBoxResource(QDesignerFormEditorInterface *core) :
    QDesignerFormBuilder(core, DisableScripts)
{
}


QWidget *WidgetBoxResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name)
{
    if (widgetName == QLatin1String("Spacer")) {
        Spacer *spacer = new Spacer(parentWidget);
        spacer->setObjectName(name);
        return spacer;
    }

    return QDesignerFormBuilder::createWidget(widgetName, parentWidget, name);
}

QWidget *WidgetBoxResource::create(DomWidget *ui_widget, QWidget *parent)
{
    QWidget *result = QDesignerFormBuilder::create(ui_widget, parent);
    result->setFocusPolicy(Qt::NoFocus);
    result->setObjectName(ui_widget->attributeName());

    return result;
}

QWidget *WidgetBoxResource::createWidgetWithResources(const DomUI *dom_ui, DomWidget *dom_widget, DomResources *dom_resources, QWidget *result)
{
    initialize(dom_ui);
    QtResourceSet *resourceSet = core()->resourceModel()->currentResourceSet();
    createResources(dom_resources);
    core()->resourceModel()->setCurrentResourceSet(internalResourceSet());

    QWidget *widget = createWidget(dom_widget, result);
    core()->resourceModel()->setCurrentResourceSet(resourceSet);
    core()->resourceModel()->removeResourceSet(internalResourceSet());
    return widget;
}

/*******************************************************************************
** WidgetBoxResource
*/

static QSize geometryProp(const DomWidget *dw)
{
    const QList<DomProperty*> prop_list = dw->elementProperty();
    const QString geometry = QLatin1String("geometry");
    foreach (DomProperty *prop, prop_list) {
        if (prop->attributeName() !=  geometry)
            continue;
        DomRect *dr = prop->elementRect();
        if (dr == 0)
            continue;
        return QSize(dr->elementWidth(), dr->elementHeight());
    }
    return QSize();
}

static QSize domWidgetSize(DomWidget *dw)
{
    QSize size = geometryProp(dw);
    if (size.isValid())
        return size;

    foreach (const DomWidget *child, dw->elementWidget()) {
        size = geometryProp(child);
        if (size.isValid())
            return size;
    }

    foreach (const DomLayout *dl, dw->elementLayout()) {
        foreach (DomLayoutItem *item, dl->elementItem()) {
            const DomWidget *child = item->elementWidget();
            if (child == 0)
                continue;
            size = geometryProp(child);
            if (size.isValid())
                return size;
        }
    }

    return QSize();
}

static QWidget *decorationFromDomWidget(const DomUI *dom_ui, DomWidget *dom_widget, DomResources *dom_resources, QDesignerFormEditorInterface *core)
{
    QWidget *result = new QWidget(0, Qt::ToolTip);

    WidgetBoxResource builder(core);
    QWidget *w = builder.createWidgetWithResources(dom_ui, dom_widget, dom_resources, result);
    QSize size = domWidgetSize(dom_widget);
    const QSize minimumSize = w->minimumSizeHint();
    if (!size.isValid())
        size = w->sizeHint();
    if (size.width() < minimumSize.width())
        size.setWidth(minimumSize.width());
    if (size.height() < minimumSize.height())
        size.setHeight(minimumSize.height());
    // A QWidget might have size -1,-1 if no geometry property is specified in the widget box.
    if (size.isEmpty())
        size = size.expandedTo(QSize(16, 16));
    w->setGeometry(QRect(QPoint(0, 0), size));
    result->resize(size);
    return result;
}

WidgetBoxDnDItem::WidgetBoxDnDItem(QDesignerFormEditorInterface *core,
                                   DomUI *dom_ui,
                                   const QPoint &global_mouse_pos) :
    QDesignerDnDItem(CopyDrop)
{
    DomWidget *child = dom_ui->elementWidget()->elementWidget().front();
    QWidget *decoration = decorationFromDomWidget(dom_ui, child, dom_ui->elementResources(), core);
    decoration->move(global_mouse_pos - QPoint(5, 5));

    init(dom_ui, 0, decoration, global_mouse_pos);
}
}

QT_END_NAMESPACE
