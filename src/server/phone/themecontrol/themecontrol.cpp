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

#include "themecontrol.h"
#include "themebackground_p.h"
#include "qabstractthemewidgetfactory.h"

#include <QtopiaChannel>
#include <QSettings>
#include <QValueSpace>
#include <QWaitWidget>
#include <QDebug>
#include <themedview.h>
#include <QThemedView>
#include <QThemeItemFactory>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <custom.h>
#include <QExportedBackground>

/*!
  \class ThemeControl
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \brief The ThemeControl class manages the registered theme views.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*! \internal */
ThemeControl::ThemeControl(QObject* parent)
: QObject(parent), m_exportBackground(false), m_widgetFactory(0)
{
    // Listen to system channel
    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(sysMessage(QString,QByteArray)) );
    
    refresh();
}

void ThemeControl::sysMessage(const QString& message, const QByteArray &data)
{
    QDataStream stream( data );
    if ( message == "applyStyleSplash()" ) {
        QWaitWidget *waitWidget = new QWaitWidget();
        waitWidget->setWindowFlags( Qt::WindowStaysOnTopHint | Qt::SplashScreen );
        waitWidget->show();
        qApp->processEvents();
        refresh();
        polishWindows();
        delete waitWidget;
    } else if ( message == "applyStyleNoSplash()" ) {
        qApp->processEvents();
        refresh();
        polishWindows();
    }
}

void ThemeControl::polishWindows()
{
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    QExportedBackground::polishWindows(0);
    //only polish secondary display if we have one
    QValueSpaceItem item("/System/ServerWidgets/QAbstractSecondaryDisplay");
    if ( !item.value().toString().isEmpty() ) {
        QExportedBackground::polishWindows(1);
    }
#endif
}

/*!
  Register a theme \a view with the given \a name.
  */
void ThemeControl::registerThemedView(ThemedView *view,
                                      const QString &name)
{
    m_themes.append(qMakePair(view, name));
    doTheme(view, name);
}

/*!
  Register a theme \a view with the given \a name.
  */
void ThemeControl::registerThemedView(QThemedView *view,
                                      const QString &name)
{
    m_qthemes.append(qMakePair(view, name));
    doTheme(view, name);
}

/*!
  Returns true if the theme configuration indicates that the background should
  be exported.
 */
bool ThemeControl::exportBackground() const
{
    return m_exportBackground;
}

/*!
  Update themed views.
  */
void ThemeControl::refresh()
{
    emit themeChanging();

    QSettings qpeCfg("Trolltech","qpe");
    qpeCfg.beginGroup("Appearance");
    m_exportBackground = qpeCfg.value("ExportBackground", true).toBool();

    QString themeFile = findFile(qpeCfg.value("Theme").toString()); // The server ensures this value is present and correct
    QSettings cfg(themeFile, QSettings::IniFormat);
    cfg.beginGroup("Theme");
    m_themeName = cfg.value("Name[]", "Unnamed").toString(); //we must use the untranslated theme name
    if ( m_themeName == "Unnamed" )
        qLog(I18n) << "Invalid theme name: Cannot load theme translations";

    // XXX hack around broken QSettings
    QString str = cfg.value("ContextConfig").toString();
    QStringList keys = cfg.childKeys();

    m_themeFiles.clear();
    QThemeItemFactory::clear();
    foreach(QString screen, keys)
        m_themeFiles.insert(screen, cfg.value(screen).toString());

    for(int ii = 0; ii < m_themes.count(); ++ii) {
        doTheme(m_themes.at(ii).first, m_themes.at(ii).second);
    }
    for(int ii = 0; ii < m_qthemes.count(); ++ii) {
        doTheme(m_qthemes.at(ii).first, m_qthemes.at(ii).second);
    }

    emit themeChanged();
}

/*!
    Sets the factory used to create theme widgets displayed in themes
    to \a factory.
*/
void ThemeControl::setThemeWidgetFactory(QAbstractThemeWidgetFactory *factory)
{
    delete m_widgetFactory;
    m_widgetFactory = factory;
    for (int ii = 0; ii < m_themes.count(); ++ii)
        doThemeWidgets(m_themes.at(ii).first);
}

/*!
  \fn ThemeControl::themeChanging()

  Emitted just before the theme is changed.
  */

/*!
  \fn ThemeControl::themeChanged()

  Emitted immediately after the theme changes.
 */

QString ThemeControl::findFile(const QString &file) const
{
    QStringList instPaths = Qtopia::installPaths();
    foreach (QString path, instPaths) {
        QString themeDataPath(path + QLatin1String("etc/themes/") + file);
        if (QFile::exists(themeDataPath)) {
            return themeDataPath;
        }
    }

    return QString();
}

void ThemeControl::doTheme(ThemedView *view, const QString &name)
{
    QString path = m_themeFiles[name + "Config"];
    if(!path.isEmpty()) {
        view->setThemeName(m_themeName);
        view->loadSource(findFile(path));
        doThemeWidgets(view);
    } else {
        qWarning("Invalid %s theme.", name.toAscii().constData());
    }
}

void ThemeControl::doTheme(QThemedView *view, const QString &name)
{
    QString path = m_themeFiles[name + "Config"];
    if(!path.isEmpty()) {
        view->setThemePrefix(m_themeName);
        view->load(findFile(path));
    } else {
        qWarning("Invalid %s theme.", name.toAscii().constData());
    }
}

void ThemeControl::doThemeWidgets(ThemedView *view)
{
    if (m_widgetFactory) {
        QList<ThemeItem*> items = view->findItems(QString(), ThemedView::Widget);
        QList<ThemeItem*>::ConstIterator it;
        for (it = items.begin(); it != items.end(); ++it)
            m_widgetFactory->createWidget((ThemeWidgetItem*)*it);
    }
}

QTOPIA_TASK(ThemeControl,ThemeControl);
QTOPIA_TASK_PROVIDES(ThemeControl,ThemeControl);
