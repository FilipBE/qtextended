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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

//QTEST_SKIP_TEST_DOC

#include <QtopiaApplication>
#include <QSignalMapper>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMenu>
#include <QSoftMenuBar>
#include <QAction>
#include <QTabWidget>
#include <QComboBox>
#include <QScrollArea>
#include <QTimer>
#include <QCheckBox>
#include <QTimeEdit>

class TestWidget : public QTabWidget
{
Q_OBJECT
public:
    TestWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
private slots:
    void setupWidgets();
    void lotsOfLogs() const;

private:
    QLineEdit *testEdit;
};
#include "main.moc"

TestWidget::TestWidget(QWidget *parent, Qt::WindowFlags f)
 : QTabWidget(parent)
{
    setWindowFlags(f);
    setWindowTitle(QTOPIA_TARGET);
    QTimer::singleShot(0, this, SLOT(setupWidgets()));
}

void TestWidget::setupWidgets() {
    {
        QWidget *page = new QWidget(this);
        QSignalMapper *sm = new QSignalMapper(page);

        QFormLayout *fl(new QFormLayout);

        QLineEdit *statusEdit = new QLineEdit;
        fl->addRow("Status", statusEdit);

        QLineEdit *le1   = new QLineEdit;
        QPushButton *pb1 = new QPushButton("Button1");
        QLineEdit *le2   = new QLineEdit;
        QPushButton *pb2 = new QPushButton("Button2");

        QPushButton *bye = new QPushButton("Close");
        connect(bye, SIGNAL(clicked()), this, SLOT(close()));

        QPushButton *pbLog = new QPushButton("Lots o' Logs");
        connect(pbLog, SIGNAL(clicked()), this, SLOT(lotsOfLogs()));

        fl->addRow("LineEdit1", le1);
        fl->addRow(pb1);
        fl->addRow("LineEdit2", le2);
        fl->addRow(pb2);
        fl->addRow(bye);
        fl->addRow(pbLog);

        QMenu *menu                = new QMenu(page);
        QAction *action1           = menu->addAction("Action 1", sm, SLOT(map()));
        QAction *action2           = menu->addAction("Action 2", sm, SLOT(map()));
        QAction *actionWithSubmenu = menu->addAction("Submenu",  sm, SLOT(map()));
        for ( int i = 1; i <= 8; ++i ) {
            QString menuName = QString("Menu Item #%1").arg(i);
            menu->addAction(menuName);
        }

        QAction *actionLast        = menu->addAction("Last!", sm, SLOT(map()));

        QMenu *submenu             = new QMenu(page);
        actionWithSubmenu->setMenu(submenu);
        QAction *subaction1        = submenu->addAction( "Subaction 1", sm, SLOT(map()) );
        QAction *subaction2        = submenu->addAction( "Subaction 2", sm, SLOT(map()) );

        QSoftMenuBar::addMenuTo(page, menu);

        connect(le1, SIGNAL(textChanged(QString)), sm, SLOT(map()));
        connect(le2, SIGNAL(textChanged(QString)), sm, SLOT(map()));
        connect(pb1, SIGNAL(clicked()),            sm, SLOT(map()));
        connect(pb2, SIGNAL(clicked()),            sm, SLOT(map()));

        sm->setMapping(le1, "LineEdit1 text changed");
        sm->setMapping(le2, "LineEdit2 text changed");
        sm->setMapping(pb1, "Button1 clicked");
        sm->setMapping(pb2, "Button2 clicked");
        sm->setMapping(action1, "Action 1 activated");
        sm->setMapping(action2, "Action 2 activated");
        sm->setMapping(actionWithSubmenu, "Submenu activated");
        sm->setMapping(actionLast, "Last activated");
        sm->setMapping(subaction1, "Subaction 1 activated");
        sm->setMapping(subaction2, "Subaction 2 activated");

        connect(sm, SIGNAL(mapped(QString)), statusEdit, SLOT(setText(QString)));

        page->setLayout(fl);

        addTab(page, "First Tab");
    }

    {
        QWidget *page = new QWidget;
        QSignalMapper *sm = new QSignalMapper(page);

        QFormLayout *fl(new QFormLayout);

        QLineEdit *statusEdit = new QLineEdit;
        fl->addRow("CowStatus", statusEdit);

        QComboBox *cb    = new QComboBox;
        cb->addItem("Neigh");
        cb->addItem("Woof");
        cb->addItem("Moo");
        cb->addItem("Meow");
        cb->addItem("Quack");
        cb->addItem("Honk");
        cb->addItem("Vrooooom");
        cb->addItem("\"Hi Frank\"");
        cb->addItem("Choo choo!");
        fl->addRow("Cow Goes?", cb);
        connect(cb, SIGNAL(activated(int)), sm, SLOT(map()));
        sm->setMapping(cb, "Cow Goes? changed");

        for (int i = 0; i < 20; ++i) {
            QString name( QString("CowButton%1").arg(i) );
            QPushButton *pb1 = new QPushButton(name);
            fl->addRow(pb1);
            connect(pb1, SIGNAL(clicked()), sm, SLOT(map()));
            sm->setMapping(pb1, name + " clicked");
        }

        connect(sm, SIGNAL(mapped(QString)), statusEdit, SLOT(setText(QString)));

        page->setLayout(fl);

        QScrollArea *sa = new QScrollArea(this);
        sa->setWidget(page);
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setFocusPolicy(Qt::NoFocus);
        sa->setFrameStyle(QFrame::NoFrame);
        sa->setWidgetResizable(true);
        addTab(sa, "Cow Tab");
    }

    {
        QWidget *page = new QWidget;
        QSignalMapper *sm = new QSignalMapper(page);

        QFormLayout *fl(new QFormLayout);

        QLineEdit *statusEdit = new QLineEdit;
        fl->addRow("Status", statusEdit);

        QCheckBox *cb    = new QCheckBox("Checkbox");
        cb->setChecked(false);
        fl->addRow(cb);

        QPushButton *pb  = new QPushButton("Clear");
        fl->addRow(pb);

        QTimeEdit *te    = new QTimeEdit;
        te->setTime(QTime(12, 30, 30));
        te->setDisplayFormat("hh:mm:ss");
        fl->addRow("Time", te);

        QDateEdit *de    = new QDateEdit;
        de->setDate(QDate(2001, 1, 1));
        fl->addRow("Date", de);

        connect(cb, SIGNAL(clicked()),         sm, SLOT(map()));
        connect(pb, SIGNAL(clicked()),         sm, SLOT(map()));
        connect(te, SIGNAL(editingFinished()), sm, SLOT(map()));
        connect(de, SIGNAL(editingFinished()), sm, SLOT(map()));
        sm->setMapping(cb, "'Checkbox' clicked");
        sm->setMapping(pb, "'Clear' clicked");
        sm->setMapping(te, "'Time' edited");
        sm->setMapping(de, "'Date' edited");

        connect(sm, SIGNAL(mapped(QString)), statusEdit, SLOT(setText(QString)));

        page->setLayout(fl);

        addTab(page, "Awesome Tab");
    }

    for (int i = 0; i < 5; ++i) {
        QWidget* widget = new QWidget(this);
        addTab(widget, QString("Distant Tab %1").arg(i));
    }
}

/*
    Output lots of log messages
    Used to test qtuitest's behavior/performance when huge amounts of log messages occur.
*/
void TestWidget::lotsOfLogs() const
{
    static int logCounter = 0;
    static const char messageBegin[] = "A log message";
    static const char messageEnd[] =
"- in fact, a really long log message.  In relative terms, a really really long one, I would say. "
"I would go so far as to say that, if a program were, say, reading these log messages as they "
"were written, and that program failed to correctly place a limit on its log buffer sizes, then "
"these messages might just happen to expose that problem.  In fact, it's not merely \"might\", "
"I would say that it is downright probable!"
    ;
    static const int  lines = 4000;

    for (int i = 0; i < lines; ++i) {
        QString str;
        str.sprintf("%s %d-%d %s", messageBegin, logCounter, i, messageEnd);
        qWarning("%s", qPrintable(str));
    }

    ++logCounter;
}

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, TestWidget)
QTOPIA_MAIN

