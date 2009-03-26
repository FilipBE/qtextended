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

class TestWidget : public QWidget
{
Q_OBJECT
public:
    TestWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
private slots:
    void setupWidgets();
};
#include "main.moc"

TestWidget::TestWidget(QWidget *parent, Qt::WindowFlags f)
 : QWidget(parent,f)
{
    setWindowTitle(QTOPIA_TARGET);
    QTimer::singleShot(0, this, SLOT(setupWidgets()));
}

void TestWidget::setupWidgets() {

    QTabWidget* tw = new QTabWidget;
    QPushButton* tab1 = new QPushButton("A Button", this);
    QPushButton* tab2 = new QPushButton("A Nother Button", this);

    tw->addTab(tab1, "Thirst");
    tw->addTab(tab2, "Sekond");

    QLineEdit* le = new QLineEdit;

    QFormLayout* form = new QFormLayout;
    form->addRow("Thing", le);
    form->addRow(tw);

    setLayout(form);

    le->setFocus();
}

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, TestWidget)
QTOPIA_MAIN

