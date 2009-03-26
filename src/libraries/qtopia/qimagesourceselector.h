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

#ifndef QIMAGESOURCESELECTOR_H
#define QIMAGESOURCESELECTOR_H

#include <qdialog.h>
#include <qcontent.h>

class QDSAction;

class QImageSourceSelector;
class QImageSourceSelectorDialogPrivate;

class QTOPIA_EXPORT QImageSourceSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    QImageSourceSelectorDialog( QWidget *parent );
    virtual ~QImageSourceSelectorDialog();

    void setMaximumImageSize( const QSize &s );
    QSize maximumImageSize() const;

    virtual void setContent( const QContent &image );
    virtual QContent content() const;

private:
    void init();

    QImageSourceSelector* selector;
    QImageSourceSelectorDialogPrivate* d;
};

#endif
