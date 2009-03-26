/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#include "qdesigner.h"
#include "preferences.h"
#include "qdesigner_settings.h"
#include "qdesigner_workbench.h"
#include "qdesigner_propertyeditor.h"
#include "qdesigner_objectinspector.h"

#include <qdesigner_utils_p.h>
#include <previewconfigurationwidget_p.h>
#include <previewmanager_p.h>

#include <QtCore/QVariant>
#include <QtCore/QDir>

#include <QtGui/QDesktopWidget>
#include <QtGui/QStyle>
#include <QtGui/QListView>

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

static const char *designerPath = "/.designer";
static const char *newFormShowKey = "newFormDialog/ShowOnStartup";
static const char *mainWindowStateKey = "MainWindowState";
static const char *toolBoxStateKey = "ToolBoxState";
static const char *toolBarsStateKey = "ToolBarsState";
static const char *backupOrgListKey = "backup/fileListOrg";
static const char *backupBakListKey = "backup/fileListBak";
static const char *previewKey = "Preview";
static const char *defaultGridKey = "defaultGrid";
static const char *formTemplatePathsKey = "FormTemplatePaths";
static const char *recentFilesListKey = "recentFilesList";
static const char *actionEditorViewModeKey = "ActionEditorViewMode";
static const char *formTemplateKey = "FormTemplate";

static bool checkTemplatePath(const QString &path, bool create)
{
    QDir current(QDir::current());
    if (current.exists(path))
        return true;

    if (!create)
        return false;

    if (current.mkpath(path))
        return true;

    qdesigner_internal::designerWarning(QObject::tr("The template path %1 could not be created.").arg(path));
    return false;
}

QDesignerSettings::QDesignerSettings()
{
}

const QStringList &QDesignerSettings::defaultFormTemplatePaths()
{
    static QStringList rc;
    if (rc.empty()) {
        // Ensure default form template paths
        const QString templatePath = QLatin1String("/templates");
        // home
        QString path = QDir::homePath();
        path += QLatin1String(designerPath);
        path += templatePath;
        if (checkTemplatePath(path, true))
            rc += path;

        // designer/bin: Might be owned by root in some installations, do not force it.
        path = qDesigner->applicationDirPath();
        path += templatePath;
        if (checkTemplatePath(path, false))
            rc += path;
    }
    return rc;
}

QStringList QDesignerSettings::formTemplatePaths() const
{
    return value(QLatin1String(formTemplatePathsKey),defaultFormTemplatePaths()).toStringList();
}

void QDesignerSettings::setFormTemplatePaths(const QStringList &paths)
{
    setValue(QLatin1String(formTemplatePathsKey), paths);
}

QString  QDesignerSettings::formTemplate() const
{
    return value(QLatin1String(formTemplateKey)).toString();
}

void QDesignerSettings::setFormTemplate(const QString &t)
{
    setValue(QLatin1String(formTemplateKey), t);
}

void QDesignerSettings::saveGeometryFor(const QWidget *w)
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    saveGeometryHelper(w, w->objectName());
}

void QDesignerSettings::setGeometryFor(QWidget *w, const QRect &fallBack) const
{
    Q_ASSERT(w && !w->objectName().isEmpty());
    setGeometryHelper(w, w->objectName(),
                      fallBack.isNull() ? QRect(QPoint(0, 0), w->sizeHint()) : fallBack);
}

void QDesignerSettings::saveGeometryHelper(const QWidget *w, const QString &key)
{
    beginGroup(key);
    setValue(QLatin1String("visible"), w->isVisible());
QDesignerSettings::    setValue(QLatin1String("geometry"), w->saveGeometry());
    endGroup();
}

void QDesignerSettings::setGeometryHelper(QWidget *w, const QString &key,
                                          const QRect &fallBack) const
{
    QByteArray ba(value(key + QLatin1String("/geometry")).toByteArray());

    if (ba.isEmpty()) {
        w->move(fallBack.topLeft());
        w->resize(fallBack.size());
    } else {
        w->restoreGeometry(ba);
    }

    if (value(key + QLatin1String("/visible"), true).toBool())
        w->show();
}

QStringList QDesignerSettings::recentFilesList() const
{
    return value(QLatin1String(recentFilesListKey)).toStringList();
}

void QDesignerSettings::setRecentFilesList(const QStringList &sl)
{
    setValue(QLatin1String(recentFilesListKey), sl);
}

void QDesignerSettings::setShowNewFormOnStartup(bool showIt)
{
    setValue(QLatin1String(newFormShowKey), showIt);
}

bool QDesignerSettings::showNewFormOnStartup() const
{
    return value(QLatin1String(newFormShowKey), true).toBool();
}

QByteArray QDesignerSettings::mainWindowState() const
{
    return value(QLatin1String(mainWindowStateKey)).toByteArray();
}

void QDesignerSettings::setMainWindowState(const QByteArray &mainWindowState)
{
    setValue(QLatin1String(mainWindowStateKey), mainWindowState);
}

QByteArray QDesignerSettings::toolBoxState() const
{
    return value(QLatin1String(toolBoxStateKey)).toByteArray();
}

