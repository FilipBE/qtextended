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
#include <QTimer>
#include <QtopiaIpcEnvelope>
#include <QCallList>
#include <QGroupBox>
#include <QCheckBox>
#include <QTabWidget>
#include <QScrollArea>
#include <QLabel>
#include <QTimeEdit>
#include <QMenu>
#include <QDialog>

class TestWidget : public QTabWidget
{
Q_OBJECT
public:
    TestWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
private slots:
    void setupWidgets();
    void raiseCallHistory();
    void raiseDialer();
    void raiseTestapp1();
    void raiseDialog();
    void doPopup();
    void doDialog();
    void doPerflog();
    void setButtonText(const QString &text);
private:
    QLineEdit *popupEdit;
    QPushButton *menuButton;
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
        QSignalMapper *sm(new QSignalMapper(page));

        QLineEdit *statusEdit = new QLineEdit;
        fl->addRow("Status", statusEdit);
        QTimer::singleShot(0, statusEdit, SLOT(setFocus()));

        QPushButton *nothing      = new QPushButton("Do Nothing");
        QPushButton *callHistory  = new QPushButton("Show Call History");
        QPushButton *dialer       = new QPushButton("Show Dialer");
        QPushButton *testapp1     = new QPushButton("Show testapp1");
        QPushButton *dialog       = new QPushButton("Show Modal Dialog");
        connect(callHistory, SIGNAL(clicked()), this, SLOT(raiseCallHistory()));
        connect(dialer,      SIGNAL(clicked()), this, SLOT(raiseDialer()));
        connect(testapp1,    SIGNAL(clicked()), this, SLOT(raiseTestapp1()));
        connect(dialog,      SIGNAL(pressed()), this, SLOT(raiseDialog()));

        QGroupBox *checkableGroupBox = new QGroupBox;
        checkableGroupBox->setCheckable(true);
        checkableGroupBox->setChecked(false);
        checkableGroupBox->setTitle("Checkable");
        QPushButton *buttonInGroupBox = new QPushButton("Groupie");
        {
            QFormLayout *subFl(new QFormLayout);
            subFl->addRow(buttonInGroupBox);
            checkableGroupBox->setLayout(subFl);
        }

        /*
            Test that a focusable widget located in a noncheckable group box
            which contains no other focusable widgets can be referred to using
            the group box label.
        */
        QGroupBox *lineEditGroupBox = new QGroupBox;
        {
            lineEditGroupBox->setCheckable(false);
            lineEditGroupBox->setTitle("Bug206084");
            QHBoxLayout* hbox = new QHBoxLayout;
            hbox->addWidget(new QLineEdit);
            lineEditGroupBox->setLayout(hbox);
        }

        fl->addRow(nothing);
        fl->addRow(callHistory);
        fl->addRow(dialer);
        fl->addRow(testapp1);
        fl->addRow(dialog);
        fl->addRow(checkableGroupBox);
        fl->addRow(lineEditGroupBox);

        for (int i = 0; i < 5; ++i) {
            fl->addRow(new QPushButton(QString("Spacer %1").arg(i)));
        }

        QTimeEdit *te = new QTimeEdit;
        te->setTime(QTime(12, 30));
        te->setDisplayFormat("hh:mm");
        fl->addRow("Time", te);

        connect(nothing,           SIGNAL(clicked()),     sm, SLOT(map()));
        connect(dialer,            SIGNAL(clicked()),     sm, SLOT(map()));
        connect(callHistory,       SIGNAL(clicked()),     sm, SLOT(map()));
        connect(testapp1,          SIGNAL(clicked()),     sm, SLOT(map()));
        connect(checkableGroupBox, SIGNAL(clicked(bool)), sm, SLOT(map()));
        connect(buttonInGroupBox,  SIGNAL(clicked()),     sm, SLOT(map()));

        sm->setMapping(nothing,           "'Do Nothing' clicked");
        sm->setMapping(callHistory,       "'Show Call History' clicked");
        sm->setMapping(dialer,            "'Show Dialer' clicked");
        sm->setMapping(testapp1,          "'Show testapp1' clicked");
        sm->setMapping(checkableGroupBox, "'Checkable' clicked");
        sm->setMapping(buttonInGroupBox,  "'Groupie' clicked");

        connect(sm, SIGNAL(mapped(QString)), statusEdit, SLOT(setText(QString)));

