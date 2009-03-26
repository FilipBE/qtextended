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

#include "slideshowdialog.h"

#include <qlayout.h>

SlideShowDialog::SlideShowDialog( QWidget* parent, Qt::WFlags f )
    : QDialog( parent, f )
{
#define LAYOUT_MARGIN 5
#define LAYOUT_SPACING 4
#define PAUSE_MIN 5
#define PAUSE_MAX 60

    setWindowTitle( tr( "Slide Show" ) );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    vbox->setMargin( LAYOUT_MARGIN );
    vbox->setSpacing( LAYOUT_SPACING );
    vbox->addStretch();

    // Construct pause between images label and slider
    slide_length_label = new QLabel( this );
    vbox->addWidget( slide_length_label );
    slide_length_slider = new QSlider( Qt::Horizontal, this );
    vbox->addWidget( slide_length_slider );
    connect( slide_length_slider, SIGNAL(valueChanged(int)),
        this, SLOT(updateSlideLengthLabel(int)) );
    slide_length_slider->setMinimum( PAUSE_MIN );
    slide_length_slider->setMaximum( PAUSE_MAX );

    // Construct display image name label and check box
    display_name_check = new QCheckBox( tr( "Display names" ), this );
    vbox->addWidget( display_name_check );

    // Construct loop through label and check box
    loop_through_check = new QCheckBox( tr( "Loop through" ), this );
    vbox->addWidget( loop_through_check );

    vbox->addStretch();
}

void SlideShowDialog::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    if (windowState() & Qt::WindowMaximized)
        setWindowState(windowState() & ~Qt::WindowMaximized | Qt::WindowMinimized);
}
