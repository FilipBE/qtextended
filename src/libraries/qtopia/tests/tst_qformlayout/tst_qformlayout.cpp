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

#include <QObject>
#include <QTest>
#include <shared/qtopiaunittest.h>
#include <QFormLayout>
#include <QtopiaApplication>

#include <QtGui>
#include <QPhoneStyle>

//TESTED_CLASS=QFormLayout
//TESTED_FILES=src/libraries/qtopia/qformlayout.h

class tst_QFormLayout : public QObject
{
    Q_OBJECT

private slots:
    void rowCount();
    void buddies();
    void getPosition();
    void wrapping();
    void spacing();
    void contentsRect();
    //void formStyle();
};

//TODO: should set style before every test?

void tst_QFormLayout::rowCount()
{
    QWidget *w = new QWidget;
    QFormLayout *fl = new QFormLayout(w);

    fl->addRow(tr("Label 1"), new QLineEdit);
    fl->addRow(tr("Label 2"), new QLineEdit);
    fl->addRow(tr("Label 3"), new QLineEdit);
    QCOMPARE(fl->rowCount(), 3);

    fl->addRow(new QWidget);
    fl->addRow(new QHBoxLayout);
    QCOMPARE(fl->rowCount(), 5);

    fl->insertRow(1, tr("Label 0.5"), new QLineEdit);
    QCOMPARE(fl->rowCount(), 6);

    //TODO: remove items

    delete w;
}

void tst_QFormLayout::buddies()
{
    QWidget *w = new QWidget;
    QFormLayout *fl = new QFormLayout(w);

    //normal buddy case
    QLineEdit *le = new QLineEdit;
    fl->addRow(tr("Label 1"), le);
    QLabel *label = qobject_cast<QLabel *>(fl->labelForField(le));
    QVERIFY(label);
    QCOMPARE(label->buddy(), le);

    //null label
    QLineEdit *le2 = new QLineEdit;
    fl->addRow(0, le2);
    QWidget *label2 = fl->labelForField(le2);
    QVERIFY(label2 == 0);

    //no label
    QLineEdit *le3 = new QLineEdit;
    fl->addRow(le3);
    QWidget *label3 = fl->labelForField(le3);
    QVERIFY(label3 == 0);

    //TODO: empty label?

    delete w;
}

void tst_QFormLayout::getPosition()
{
    QWidget *w = new QWidget;
    QFormLayout *fl = new QFormLayout(w);

    QList<QLabel*> labels;
    QList<QLineEdit*> fields;
    for (int i = 0; i < 5; ++i) {
        labels.append(new QLabel(QString("Label %1").arg(i+1)));
        fields.append(new QLineEdit);
        fl->addRow(labels[i], fields[i]);
    }

    //a field
    {
        int row;
        QFormLayout::ItemRole role;
        fl->getWidgetPosition(fields[3], &row, &role);
        QCOMPARE(row, 3);
        QCOMPARE(role, QFormLayout::FieldRole);
        int old_row = row;
        QFormLayout::ItemRole old_role = role;
        fl->getItemPosition(3*2 + 1, &row, &role);
        QCOMPARE(row, old_row);
        QCOMPARE(role, old_role);
    }

    //a label
    {
        int row;
        QFormLayout::ItemRole role;
        fl->getWidgetPosition(labels[2], &row, &role);
        QCOMPARE(row, 2);
        QCOMPARE(role, QFormLayout::LabelRole);
        int old_row = row;
        QFormLayout::ItemRole old_role = role;
        fl->getItemPosition(2*2, &row, &role);
        QCOMPARE(row, old_row);
        QCOMPARE(role, old_role);
    }

    //a layout that's been inserted
    {
        QVBoxLayout *vbl = new QVBoxLayout;
        fl->insertRow(2, "Label 1.5", vbl);
        int row;
        QFormLayout::ItemRole role;
        fl->getLayoutPosition(vbl, &row, &role);
        QCOMPARE(row, 2);
        QCOMPARE(role, QFormLayout::FieldRole);
        int old_row = row;
        QFormLayout::ItemRole old_role = role;
        fl->getItemPosition(labels.count()*2 + 1, &row, &role);
        QCOMPARE(row, old_row);
        QCOMPARE(role, old_role);
    }

    delete w;
}

