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

#ifndef CONFIG_H
#define CONFIG_H

#include "profile.h"

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QMap>

#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

QT_BEGIN_NAMESPACE

class Profile;

struct FontSettings
{
    FontSettings() : useWindowFont(false), useBrowserFont(false),
        windowWritingSystem(QFontDatabase::Latin), browserWritingSystem(QFontDatabase::Latin)
        { }

    QFont windowFont;
    QFont browserFont;

    bool useWindowFont;
    bool useBrowserFont;

    QFontDatabase::WritingSystem windowWritingSystem;
    QFontDatabase::WritingSystem browserWritingSystem;
};

class Config
{
public:

    Config();

    void load();
    void save();
    Profile *profile() const { return profil; }
    QString profileName() const { return profil->props[QLatin1String("name")]; }
    bool validProfileName() const;
    void hideSideBar( bool b );
    bool sideBarHidden() const;
    QStringList mimePaths();

    // From profile, read only
    QStringList docFiles() const;
    QStringList docTitles() const;
    QString indexPage( const QString &title ) const;
    QString docImageDir( const QString &title ) const;
    QPixmap docIcon( const QString &title ) const;

    QStringList profiles() const;
    QString title() const;
    QString aboutApplicationMenuText() const;
    QString aboutURL() const;
    QPixmap applicationIcon() const;

    // From QSettings, read / write
    QString homePage() const;
    void setHomePage( const QString &hom ) { home = hom; }

    QStringList source() const;
    void setSource( const QStringList &s ) { src = s; }

    int sideBarPage() const { return sideBar; }
    void setSideBarPage( int sbp ) { sideBar = sbp; }

    QByteArray windowGeometry() const { return winGeometry; }
    void setWindowGeometry( const QByteArray &geometry ) { winGeometry = geometry; }
    
    QByteArray mainWindowState() const { return mainWinState; }
    void setMainWindowState( const QByteArray &state ) { mainWinState = state; }

    qreal fontPointSize() const { return pointFntSize; }
    void setFontPointSize(qreal size)
    { 
        pointFntSize = size;
        m_fontSettings.useBrowserFont = true;
        m_fontSettings.browserFont.setPointSizeF(size); 
    }

    FontSettings fontSettings() { return m_fontSettings; }
    void setFontSettings(const FontSettings &settings) { m_fontSettings = settings; }

    QString assistantDocPath() const;

    bool docRebuild() const { return rebuildDocs; }
    void setDocRebuild( bool rb ) { rebuildDocs = rb; }

    void saveProfile( Profile *profile );
    void loadDefaultProfile();
    bool defaultProfileExists();

    static Config *configuration();
    static Config *loadConfig(const QString &profileFileName);

private:
    Config( const Config &c );
    Config& operator=( const Config &c );

    void saveSettings();
    
private:
    Profile *profil;

    QStringList profileNames;
    QString home;
    QStringList src;
    QByteArray mainWinState;
    QByteArray winGeometry;
    qreal pointFntSize;
    int sideBar;
    bool hideSidebar;
    bool rebuildDocs;
    FontSettings m_fontSettings;
};

QT_END_NAMESPACE

#endif // CONFIG_H
