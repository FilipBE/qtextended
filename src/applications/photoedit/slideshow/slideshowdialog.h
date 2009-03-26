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

#ifndef SLIDESHOWDIALOG_H
#define SLIDESHOWDIALOG_H

#include <qdialog.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qstring.h>

class SlideShowDialog : public QDialog {
    Q_OBJECT
public:
    SlideShowDialog( QWidget* parent = 0, Qt::WFlags f = 0 );

    // Return length of each slide in seconds
    int slideLength() { return slide_length_slider->value(); }

    // Return if true display name has been enabled
    bool isDisplayName() { return display_name_check->isChecked(); }

    // Return if true loop through has been enabled
    bool isLoopThrough() { return loop_through_check->isChecked(); }

private slots:
    // Update label with pause value
    void updateSlideLengthLabel( int sec ) { slide_length_label->setText(
        QString( tr("Slide length ( %1 sec. )") ).arg( sec ) ) ; }

protected:
    void resizeEvent(QResizeEvent *);

private:
    QLabel *slide_length_label;
    QSlider *slide_length_slider;
    QCheckBox *display_name_check, *loop_through_check;
};

#endif