void tst_QFormLayout::wrapping()
{
    //TODO: ensure QtopiaDefaultStyle

    QWidget *w = new QWidget;
    QFormLayout *fl = new QFormLayout(w);

    QLineEdit *le = new QLineEdit;
    QLabel *lbl = new QLabel("A long label");
    le->setMinimumWidth(200);
    fl->addRow(lbl, le);

    w->setFixedWidth(240);
    w->show();

    QCOMPARE(le->geometry().y() > lbl->geometry().y(), true);

    //TODO: additional tests covering different wrapping cases

    delete w;
}

class CustomLayoutStyle : public QPhoneStyle
{
    Q_OBJECT
public:
    CustomLayoutStyle() : QPhoneStyle()
    {
        hspacing = 5;
        vspacing = 10;
    }

    virtual int pixelMetric(PixelMetric metric, const QStyleOption * option = 0,
                            const QWidget * widget = 0 ) const;

    int hspacing;
    int vspacing;
};

int CustomLayoutStyle::pixelMetric(PixelMetric metric, const QStyleOption * option /*= 0*/,
                                   const QWidget * widget /*= 0*/ ) const
{
    switch (metric) {
        case PM_LayoutHorizontalSpacing:
            return hspacing;
        case PM_LayoutVerticalSpacing:
            return vspacing;
        break;
        default:
            break;
    }
    return QPhoneStyle::pixelMetric(metric, option, widget);
}

void tst_QFormLayout::spacing()
{
    //TODO: confirm spacing behavior
    QWidget *w = new QWidget;
    CustomLayoutStyle *style = new CustomLayoutStyle;
    style->hspacing = 5;
    style->vspacing = 10;
    w->setStyle(style);
    QFormLayout *fl = new QFormLayout(w);
    QCOMPARE(style->hspacing, fl->horizontalSpacing());
    QCOMPARE(style->vspacing, fl->verticalSpacing());

    //QCOMPARE(fl->spacing(), -1);
    fl->setVerticalSpacing(5);
    QCOMPARE(5, fl->horizontalSpacing());
    QCOMPARE(5, fl->verticalSpacing());
    //QCOMPARE(fl->spacing(), 5);
    fl->setVerticalSpacing(-1);
    QCOMPARE(style->hspacing, fl->horizontalSpacing());
    QCOMPARE(style->vspacing, fl->verticalSpacing());

    style->hspacing = 5;
    style->vspacing = 5;
    //QCOMPARE(fl->spacing(), 5);


    fl->setHorizontalSpacing(20);
    //QCOMPARE(fl->spacing(), -1);
    style->vspacing = 20;
    QCOMPARE(fl->horizontalSpacing(), 20);
    QCOMPARE(fl->verticalSpacing(), 20);
    //QCOMPARE(fl->spacing(), 20);
    fl->setHorizontalSpacing(-1);
    //QCOMPARE(fl->spacing(), -1);
    style->hspacing = 20;
    //QCOMPARE(fl->spacing(), 20);

    delete w;
    delete style;
}

void tst_QFormLayout::contentsRect()
{
    QWidget w;
    QFormLayout form;
    w.setLayout(&form);
    form.addRow("Label", new QPushButton(&w));
    w.show();
/*#if defined(Q_WS_X11)
    qt_x11_wait_for_window_manager(&w);     // wait for the show
#endif*/
    int l, t, r, b;
    form.getContentsMargins(&l, &t, &r, &b);
    QRect geom = form.geometry();

    QCOMPARE(geom.adjusted(+l, +t, -r, -b), form.contentsRect());
}

QTEST_APP_MAIN(tst_QFormLayout, QtopiaApplication)
#include "tst_qformlayout.moc"
