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

#ifndef QTUITESTWIDGETS_P_H
#define QTUITESTWIDGETS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QMetaType>
#include <QHash>
#include <Qt>
#include <qtuitestnamespace.h>

class QtUiTestWidgetsPrivate;
class QtUiTestWidgetsService;
class QPoint;

class QtUiTestWidgets : public QObject
{
    Q_OBJECT

public:
    static QtUiTestWidgets* instance();

    virtual ~QtUiTestWidgets();

    void setInputOption(QtUiTest::InputOption,bool = true);
    bool testInputOption(QtUiTest::InputOption) const;

    QString errorString() const;
    void setErrorString(QString const&);

    void registerFactory(QtUiTest::WidgetFactory*);

    void mousePress  (QPoint const&,Qt::MouseButtons = Qt::LeftButton,
            QtUiTest::InputOption = QtUiTest::NoOptions);
    void mouseRelease(QPoint const&,Qt::MouseButtons = Qt::LeftButton,
            QtUiTest::InputOption = QtUiTest::NoOptions);
    void mouseClick  (QPoint const&,Qt::MouseButtons = Qt::LeftButton,
            QtUiTest::InputOption = QtUiTest::NoOptions);

    void keyPress  (Qt::Key,Qt::KeyboardModifiers = 0,QtUiTest::InputOption = QtUiTest::NoOptions);
    void keyRelease(Qt::Key,Qt::KeyboardModifiers = 0,QtUiTest::InputOption = QtUiTest::NoOptions);
    void keyClick  (Qt::Key,Qt::KeyboardModifiers = 0,QtUiTest::InputOption = QtUiTest::NoOptions);

private:
    Q_DISABLE_COPY(QtUiTestWidgets)

    friend class QtUiTestWidgetsPrivate;
    friend class QtUiTestWidgetsService;

    QtUiTestWidgetsPrivate* d;

    QtUiTestWidgets();
    QObject* findWidget(QtUiTest::WidgetType);
    QObject* testWidget(QObject*,QByteArray const&);

    void refreshPlugins();
    void clear();

    Q_PRIVATE_SLOT(d, void _q_objectDestroyed())
    Q_PRIVATE_SLOT(d, void _q_postNextKeyEvent())
    Q_PRIVATE_SLOT(d, void _q_postNextMouseEvent())

    friend QObject* QtUiTest::testWidget(QObject*,const char*);
    friend QObject* QtUiTest::findWidget(QtUiTest::WidgetType);
};

#endif

