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

#include "qtestverifydlg_p.h"

#include <QLabel>
#include <QPushButton>
#include <QDesktopWidget>
#include <QLayout>
#include <QPixmap>
#include <QTabWidget>


#define WIDTH 364
#define HEIGHT 310

QTestVerifyDlg::QTestVerifyDlg( QWidget* parent )
    : QDialog( parent )
{
    resize( WIDTH, HEIGHT );

    QVBoxLayout *mainLayout = new QVBoxLayout( this );
    mainLayout->setMargin(11);
    mainLayout->setSpacing(6);

    questionLabel = new QLabel();
    questionLabel->setAlignment( Qt::AlignCenter );
    mainLayout->addWidget( questionLabel );

    QHBoxLayout *pixLayout = new QHBoxLayout;
    pixLayout->setMargin(0);
    pixLayout->setSpacing(6);

    leftTabWidget = new QTabWidget();
    leftTabWidget->setTabPosition( QTabWidget::South );
    actualLabel = new QLabel();
    QSizePolicy actualSizePolicy = QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    actualSizePolicy.setHorizontalStretch(0);
    actualSizePolicy.setVerticalStretch(0);
    actualSizePolicy.setHeightForWidth(actualLabel->sizePolicy().hasHeightForWidth());
    actualLabel->setSizePolicy( actualSizePolicy );
    actualLabel->setFrameShape( QLabel::Box );
    actualLabel->setFrameShadow( QLabel::Sunken );
    actualLabel->setScaledContents( false );
    actualLabel->setAlignment( Qt::AlignCenter );
    leftTabWidget->addTab( actualLabel, "actual" );
    pixLayout->addWidget( leftTabWidget );
    pixLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );

    rightTabWidget = new QTabWidget();
    rightTabWidget->setTabPosition( QTabWidget::South );
    expectedLabel = new QLabel();
    expectedLabel->setObjectName( "expectedLabel" );
    QSizePolicy expectedSizePolicy = QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    expectedSizePolicy.setHorizontalStretch(0);
    expectedSizePolicy.setVerticalStretch(0);
    expectedSizePolicy.setHeightForWidth(expectedLabel->sizePolicy().hasHeightForWidth());
    expectedLabel->setSizePolicy( expectedSizePolicy );
    expectedLabel->setFrameShape( QLabel::Box );
    expectedLabel->setFrameShadow( QLabel::Sunken );
    expectedLabel->setScaledContents( false );
    expectedLabel->setAlignment( Qt::AlignCenter );
    rightTabWidget->addTab( expectedLabel, "expected" );
    pixLayout->addWidget( rightTabWidget );
    mainLayout->addLayout( pixLayout );

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(6);
    okButton = new QPushButton();
    buttonLayout->addWidget( okButton );
    cancelButton = new QPushButton();
    buttonLayout->addWidget( cancelButton );
    buttonLayout->addItem( new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum ) );
    mainLayout->addLayout( buttonLayout );

    // signals and slots connections
    connect( okButton, SIGNAL(clicked()), this, SLOT(accept()), Qt::DirectConnection );
    connect( cancelButton, SIGNAL(clicked()), this, SLOT(reject()), Qt::DirectConnection );

    okButton->setText( "Yes" );
    cancelButton->setText( "No" );
    setWindowTitle( "Verify Pixmap" );
    questionText = "Does the LEFT pixmap present the correct output?\n ";
    questionLabel->setText(questionText);
}

/*
    Destroys the object and frees any allocated resources
*/
QTestVerifyDlg::~QTestVerifyDlg()
{
    //hide();
}

void QTestVerifyDlg::setData( const QPixmap &actual, const QPixmap &expected, const QString &comment)
{
    questionLabel->setText(questionText + comment);
    if (!actual.isNull())
        actualLabel->setPixmap(actual);

    bool expectedValid = true;
    if (!expected.isNull())
        expectedLabel->setPixmap(expected);
    else {
        QString S = "No expected pixmap available";
        expectedLabel->setText( S );
        QSize s1 = actualLabel->sizeHint();
        QSize s2 = expectedLabel->sizeHint();
        while (s1.width() > s2.width()) {
            S = " " + S + " ";
            expectedLabel->setText( S );
            s1 = actualLabel->sizeHint();
            s2 = expectedLabel->sizeHint();
        }
        S+="\n\nIf you click on 'Yes' the LEFT pixmap\nwill become the expected pixmap\nand will be used for future comparisons.";
        expectedLabel->setText( S );

        expectedValid = false;
    }

    /*bool equal = (actual->isNull() && expected->isNull());

    if (!equal) {
        if (!actual->isNull() && !expected->isNull()) {
            QByteArray actualDat;
            QDataStream a(&actualDat, QIODevice::ReadWrite);
            a << *actual;

            QByteArray expectedDat;
            QDataStream e(&expectedDat, QIODevice::ReadWrite);
            e << *expected;
            equal = (actualDat == expectedDat);
        }
    }

    if (!equal) {
        setWindowTitle("Verify Pixmaps (not Equal)");
    } else {
        setWindowTitle("Verify Pixmaps (Equal)");
    }*/

    updateGeometry();

    // try to make sure the parent and this widget don't overlap.
    /*QDesktopWidget *d = QApplication::desktop();
    int w = d->width();     // returns desktop width
    int h = d->height();    // returns desktop height
    QWidget *p;
    p = (QWidget*)parent();
    if (p != 0) {
        QRect rect = p->frameGeometry();
        int px = rect.x();
        int py = rect.y();
        int pw = rect.width();
        int ph = rect.height();

        int _x, _y;
        QSize size = sizeHint();
        int _w, _h;
        _w = size.width();
        _h = size.height();

        _x = (w / 2) - (_w / 2);

        if (_x < px + pw + 5) {
            _y = py + ph - _h;
            if (_y < 20)
                _y = 20;

            if (_w+5 <= px) {
                // place dialog LEFT of parent.
                setGeometry( px - (_w+5), _y, _w, _h );

            } else if (w - (_w+5) >= px + pw + 5) {
                // place dialog RIGHT of parent
                setGeometry( px + pw + 5, _y, _w, _h );
            }
        } else {
            // place dialog in the centre
            _y = (h / 2) - (_h / 2);
            if (_y < 20)
                _y = 20;
            setGeometry( _x, _y, _w, _h );
        }
    }*/
}
