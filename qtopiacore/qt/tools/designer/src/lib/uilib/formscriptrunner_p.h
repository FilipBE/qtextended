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

#ifndef FORMSCRIPTRUNNER_H
#define FORMSCRIPTRUNNER_H

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

#include <QtDesigner/uilib_global.h>
#include <QtCore/QList>
#include <QtCore/QFlags>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QWidget;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class DomWidget;

class QDESIGNER_UILIB_EXPORT QFormScriptRunner
{
public:
    QFormScriptRunner();
    ~QFormScriptRunner();

    typedef QList<QWidget*> WidgetList;

    bool run(const DomWidget *domWidget,
             const QString &customWidgetScript,
             QWidget *widget, const WidgetList &children,
             QString *errorMessage);

    struct Error {
        QString objectName;
        QString script;
        QString errorMessage;
    };
    typedef QList<Error> Errors;
    Errors errors() const;
    void clearErrors();

    enum Option {
         NoOptions = 0x0,
         DisableWarnings = 0x1,
         DisableScripts = 02
     };
     Q_DECLARE_FLAGS(Options, Option)

    Options options() const;
    void setOptions(Options options);

private:
    class QFormScriptRunnerPrivate;
    QFormScriptRunnerPrivate *m_impl;

    QFormScriptRunner(const QFormScriptRunner &);
    void operator = (const QFormScriptRunner &);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFormScriptRunner::Options)

#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // FORMSCRIPTRUNNER_H
