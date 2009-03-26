/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QPICTUREFORMATPLUGIN_H
#define QPICTUREFORMATPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if !defined(QT_NO_LIBRARY) && !defined(QT_NO_PICTURE)

class QPicture;
class QImage;
class QString;
class QStringList;

struct Q_GUI_EXPORT QPictureFormatInterface : public QFactoryInterface
{
    virtual bool loadPicture(const QString &format, const QString &filename, QPicture *) = 0;
    virtual bool savePicture(const QString &format, const QString &filename, const QPicture &) = 0;

    virtual bool installIOHandler(const QString &) = 0;
};

#define QPictureFormatInterface_iid "com.trolltech.Qt.QPictureFormatInterface"
Q_DECLARE_INTERFACE(QPictureFormatInterface, QPictureFormatInterface_iid)


class Q_GUI_EXPORT QPictureFormatPlugin : public QObject, public QPictureFormatInterface
{
    Q_OBJECT
    Q_INTERFACES(QPictureFormatInterface:QFactoryInterface)
public:
    explicit QPictureFormatPlugin(QObject *parent = 0);
    ~QPictureFormatPlugin();

    virtual QStringList keys() const = 0;
    virtual bool loadPicture(const QString &format, const QString &filename, QPicture *pic);
    virtual bool savePicture(const QString &format, const QString &filename, const QPicture &pic);
    virtual bool installIOHandler(const QString &format) = 0;

};

#endif // QT_NO_LIBRARY || QT_NO_PICTURE

QT_END_NAMESPACE

QT_END_HEADER

#endif // QPICTUREFORMATPLUGIN_H
