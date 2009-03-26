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

#ifndef ATTACHMENTOPTIONS_H
#define ATTACHMENTOPTIONS_H

#include <QByteArray>
#include <QContent>
#include <QDialog>
#include <QList>
#include <QSize>
#include <QString>

#include <qtopiaglobal.h>

class QByteArray;
class QContent;
class QLabel;
class QMailMessagePart;
class QPushButton;
class QString;

class AttachmentOptions : public QDialog
{
    Q_OBJECT

public:
    enum ContentClass
    {
        Text,
        Image,
        Media,
        Multipart,
        Other
    };

    AttachmentOptions(QWidget* parent);
    ~AttachmentOptions();

    QSize sizeHint() const;

public slots:
    void setAttachment(QMailMessagePart& part);

    void viewAttachment();
    void saveAttachment();

private:
    QSize _parentSize;
    QLabel* _name;
    QLabel* _type;
    //QLabel* _comment;
    QLabel* _sizeLabel;
    QLabel* _size;
    QPushButton* _view;
    QLabel* _viewer;
    QPushButton* _save;
    QLabel* _document;
    QMailMessagePart* _part;
    ContentClass _class;
    QString _decodedText;
    QByteArray _decodedData;
    QList<QContent> _temporaries;
};

#endif

