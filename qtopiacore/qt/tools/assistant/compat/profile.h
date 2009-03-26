/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#ifndef PROFILE_H
#define PROFILE_H

#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QMap>

QT_BEGIN_NAMESPACE

class DocuParser;

class Profile
{
public:
    enum ProfileType { DefaultProfile, UserProfile };
    Profile();

    bool isValid() const;

    void addDCF( const QString &docfile );
    void addDCFIcon( const QString title, const QString &icon );
    void addDCFIndexPage( const QString title, const QString &indexPage );
    void addDCFImageDir( const QString title, const QString &imgDir );
    void addDCFTitle( const QString &dcf, const QString &title );
    void addProperty( const QString &name, const QString &value );
    bool hasDocFile( const QString &docFile );
    void removeDocFileEntry( const QString &title );

    ProfileType profileType() const { return type; }
    void setProfileType( ProfileType t ) { type = t; }

    DocuParser *docuParser() const { return dparser; }
    void setDocuParser( DocuParser *dp ) { dparser = dp; }

    static Profile* createDefaultProfile(const QString &docPath = QString());
    static QString makeRelativePath(const QString &base, const QString &path);
    static QString storableFilePath(const QString &fileName);
    static QString loadableFilePath(const QString &fileName);

    uint valid:1;
    ProfileType type;
    DocuParser *dparser;
    QMap<QString,QString> props;
    QMap<QString,QString> icons;
    QMap<QString,QString> indexPages;
    QMap<QString,QString> imageDirs;
    QMap<QString,QString> dcfTitles;
    QStringList docs;
};

QT_END_NAMESPACE

#endif // PROFILE_H
