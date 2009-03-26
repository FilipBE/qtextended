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
#include <QtGui>
#include <QtCore>
#include <QFormLayout>

class TestWidget : public QTabWidget
{
Q_OBJECT
public:
    TestWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
private slots:
    void setupWidgets();
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
        QWidget *page(new QWidget);
        QFormLayout *fl(new QFormLayout);

        QComboBox *cb = new QComboBox;
        cb->addItems( QStringList() << "Off" << "Ask" << "On" );
        fl->addRow("Automatic", cb);

        cb = new QComboBox;
        cb->addItems( QStringList() << "New York" << "Los Angeles" );
        fl->addRow("Time Zone", cb);

        page->setLayout(fl);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidget(page);
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setFocusPolicy(Qt::NoFocus);
        sa->setFrameStyle(QFrame::NoFrame);
        sa->setWidgetResizable(true);
        addTab(sa, "Time");
    }

    {
        QWidget *page(new QWidget);
        QFormLayout *fl(new QFormLayout);

        QTextEdit *textEdit = new QTextEdit;
        fl->addRow("Text", textEdit);

        page->setLayout(fl);
        addTab(page, "Text");
    }

    {
        QWidget *page = new QWidget;
        QSignalMapper *sm = new QSignalMapper(page);

        QFormLayout *fl(new QFormLayout);

        QLineEdit *statusEdit = new QLineEdit;
        statusEdit->setFocusPolicy(Qt::NoFocus);
        fl->addRow("Status", statusEdit);

        QPushButton *button1 = new QPushButton("NoFocus1");
        button1->setFocusPolicy(Qt::NoFocus);
        button1->setShortcut(QKeySequence(Qt::Key_Select));
        connect(button1, SIGNAL(clicked()), sm, SLOT(map()));
        sm->setMapping(button1, "NoFocus1 clicked");
        fl->addRow(button1);

        QPushButton *button2 = new QPushButton("NoFocus2");
        button2->setFocusPolicy(Qt::NoFocus);
        button2->setShortcut(QKeySequence(Qt::Key_0));
        connect(button2, SIGNAL(clicked()), sm, SLOT(map()));
        sm->setMapping(button2, "NoFocus2 clicked");
        fl->addRow(button2);

        connect(sm, SIGNAL(mapped(QString)), statusEdit, SLOT(setText(QString)));
        page->setLayout(fl);

        QScrollArea *sa = new QScrollArea(this);
        sa->setWidget(page);
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setFocusPolicy(Qt::NoFocus);
        sa->setFrameStyle(QFrame::NoFrame);
        sa->setWidgetResizable(true);
        addTab(sa, "Shortcuts");
    }

}

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, TestWidget)
QTOPIA_MAIN