        page->setLayout(fl);
        QScrollArea *sa = new QScrollArea(this);
        sa->setWidget(page);
        sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        sa->setFocusPolicy(Qt::NoFocus);
        sa->setFrameStyle(QFrame::NoFrame);
        sa->setWidgetResizable(true);
        addTab(sa, "Tab One");
    }

    {
        QWidget *page(new QWidget);
        QFormLayout *fl(new QFormLayout);

        QPushButton *messageButton(new QPushButton("Popup"));
        fl->addRow(messageButton);
        connect(messageButton, SIGNAL(clicked()), this, SLOT(doPopup()));

        QPushButton *dialogButton(new QPushButton("Dialog"));
        fl->addRow(dialogButton);
        connect(dialogButton, SIGNAL(clicked()), this, SLOT(doDialog()), Qt::QueuedConnection);

        popupEdit = new QLineEdit;
        fl->addRow("Popup response", popupEdit);

        QCheckBox *cb     = new QCheckBox("Checkbox");

        QLineEdit *sillyLineEdit1 = new QLineEdit;
        QLineEdit *sillyLineEdit2 = new QLineEdit;
        QLabel *sillyLabel1       = new QLabel("Silly 1");
        QLabel *sillyLabel2       = new QLabel("Silly 2");

        sillyLineEdit1->setVisible(false);
        sillyLineEdit2->setVisible(false);
        sillyLabel1->setVisible(false);
        sillyLabel2->setVisible(false);

        connect(cb, SIGNAL(toggled(bool)), sillyLineEdit1, SLOT(setVisible(bool)));
        connect(cb, SIGNAL(toggled(bool)), sillyLineEdit2, SLOT(setVisible(bool)));
        connect(cb, SIGNAL(toggled(bool)), sillyLabel1,    SLOT(setVisible(bool)));
        connect(cb, SIGNAL(toggled(bool)), sillyLabel2,    SLOT(setVisible(bool)));

        fl->addRow(cb);
        fl->addRow(sillyLabel1, sillyLineEdit1);
        fl->addRow(sillyLabel2, sillyLineEdit2);

        QGroupBox *gb = new QGroupBox("CheckGroup");
        gb->setCheckable(true);
        gb->setChecked(false);
        {
            QFormLayout *subFl(new QFormLayout);
            subFl->addRow(new QPushButton("Do Nothing"));
            gb->setLayout(subFl);
        }
        fl->addRow(gb);

        page->setLayout(fl);
        addTab(page, "Tab Two");
    }

    {
        QWidget *page(new QWidget);
        QFormLayout *fl(new QFormLayout);
        QSignalMapper *sm(new QSignalMapper(page));

        menuButton = new QPushButton("Menu");
        QMenu *menu(new QMenu);
        for ( int i = 1; i <= 6; ++i ) {
            QString menuName = QString("Menu Item #%1").arg(i);
            QAction *a = menu->addAction(menuName, sm, SLOT(map()));
            sm->setMapping(a, a->text());
        }
        menuButton->setMenu(menu);
        fl->addRow("Menu", menuButton);

        connect(sm, SIGNAL(mapped(QString)), this, SLOT(setButtonText(QString)));

        QTimeEdit *te(new QTimeEdit);
        te->setDisplayFormat("hh:mm:ss");
        te->setTime(QTime(12, 34, 56));
        fl->addRow("Time", te);

        QGroupBox *gb(new QGroupBox("Groupbox"));
        QVBoxLayout *vb(new QVBoxLayout);
        vb->addWidget( new QPushButton("Child Button 1") );
        vb->addWidget( new QPushButton("Child Button 2") );
        gb->setLayout(vb);
        fl->addRow(gb);

        page->setLayout(fl);
        addTab(page, "Tab Free");
    }

}

void TestWidget::raiseCallHistory() {
    QtopiaIpcEnvelope("QPE/Application/qpe", "CallHistory::showCallHistory(QCallList::ListType,QString)") << QCallList::All << QString();
}

void TestWidget::raiseDialer() {
    QtopiaIpcEnvelope("QPE/Application/qpe", "Dialer::showDialer(QString)") << "";
}

void TestWidget::raiseTestapp1() {
    QtopiaIpcEnvelope("QPE/Application/testapp1", "raise()");
}

void TestWidget::raiseDialog() {
    QDialog dlg;
    QPushButton pb("Close", &dlg);
    connect(&pb, SIGNAL(clicked()), &dlg, SLOT(accept()));
    dlg.exec();
}

void TestWidget::doPopup() {
    QString opt = (
     QMessageBox::warning(this, "A Message Box", "Please choose yes or no",
        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) ? "Yes" : "No";
    popupEdit->setText(opt);
}

void TestWidget::doDialog() {
    QDialog dlg;
    dlg.setWindowTitle("New Dialog");

    QPushButton closeButton("Close", &dlg);
    connect(&closeButton, SIGNAL(clicked()), &dlg, SLOT(accept()));

    QTimer::singleShot(50, this, SLOT(doPerflog()));
    dlg.exec();
}

void TestWidget::doPerflog() {
    // Send a performance log in an attempt to confuse the test.
    qLog(QtUitest) << "about to show dialog";
}

void TestWidget::setButtonText(const QString &text) {
    menuButton->setText(text);
}

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, TestWidget)
QTOPIA_MAIN