void QDesignerSettings::setToolBoxState(const QByteArray &state)
{
    setValue(QLatin1String(toolBoxStateKey), state);
}

QByteArray QDesignerSettings::toolBarsState() const
{
    return value(QLatin1String(toolBarsStateKey)).toByteArray();
}

void QDesignerSettings::setToolBarsState(const QByteArray &toolBarsState)
{
    setValue(QLatin1String(toolBarsStateKey), toolBarsState);
}

void QDesignerSettings::clearBackup()
{
    remove(QLatin1String(backupOrgListKey));
    remove(QLatin1String(backupBakListKey));
}

void QDesignerSettings::setBackup(const QMap<QString, QString> &map)
{
    const QStringList org = map.keys();
    const QStringList bak = map.values();

    setValue(QLatin1String(backupOrgListKey), org);
    setValue(QLatin1String(backupBakListKey), bak);
}

QMap<QString, QString> QDesignerSettings::backup() const
{
    const QStringList org = value(QLatin1String(backupOrgListKey), QStringList()).toStringList();
    const QStringList bak = value(QLatin1String(backupBakListKey), QStringList()).toStringList();

    QMap<QString, QString> map;
    const int orgCount = org.count();
    for (int i = 0; i < orgCount; ++i)
        map.insert(org.at(i), bak.at(i));

    return map;
}

void QDesignerSettings::setPreviewConfiguration(const qdesigner_internal::PreviewConfiguration &pc)
{
    pc.toSettings(QLatin1String(previewKey), *this);
}

qdesigner_internal::PreviewConfiguration QDesignerSettings::previewConfiguration() const
{
    qdesigner_internal::PreviewConfiguration rc;
    rc.fromSettings(QLatin1String(previewKey), *this);
    return rc;
}

qdesigner_internal::PreviewConfigurationWidgetState QDesignerSettings::previewConfigurationWidgetState() const
{
    qdesigner_internal::PreviewConfigurationWidgetState rc;
    rc.fromSettings(QLatin1String(previewKey), *this);
    return rc;
}

void QDesignerSettings::setPreviewConfigurationWidgetState(const qdesigner_internal::PreviewConfigurationWidgetState &pc)
{
    pc.toSettings(QLatin1String(previewKey), *this);
}

void QDesignerSettings::setPreferences(const Preferences& p)
{
    beginGroup(QLatin1String("UI"));
    setValue(QLatin1String("currentMode"), p.m_uiMode);
    setValue(QLatin1String("font"), p.m_font);
    setValue(QLatin1String("useFont"), p.m_useFont);
    setValue(QLatin1String("writingSystem"), p.m_writingSystem);
    endGroup();
    // grid
    setValue(QLatin1String(defaultGridKey), p.m_defaultGrid.toVariantMap());
    setPreviewConfigurationWidgetState(p.m_previewConfigurationWidgetState);
    setPreviewConfiguration(p.m_previewConfiguration);
    // merge template paths
    QStringList templatePaths = defaultFormTemplatePaths();
    templatePaths += p.m_additionalTemplatePaths;
    setFormTemplatePaths(templatePaths);
}

Preferences QDesignerSettings::preferences() const
{
    Preferences rc;
#ifdef Q_WS_WIN
    const UIMode defaultMode = DockedMode;
#else
    const UIMode defaultMode = TopLevelMode;
#endif
    rc.m_uiMode = static_cast<UIMode>(value(QLatin1String("UI/currentMode"), defaultMode).toInt());
    rc.m_writingSystem = static_cast<QFontDatabase::WritingSystem>(value(QLatin1String("UI/writingSystem"), QFontDatabase::Any).toInt());
    rc.m_font = qVariantValue<QFont>(value(QLatin1String("UI/font")));
    rc.m_useFont = value(QLatin1String("UI/useFont"), QVariant(false)).toBool();
    const QVariantMap defaultGridMap = value(QLatin1String(defaultGridKey), QVariantMap()).toMap();
    if (!defaultGridMap.empty())
        rc.m_defaultGrid.fromVariantMap(defaultGridMap);
    rc.m_additionalTemplatePaths = additionalFormTemplatePaths();
    rc.m_previewConfigurationWidgetState = previewConfigurationWidgetState();
    rc.m_previewConfiguration = previewConfiguration();
    return rc;
}

QStringList QDesignerSettings::additionalFormTemplatePaths() const
{
    // get template paths excluding internal ones
    QStringList rc = formTemplatePaths();
    foreach (QString internalTemplatePath, defaultFormTemplatePaths()) {
        const int index = rc.indexOf(internalTemplatePath);
        if (index != -1)
            rc.removeAt(index);
    }
    return rc;
}

int QDesignerSettings::actionEditorViewMode() const
{
    return value(QLatin1String(actionEditorViewModeKey), 0).toInt();
}

void QDesignerSettings::setActionEditorViewMode(int vm)
{
    setValue(QLatin1String(actionEditorViewModeKey), vm);
}

QT_END_NAMESPACE
