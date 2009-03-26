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

#ifndef QDOCUMENTPROPERTIES_H
#define QDOCUMENTPROPERTIES_H

#include <qtopiaglobal.h>
#include <qcontent.h>
#include <QDialog>

class QContent;
class QFormLayout;
class QDocumentPropertiesWidgetPrivate;

class QTOPIA_EXPORT QDocumentPropertiesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QDocumentPropertiesWidget( const QContent &lnk, QWidget* parent = 0 );
    ~QDocumentPropertiesWidget();

    QContent document() const;

public slots:
    void applyChanges();

signals:
    void done();

protected slots:
    void beamLnk();
    void unlinkLnk();
    void duplicateLnk();
    void showLicenses();

private:
    bool moveLnk();
    void addRights( const QDrmRights &rights, QFormLayout *layout );
    QContent lnk;
    int fileSize;

    QString safePath( const QString &name, const QString &location, const QString &type, const QString &oldPath ) const;
    QString safeName(const QString &name, const QDir &directory, const QString &oldPath) const;

    QDocumentPropertiesWidgetPrivate *d;
};

class QTOPIA_EXPORT QDocumentPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QDocumentPropertiesDialog( const QContent &lnk, QWidget* parent = 0 );
    ~QDocumentPropertiesDialog();

    QContent document() const;

    void done(int);

private:
    QDocumentPropertiesWidget *d;
};

#endif
