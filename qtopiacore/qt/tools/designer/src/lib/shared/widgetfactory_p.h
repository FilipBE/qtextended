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


#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "shared_global_p.h"
#include "pluginmanager_p.h"

#include <QtDesigner/QDesignerWidgetFactoryInterface>

#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QObject;
class QWidget;
class QLayout;
class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT WidgetFactory: public QDesignerWidgetFactoryInterface
{
    Q_OBJECT
public:
    explicit WidgetFactory(QDesignerFormEditorInterface *core, QObject *parent = 0);
    ~WidgetFactory();

    virtual QWidget* containerOfWidget(QWidget *widget) const;
    virtual QWidget* widgetOfContainer(QWidget *widget) const;

    QObject* createObject(const QString &className, QObject* parent) const;

    virtual QWidget *createWidget(const QString &className, QWidget *parentWidget) const;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const;

    virtual bool isPassiveInteractor(QWidget *widget);
    virtual void initialize(QObject *object) const;

    virtual QDesignerFormEditorInterface *core() const;

    static QString classNameOf(QDesignerFormEditorInterface *core, QObject* o);

    QDesignerFormWindowInterface *currentFormWindow(QDesignerFormWindowInterface *fw);

    static QLayout *createUnmanagedLayout(QWidget *parentWidget, int type);
public slots:
    void loadPlugins();

private:
    struct Strings { // Reduce string allocations by storing predefined strings
        Strings();
        const QString m_alignment;
        const QString m_bottomMargin;
        const QString m_geometry;
        const QString m_leftMargin;
        const QString m_line;
        const QString m_objectName;
        const QString m_spacerName;
        const QString m_orientation;
        const QString m_q3WidgetStack;
        const QString m_qAction;
        const QString m_qAxWidget;
        const QString m_qDialog;
        const QString m_qDockWidget;
        const QString m_qLayoutWidget;
        const QString m_qMenu;
        const QString m_qMenuBar;
        const QString m_qWidget;
        const QString m_rightMargin;
        const QString m_sizeHint;
        const QString m_spacer;
        const QString m_text;
        const QString m_title;
        const QString m_topMargin;
        const QString m_windowIcon;
        const QString m_windowTitle;
    };

    QWidget* createCustomWidget(const QString &className, QWidget *parentWidget, bool *creationError) const;
    QDesignerFormWindowInterface *findFormWindow(QWidget *parentWidget) const;

    const Strings m_strings;
    QDesignerFormEditorInterface *m_core;
    typedef QMap<QString, QDesignerCustomWidgetInterface*> CustomWidgetFactoryMap;
    CustomWidgetFactoryMap m_customFactory;
    QDesignerFormWindowInterface *m_formWindow;

    static QPointer<QWidget> *m_lastPassiveInteractor;
    static bool m_lastWasAPassiveInteractor;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETFACTORY_H
