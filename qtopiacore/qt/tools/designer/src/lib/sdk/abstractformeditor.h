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

#ifndef ABSTRACTFORMEDITOR_H
#define ABSTRACTFORMEDITOR_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>
#include <QtCore/QPointer>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDesignerWidgetBoxInterface;
class QDesignerPropertyEditorInterface;
class QDesignerFormWindowManagerInterface;
class QDesignerWidgetDataBaseInterface;
class QDesignerMetaDataBaseInterface;
class QDesignerWidgetFactoryInterface;
class QDesignerObjectInspectorInterface;
class QDesignerPromotionInterface;
class QDesignerBrushManagerInterface;
class QDesignerIconCacheInterface;
class QDesignerActionEditorInterface;
class QDesignerIntegrationInterface;
class QDesignerPluginManager;
class QDesignerIntrospectionInterface;
class QDesignerDialogGuiInterface;
class QtResourceModel;
class QtGradientManager;

class QWidget;

class QExtensionManager;

class  QDesignerFormEditorInterfacePrivate;

class QDESIGNER_SDK_EXPORT QDesignerFormEditorInterface : public QObject
{
    Q_OBJECT
public:
    QDesignerFormEditorInterface(QObject *parent = 0);
    virtual ~QDesignerFormEditorInterface();

    QExtensionManager *extensionManager() const;

    QWidget *topLevel() const;
    QDesignerWidgetBoxInterface *widgetBox() const;
    QDesignerPropertyEditorInterface *propertyEditor() const;
    QDesignerObjectInspectorInterface *objectInspector() const;
    QDesignerFormWindowManagerInterface *formWindowManager() const;
    QDesignerWidgetDataBaseInterface *widgetDataBase() const;
    QDesignerMetaDataBaseInterface *metaDataBase() const;
    QDesignerPromotionInterface *promotion() const;
    QDesignerWidgetFactoryInterface *widgetFactory() const;
    QDesignerBrushManagerInterface *brushManager() const;
    QDesignerIconCacheInterface *iconCache() const;
    QDesignerActionEditorInterface *actionEditor() const;
    QDesignerIntegrationInterface *integration() const;
    QDesignerPluginManager *pluginManager() const;
    QDesignerIntrospectionInterface *introspection() const;
    QDesignerDialogGuiInterface *dialogGui() const;
    QString resourceLocation() const;
    QtResourceModel *resourceModel() const;
    QtGradientManager *gradientManager() const;

    void setTopLevel(QWidget *topLevel);
    void setWidgetBox(QDesignerWidgetBoxInterface *widgetBox);
    void setPropertyEditor(QDesignerPropertyEditorInterface *propertyEditor);
    void setObjectInspector(QDesignerObjectInspectorInterface *objectInspector);
    void setPluginManager(QDesignerPluginManager *pluginManager);
    void setActionEditor(QDesignerActionEditorInterface *actionEditor);
    void setIntegration(QDesignerIntegrationInterface *integration);
    void setIntrospection(QDesignerIntrospectionInterface *introspection);
    void setDialogGui(QDesignerDialogGuiInterface *dialogGui);
    void setResourceModel(QtResourceModel *model);
    void setGradientManager(QtGradientManager *manager);

protected:
    void setFormManager(QDesignerFormWindowManagerInterface *formWindowManager);
    void setMetaDataBase(QDesignerMetaDataBaseInterface *metaDataBase);
    void setWidgetDataBase(QDesignerWidgetDataBaseInterface *widgetDataBase);
    void setPromotion(QDesignerPromotionInterface *promotion);
    void setWidgetFactory(QDesignerWidgetFactoryInterface *widgetFactory);
    void setExtensionManager(QExtensionManager *extensionManager);
    void setBrushManager(QDesignerBrushManagerInterface *brushManager);
    void setIconCache(QDesignerIconCacheInterface *cache);

private:
    QPointer<QWidget> m_pad1;
    QPointer<QDesignerWidgetBoxInterface> m_pad2;
    QPointer<QDesignerPropertyEditorInterface> m_pad3;
    QPointer<QDesignerFormWindowManagerInterface> m_pad4;
    QPointer<QExtensionManager> m_pad5;
    QPointer<QDesignerMetaDataBaseInterface> m_pad6;
    QPointer<QDesignerWidgetDataBaseInterface> m_pad7;
    QPointer<QDesignerWidgetFactoryInterface> m_pad8;
    QPointer<QDesignerObjectInspectorInterface> m_pad9;
    QPointer<QDesignerBrushManagerInterface> m_pad10;
    QPointer<QDesignerIconCacheInterface> m_pad11;
    QPointer<QDesignerActionEditorInterface> m_pad12;
    QDesignerFormEditorInterfacePrivate *d;

private:
    QDesignerFormEditorInterface(const QDesignerFormEditorInterface &other);
    void operator = (const QDesignerFormEditorInterface &other);
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // ABSTRACTFORMEDITOR_H
