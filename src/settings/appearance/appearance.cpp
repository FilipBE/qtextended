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

#include "appearance.h"
#include "themedview.h"

#include <QStringList>
#include <QStringListModel>
#include <QSettings>
#include <QTranslatableSettings>
#include <QComboBox>
#include <QLabel>
#include <QPixmap>
#include <QFormLayout>
#include <QDesktopWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QDir>
#include <QList>
#include <QHash>
#include <QHashIterator>
#include <QScrollArea>
#include <QMenu>
#include <QTimer>

#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>
#include <QtopiaApplication>
#include <QSoftMenuBar>
#include <QtopiaChannel>
#include <QPhoneProfile>
#include <qtopianamespace.h>
#include <qtopialog.h>
#include <private/contextkeymanager_p.h>

QHash<QString, QString> appearance_defaultColorValues()
{
    QHash<QString, QString> values;
    values["Background"] = "#EEEEEE";
    values["Background_alpha"] = "64";
    values["Foreground"] = "#000000";
    values["Button"] = "#F0F0F0";
    values["Button_alpha"] = "176";
    values["Highlight"] = "#8BAF31";
    values["Highlight_alpha"] = "176";
    values["HighlightedText"] = "#FFFFFF";
    values["Text"] = "#000000";
    values["ButtonText"] = "#000000";
    values["ButtonText_disabled"] = "";
    values["ButtonText_disabled_alpha"] = "255";
    values["Base"] = "#FFFFFF";
    values["Base_alpha"] = "176";
    values["AlternateBase"] = "#CBEF71";
    values["AlternateBase_alpha"] = "176";
    values["Text_disabled"] = "";
    values["Text_disabled_alpha"] = "255";
    values["Foreground_disabled"] = "";
    values["Foreground_disabled_alpha"] = "255";
    values["Shadow"] = "";
    values["Link"] = "#0000FF";
    values["LinkVisited"] = "#FF00FF";
    return values;
}
static const QHash<QString, QString> gDefaultColorValues = appearance_defaultColorValues();

static QSettings gConfig("Trolltech", "qpe");


class ThemeItemPreview
{
public:
    ThemeItemPreview(const QString &pageItemName, const QString &xmlSourcePath);
    ~ThemeItemPreview();

    void setPalette(const QPalette &palette);
    void getPixmap(QPixmap *pixmap, int width, int height);
    void getPixmap(QPixmap *pixmap, int width, int height, QSoftMenuBar::LabelType labelType);

private:
    void resetButton(bool useIcon, int buttonIndex, QSoftMenuBar::StandardLabel label);

    QString m_pageItemName;
    QString m_xmlSourcePath;
    ThemedView *m_themedView;
};

ThemeItemPreview::ThemeItemPreview(const QString &pageItemName, const QString &xmlSourcePath)
    : m_pageItemName(pageItemName),
      m_xmlSourcePath(xmlSourcePath),
      m_themedView(new ThemedView)
{
}

ThemeItemPreview::~ThemeItemPreview()
{
    delete m_themedView;
}

void ThemeItemPreview::setPalette(const QPalette &palette)
{
    m_themedView->setPalette(palette);
}

void ThemeItemPreview::getPixmap(QPixmap *pixmap, int width, int height)
{
    getPixmap(pixmap, width, height, QSoftMenuBar::LabelType(-1));
}

void ThemeItemPreview::getPixmap(QPixmap *pixmap, int width, int height, QSoftMenuBar::LabelType labelType)
{
    m_themedView->loadSource(m_xmlSourcePath);
    m_themedView->resize(width, height);

    if (labelType != QSoftMenuBar::LabelType(-1)) {
        bool useIcon = (labelType == QSoftMenuBar::IconLabel);
        QList<QSoftMenuBar::StandardLabel> labels;
        if (QApplication::isLeftToRight())
            labels << QSoftMenuBar::Options << QSoftMenuBar::Select << QSoftMenuBar::Back;
        else
            labels << QSoftMenuBar::Back << QSoftMenuBar::Select << QSoftMenuBar::Options;
        for (int i=0; i<labels.size(); i++)
            resetButton(useIcon, i, labels[i]);
    }

    ThemeItem *page = m_themedView->findItem(m_pageItemName, ThemedView::Page);
    if (page)
        *pixmap = QPixmap::grabWidget(m_themedView, page->rect());
}

void ThemeItemPreview::resetButton(bool useIcon, int buttonIndex, QSoftMenuBar::StandardLabel label)
{
    ThemeTextItem *textItem = (ThemeTextItem *)m_themedView->findItem(
            "tbutton" + QString::number(buttonIndex) , ThemedView::Text);
    ThemeImageItem *imageItem = (ThemeImageItem *)m_themedView->findItem(
            "button" + QString::number(buttonIndex), ThemedView::Image);

    ContextKeyManager *mgr = ContextKeyManager::instance();
    if (!useIcon) {
        if (textItem) {
           textItem->setVisible(true);
           textItem->setText(mgr->standardText(label));
        }
        if (imageItem) {
            imageItem->setVisible(false);
        }
    } else {
        if (textItem) {
            textItem->setVisible(false);
        }
        if (imageItem) {
            imageItem->setVisible(true);
            imageItem->setImage(QPixmap(":icon/" + mgr->standardPixmap(label)));
        }
    }
}

//==============================================================================

class Theme
{
public:
    enum StringAttribute {
        StyleName,
        ServerWidgets,
        DecorationConfig,
        ExtendedFocusHighlight,
        FormStyle,
        PopupShadows,
        HideMenuIcons,
        FullWidthMenu,
        IconPath,
        MenuAlignment
    };

    ~Theme();

    inline const QString &uniqueName() const { return m_uniqueName; }
    inline const QString &name() const { return m_name; }

    QString stringValue(StringAttribute attr) const;
    const QStringList &colorSchemeNames() const;
    const QStringList &colorSchemeFiles() const;
    const QStringList &backgrounds() const;

    void setCurrentColorIndex(int index);
    int currentColorIndex() const;
    void setCurrentBackgroundIndex(int index);
    int currentBackgroundIndex() const;

    void getTitlePreview(QPixmap *pixmap);
    void getSoftMenuBarPreview(QPixmap *pixmap, QSoftMenuBar::LabelType labelType);
    void getBackgroundPreview(QPixmap *pixmap);

    void writeThemeSettings(bool writeAppearanceSettings);
    void writeColorSchemeSettings();

    static Theme *create(const QString &settingsPath, const QString &themeUniqueName);
    static void setAvailableColorSchemes(const QHash<QString, QString> &schemes);

private:
    Theme(const QString &uniqueName, QTranslatableSettings *settings);
    void ensureLoaded();

    QString m_uniqueName;
    QString m_name;
    QTranslatableSettings *m_settings;
    bool m_loaded;

    QStringList m_colorSchemeFiles;
    QStringList m_colorSchemeNames;
    QStringList m_backgrounds;
    QHash<StringAttribute, QString> m_attrs;

    int m_colorIndex;
    int m_backgroundIndex;

    ThemeItemPreview *m_titlePreview;
    ThemeItemPreview *m_softMenuBarPreview;
    QPalette m_palette;

    static QHash<QString, QString> availableColorSchemes;
};

QHash<QString, QString> Theme::availableColorSchemes;

Theme::Theme(const QString &uniqueName, QTranslatableSettings *settings)
    : m_uniqueName(uniqueName),
      m_settings(settings),
      m_loaded(false),
      m_colorIndex(-1),
      m_backgroundIndex(-1),
      m_titlePreview(0),
      m_softMenuBarPreview(0)
{
    settings->beginGroup(QLatin1String("Theme"));
    m_name = m_settings->value("Name").toString();
}

Theme::~Theme()
{
    delete m_settings;
    delete m_titlePreview;
    delete m_softMenuBarPreview;
}

void Theme::ensureLoaded()
{
    if (m_loaded)
        return;

    m_attrs[StyleName] = m_settings->value("Style", "Qtopia").toString();
    m_attrs[ServerWidgets] = m_settings->value("ServerWidget", "Phone").toString();
    m_attrs[DecorationConfig] = m_settings->value("DecorationConfig").toString();
    m_attrs[ExtendedFocusHighlight] = m_settings->value("ExtendedFocusHighlight", "1").toString();

    m_attrs[FormStyle] = m_settings->value("FormStyle", "QtopiaDefaultStyle").toString();
    m_attrs[PopupShadows] = m_settings->value("PopupShadows", "0").toString();
    m_attrs[HideMenuIcons] = m_settings->value("HideMenuIcons", "0").toString();
    m_attrs[FullWidthMenu] = m_settings->value("FullWidthMenu", "0").toString();
    m_attrs[IconPath] = m_settings->value("IconPath").toString();
    m_attrs[MenuAlignment] = m_settings->value("MenuAlignment", "left").toString();

    // ensure we only provide schemes that actually have a corresponding .scheme file
    QStringList colorSchemes = m_settings->value("ColorScheme").toString()
            .split("|", QString::SkipEmptyParts);
    for (int i=0; i<colorSchemes.size(); i++) {
        if (Theme::availableColorSchemes.contains(colorSchemes[i])) {
            m_colorSchemeFiles.append(colorSchemes[i]);
            QString schemePath = Theme::availableColorSchemes[colorSchemes[i]];
            QTranslatableSettings settings(schemePath, QSettings::IniFormat);
            QString colorName = settings.value("Global/Name").toString();
            if (colorName.isEmpty())
                colorName = colorSchemes[i].left(colorSchemes[i].lastIndexOf(".scheme"));
            m_colorSchemeNames << colorName;
        }
    }
    if (m_colorSchemeFiles.size() > 0)
        m_colorIndex = 0;

    m_backgrounds = m_settings->value("Backgrounds").toString().split("|", QString::SkipEmptyParts);
    if (m_backgrounds.size() > 0)
        m_backgroundIndex = 0;

    m_loaded = true;
}

QString Theme::stringValue(StringAttribute attr) const
{
    const_cast<Theme *>(this)->ensureLoaded();
    return m_attrs.value(attr, QString());
}

const QStringList &Theme::colorSchemeNames() const
{
    const_cast<Theme *>(this)->ensureLoaded();
    return m_colorSchemeNames;
}

const QStringList &Theme::colorSchemeFiles() const
{
    const_cast<Theme *>(this)->ensureLoaded();
    return m_colorSchemeFiles;
}

const QStringList &Theme::backgrounds() const
{
    const_cast<Theme *>(this)->ensureLoaded();
    return m_backgrounds;
}

void Theme::setCurrentColorIndex(int index)
{
    ensureLoaded();

    QString schemePath = Theme::availableColorSchemes.value(m_colorSchemeFiles.value(index));
    if (schemePath.isEmpty())
        return;

    QSettings scheme(schemePath, QSettings::IniFormat);
    scheme.beginGroup("Colors");
    m_palette.setColor(QPalette::Normal, QPalette::Window, scheme.value("Background").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Button, scheme.value("Button").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Highlight, scheme.value("Highlight").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Text, scheme.value("Text").toString());
    m_palette.setColor(QPalette::Normal, QPalette::Base, scheme.value("Base").toString());

    if (m_titlePreview)
        m_titlePreview->setPalette(m_palette);
    if (m_softMenuBarPreview)
        m_softMenuBarPreview->setPalette(m_palette);

    m_colorIndex = index;
}

int Theme::currentColorIndex() const
{
    const_cast<Theme *>(this)->ensureLoaded();
    return m_colorIndex;
}

void Theme::setCurrentBackgroundIndex(int index)
{
    ensureLoaded();

    if (index < 0 || index > m_backgrounds.size())
        return;
    m_backgroundIndex = index;
}

int Theme::currentBackgroundIndex() const
{
     const_cast<Theme *>(this)->ensureLoaded();
     return m_backgroundIndex;
}

void Theme::getTitlePreview(QPixmap *pixmap)
{
    ensureLoaded();
    if (!m_titlePreview) {
        m_titlePreview = new ThemeItemPreview("title",
                AppearanceSettings::findFile(QLatin1String("etc/themes/") + m_uniqueName + "/title.xml"));
        m_titlePreview->setPalette(m_palette);
    }

    QDesktopWidget *dw = QApplication::desktop();
    QSize screenSize = dw->screenGeometry(dw->primaryScreen()).size();
    m_titlePreview->getPixmap(pixmap, screenSize.width(), int(screenSize.height() * 0.09));
}

void Theme::getSoftMenuBarPreview(QPixmap *pixmap, QSoftMenuBar::LabelType labelType)
{
    ensureLoaded();
    if (!m_softMenuBarPreview) {
        m_softMenuBarPreview = new ThemeItemPreview("contextbar",
                AppearanceSettings::findFile(QLatin1String("etc/themes/") + m_uniqueName + "/context.xml"));
        m_softMenuBarPreview->setPalette(m_palette);
    }

    QDesktopWidget *dw = QApplication::desktop();
    QSize screenSize = dw->screenGeometry(dw->primaryScreen()).size();
    m_softMenuBarPreview->getPixmap(pixmap, screenSize.width(), int(screenSize.height() * 0.15), labelType);
}

void Theme::getBackgroundPreview(QPixmap *pixmap)
{
    ensureLoaded();
    QString fileName = AppearanceSettings::findFile("pics/themes/"
        + m_uniqueName + '/' + m_backgrounds.value(m_backgroundIndex) + ".png");

    QPixmap bg(fileName);
    *pixmap = bg;
}

void Theme::writeThemeSettings(bool writeAppearanceSettings)
{
    gConfig.beginGroup("Appearance");
    QString themeFileName = m_uniqueName + ".conf";
    if (writeAppearanceSettings){
        qLog(UI) << "Write config theme select"
                << m_attrs.value(Theme::StyleName).toLatin1().data()
                << m_name.toLatin1().data();
        gConfig.setValue("Style", m_attrs.value(Theme::StyleName));
        gConfig.setValue("Theme", themeFileName);
        gConfig.setValue("DecorationTheme", m_attrs.value(Theme::DecorationConfig));
    } else {
        qLog(UI) << "Write simple config theme select"
                << m_attrs.value(Theme::StyleName).toLatin1().data()
                << m_name.toLatin1().data();
        gConfig.setValue("Style", themeFileName);
        gConfig.setValue("Theme", "");
        gConfig.setValue("DecorationTheme", "");
    }
    gConfig.endGroup();

    gConfig.beginGroup("Style");
    gConfig.setValue("ExtendedFocusHighlight", m_attrs.value(ExtendedFocusHighlight));
    gConfig.setValue("FormStyle", m_attrs.value(FormStyle));
    gConfig.setValue("PopupShadows", m_attrs.value(PopupShadows));
    gConfig.setValue("HideMenuIcons", m_attrs.value(HideMenuIcons));
    gConfig.setValue("FullWidthMenu", m_attrs.value(FullWidthMenu));
    gConfig.setValue("MenuAlignment", m_attrs.value(MenuAlignment));
    gConfig.endGroup();

    gConfig.sync();
}

void Theme::writeColorSchemeSettings()
{
    if (m_colorIndex < 0 || m_colorIndex > m_colorSchemeFiles.size())
        return;

    gConfig.beginGroup("Appearance");
    gConfig.setValue("Scheme", m_colorSchemeFiles[m_colorIndex]);

    // copy colour settings from .scheme file to gConfig
    QString schemeFile = Theme::availableColorSchemes.value(m_colorSchemeFiles[m_colorIndex]);
    QSettings scheme(schemeFile, QSettings::IniFormat);
    if (scheme.status() == QSettings::NoError){
        scheme.beginGroup("Colors");
        QHashIterator<QString, QString> i(gDefaultColorValues);
        while (i.hasNext()) {
            i.next();
            gConfig.setValue(i.key(), scheme.value(i.key(), i.value()).toString());
        }
    }
    gConfig.endGroup();
    gConfig.sync();
}

Theme *Theme::create(const QString &settingsPath, const QString &themeUniqueName)
{
    QTranslatableSettings *settings = new QTranslatableSettings(settingsPath, QSettings::IniFormat);
    if (settings->status() != QSettings::NoError) {
        qLog(UI) << "Cannot load theme at" << settingsPath.toLocal8Bit().data();
        delete settings;
        return 0;
    }

    settings->beginGroup(QLatin1String("Theme"));
    if (settings->value("Name").toString().isEmpty()) {
        qLog(UI) << "Cannot load theme at" << settingsPath.toLocal8Bit().data() << "(no theme name)";
        delete settings;
        return 0;
    }
    settings->endGroup();

    return new Theme(themeUniqueName, settings);
}

void Theme::setAvailableColorSchemes(const QHash<QString, QString> &schemes)
{
    availableColorSchemes = schemes;
}


//===================================================================

AppearanceSettings::AppearanceSettings(QWidget* parent, Qt::WFlags fl)
    : QDialog(parent, fl),
      m_isStatusView(false)
{
    initUi();
    loadThemes();
    loadColorSchemes();
    loadSavedTheme();

    connect(qApp, SIGNAL(appMessage(QString,QByteArray)),
        this, SLOT(receiveAppMessage(QString,QByteArray)));

    QMenu *menu = QSoftMenuBar::menuFor(this);
    menu->addAction(QIcon(":icon/Note"), tr("Add to current profile"),
                    this, SLOT(pushSettingStatus()));
    menu->addAction(QIcon(":image/homescreen/homescreen"), tr("Homescreen Settings..."),
                    this, SLOT(openHomescreenSettings()));

    setWindowTitle(tr("Appearance"));
}

AppearanceSettings::~AppearanceSettings()
{
    qDeleteAll(m_themes);
}

Theme *AppearanceSettings::currentTheme() const
{
    return m_themes.value(m_themeCombo->currentIndex(), 0);
}

void AppearanceSettings::themeChanged(int index)
{
    Theme *theme = m_themes.value(index, 0);
    if (!theme)
        return;

    QStringListModel *model;

    model = qobject_cast<QStringListModel *>(m_colorCombo->model());
    model->setStringList(theme->colorSchemeNames());
    m_colorCombo->setCurrentIndex(theme->currentColorIndex());

    model = qobject_cast<QStringListModel *>(m_backgroundCombo->model());
    model->setStringList(theme->backgrounds());
    m_backgroundCombo->setCurrentIndex(theme->currentBackgroundIndex());
}

void AppearanceSettings::colorChanged(int index)
{
    Theme *theme = currentTheme();
    if (theme) {
        theme->setCurrentColorIndex(index);
        QTimer::singleShot(0, this, SLOT(previewColorChanges()));
    }
}

void AppearanceSettings::backgroundChanged(int index)
{
    Theme *theme = currentTheme();
    if (theme) {
        theme->setCurrentBackgroundIndex(index);
        QTimer::singleShot(0, this, SLOT(previewBackgroundChanges()));
    }
}

void AppearanceSettings::softKeyOptionChanged()
{
    QTimer::singleShot(0, this, SLOT(previewSoftMenuBarChanges()));
}

void AppearanceSettings::previewColorChanges()
{
    Theme *theme = currentTheme();
    if (theme) {
        QPixmap pm;
        theme->getTitlePreview(&pm);
        m_previewTitle->setPixmap(
                pm.scaledToWidth(int(pm.width() * 0.55), Qt::SmoothTransformation));

        previewSoftMenuBarChanges();
        m_previewBox->show();
    }
}

void AppearanceSettings::previewSoftMenuBarChanges()
{
    Theme *theme = currentTheme();
    if (theme) {
        QPixmap pm;
        QSoftMenuBar::LabelType labelType = (m_softKeyIconCheck->isChecked() ?
                QSoftMenuBar::IconLabel : QSoftMenuBar::TextLabel);
        theme->getSoftMenuBarPreview(&pm, labelType);
        m_previewSoftMenuBar->setPixmap(
                pm.scaledToWidth(int(pm.width() * 0.55), Qt::SmoothTransformation));
        m_previewBox->show();
    }
}

void AppearanceSettings::previewBackgroundChanges()
{
    Theme *theme = currentTheme();
    if (theme) {
        QDesktopWidget *dw = QApplication::desktop();
        QSize screenSize = dw->screenGeometry(dw->primaryScreen()).size();
        QPixmap pm;
        theme->getBackgroundPreview(&pm);
        m_previewBackground->setPixmap(
                pm.scaled(int(screenSize.width() * 0.2),
                          int(screenSize.height() * 0.2),
                          Qt::IgnoreAspectRatio,
                          Qt::SmoothTransformation));
        m_previewBox->show();
    }
}

void AppearanceSettings::accept()
{
    if (!m_isStatusView) { // normal appearance setting operation
        QPhoneProfileManager profileManager;
        QPhoneProfile activeProfile = profileManager.activeProfile();
        QPhoneProfile::Setting setting = activeProfile.applicationSetting("appearance");
        if (setting != QPhoneProfile::Setting())
            pushSettingStatus();
    } else { // status view from profiles
        // save current status to the profile
        pushSettingStatus();
    }

    applyCurrentSettings();
    QDialog::accept();
}

bool AppearanceSettings::themeNeedsRestart(Theme *theme) const
{
    if (!theme)
        return false;

    // If the server widgets or IconPath changes we need to restart for
    // all changes to be visible.
    QString serverWidgets;
    QSettings serverWidgetsConfig("Trolltech", "ServerWidgets");
    serverWidgetsConfig.beginGroup("Mapping");
    if (!serverWidgetsConfig.childKeys().isEmpty())
        serverWidgets = serverWidgetsConfig.value("Default", "Phone").toString();

    QString themeFile = findFile(QLatin1String("etc/themes/") + theme->uniqueName() + ".conf");
    QSettings settings(themeFile, QSettings::IniFormat);
    settings.beginGroup(QLatin1String("Theme"));
    QString activeIconPath = settings.value(QLatin1String("IconPath")).toString();

    return (serverWidgets != theme->stringValue(Theme::ServerWidgets)
            || activeIconPath != theme->stringValue(Theme::IconPath));
}

void AppearanceSettings::applyCurrentSettings()
{
    Theme *theme = currentTheme();
    if (!theme)
        return;

    bool needsRestart = themeNeedsRestart(theme);
    if (needsRestart) {
        if (QMessageBox::warning(this, tr("Restart?"),
                    tr("Device will be restarted for theme to be fully applied.<br>Apply Now?"),
                    QMessageBox::Yes, QMessageBox::No)
                != QMessageBox::Yes) {
            return;
        }
    }

    bool themeChanged = (theme->uniqueName() + ".conf" != m_savedTheme);
    bool colorSchemeChanged = (theme->colorSchemeNames()[theme->currentColorIndex()] + ".scheme"
            != m_savedColorScheme);
    bool backgroundChanged = (theme->backgrounds()[theme->currentBackgroundIndex()]
            != m_savedBackground);
    bool softKeyLabelTypeChanged = (m_softKeyIconCheck->isChecked() ?
            QSoftMenuBar::IconLabel : QSoftMenuBar::TextLabel) != m_savedLabelType;

    qLog(UI) << "Theme settings changed?" << themeChanged << colorSchemeChanged
            << backgroundChanged <<softKeyLabelTypeChanged;

    // write config settings
    if (themeChanged)
        theme->writeThemeSettings(theme->uniqueName() + ".conf" != m_themeCombo->currentText());
    if (colorSchemeChanged)
        theme->writeColorSchemeSettings();

    // apply changes
    if (themeChanged || colorSchemeChanged) {
        QtopiaChannel::send("QPE/System", "applyStyle()");
        if (themeChanged)
            QtopiaChannel::send("QPE/System", "applyStyleSplash()");
        else
            QtopiaChannel::send("QPE/System", "applyStyleNoSplash()");
    }
    if (backgroundChanged)
        applyBackgroundImage();
    if (softKeyLabelTypeChanged)
        applySoftKeyLabels();


    if (!theme->stringValue(Theme::ServerWidgets).isEmpty()) {
        QSettings serverWidgetsConfig("Trolltech", "ServerWidgets");
        serverWidgetsConfig.beginGroup("Mapping");
        serverWidgetsConfig.remove(""); //delete all entries in current grp
        serverWidgetsConfig.setValue("Default", theme->stringValue(Theme::ServerWidgets));

        if (needsRestart) {
            QtopiaIpcEnvelope env("QPE/System", "restart()");
            QtopiaApplication::quit();
        }
    }
}

void AppearanceSettings::applyBackgroundImage()
{
    QString s = m_backgroundCombo->currentText();
    gConfig.beginGroup("Appearance");
    gConfig.setValue("BackgroundImage", s);
    gConfig.endGroup();
    gConfig.sync();
    QtopiaChannel::send("QPE/System", "updateBackground()");
    QtopiaChannel::send("QPE/System", "exportBackground()");
}

void AppearanceSettings::applySoftKeyLabels()
{
    gConfig.beginGroup("ContextMenu");
    gConfig.setValue("LabelType", m_softKeyIconCheck->isChecked() ?
            int(QSoftMenuBar::IconLabel) : int(QSoftMenuBar::TextLabel));
    gConfig.endGroup();
    gConfig.sync();

    QtopiaChannel::send("QPE/System", "updateContextLabels()");
}

void AppearanceSettings::loadThemes()
{
    QStringList themeNames;
    QStringList installPaths = Qtopia::installPaths();
    for (int i=0; i<installPaths.size(); i++) {
        QString path(installPaths[i] + "etc/themes/");
        QDir dir;
        if (!dir.exists(path)) {
            qLog(UI) << "Theme style configuration path not found" << path.toLocal8Bit().data();
            continue;
        }

        // read theme.conf files
        dir.setPath(path);
        dir.setNameFilters(QStringList("*.conf")); // No tr

        for (uint j=0; j<dir.count(); j++) {
            QString name = dir[j].mid(0, dir[j].length() - 5); // cut ".conf"
            Theme *theme = Theme::create(path + dir[j], name);
            if (theme) {
                m_themes << theme;
                themeNames << theme->name();
            }
        }
    }

    QStringListModel *model;
    model = qobject_cast<QStringListModel *>(m_themeCombo->model());
    model->setStringList(themeNames);
}

void AppearanceSettings::loadColorSchemes()
{
    QHash<QString, QString> colorSchemeNames;
    QStringList installPaths = Qtopia::installPaths();
    for (int i=0; i<installPaths.size(); i++) {
        QString path(installPaths[i] + "etc/colors/");
        QDir dir;
        if (!dir.exists(path)) {
            qLog(UI) << "Color scheme configuration path not found" << path.toLocal8Bit().data();
            continue;
        }

        // read theme.conf files
        dir.setPath(path);
        dir.setNameFilters(QStringList("*.scheme")); // No tr

        // insert (scheme-file-name, scheme-path) pair
        for (uint j=0; j<dir.count(); j++) {
            if (!colorSchemeNames.contains(dir[j]))
                colorSchemeNames[dir[j]] = path + dir[j];
        }
    }
    Theme::setAvailableColorSchemes(colorSchemeNames);
}

void AppearanceSettings::loadSavedTheme()
{
    gConfig.beginGroup("Appearance");
    m_savedTheme = gConfig.value("Theme", "qtopia.conf").toString();
    m_savedColorScheme = gConfig.value("Scheme", "Qtopia").toString();
    m_savedBackground = gConfig.value("BackgroundImage", "").toString();
    gConfig.endGroup();

    gConfig.beginGroup("ContextMenu");
    m_savedLabelType = (QSoftMenuBar::LabelType)gConfig.value(
            "LabelType", QSoftMenuBar::TextLabel).toInt();
    gConfig.endGroup();
    m_softKeyIconCheck->setChecked(m_savedLabelType == QSoftMenuBar::IconLabel);

    for (int i=0; i<m_themes.size(); i++) {
        if (m_themes[i]->uniqueName() + ".conf" == m_savedTheme) {
            Theme *theme = m_themes[i];
            theme->setCurrentColorIndex(theme->colorSchemeFiles().indexOf(m_savedColorScheme));
            theme->setCurrentBackgroundIndex(theme->backgrounds().indexOf(m_savedBackground));
            m_themeCombo->setCurrentIndex(i);
            break;
        }
    }
}

void AppearanceSettings::initUi()
{
    m_themeCombo = new QComboBox;
    m_themeCombo->setModel(new QStringListModel(this));
    connect(m_themeCombo, SIGNAL(currentIndexChanged(int)), SLOT(themeChanged(int)));

    m_colorCombo = new QComboBox;
    m_colorCombo->setModel(new QStringListModel(this));
    connect(m_colorCombo, SIGNAL(currentIndexChanged(int)), SLOT(colorChanged(int)));

    m_backgroundCombo = new QComboBox;
    m_backgroundCombo->setModel(new QStringListModel(this));
    connect(m_backgroundCombo, SIGNAL(currentIndexChanged(int)), SLOT(backgroundChanged(int)));

    m_softKeyIconCheck = new QCheckBox(tr("Use icons for soft keys"));
    gConfig.beginGroup("ContextMenu");
    int labelType = (QSoftMenuBar::LabelType)gConfig.value("LabelType", QSoftMenuBar::TextLabel).toInt();
    gConfig.endGroup();
    m_softKeyIconCheck->setChecked(labelType == QSoftMenuBar::IconLabel);
    connect(m_softKeyIconCheck, SIGNAL(clicked(bool)), SLOT(softKeyOptionChanged()));

    QFormLayout *form = new QFormLayout;
    form->addRow(tr("Theme"), m_themeCombo);
    form->addRow(tr("Color"), m_colorCombo);
    form->addRow(tr("Background"), m_backgroundCombo);
    form->addRow(m_softKeyIconCheck);
    form->setAlignment(m_softKeyIconCheck, Qt::AlignCenter);

    m_previewTitle = new QLabel;
    m_previewSoftMenuBar = new QLabel;
    m_previewBackground = new QLabel;

    QGridLayout *previewGrid = new QGridLayout;
    previewGrid->setMargin(10);
    previewGrid->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    previewGrid->addWidget(m_previewTitle, 0, 0);
    previewGrid->addWidget(m_previewSoftMenuBar, 1, 0);
    previewGrid->addWidget(m_previewBackground, 0, 1, 2, 1);

    m_previewBox = new QGroupBox;
    m_previewBox->setLayout(previewGrid);
    form->addRow(m_previewBox);
    m_previewBox->hide();   // hide until a preview is shown

    QScrollArea *scroll = new QScrollArea;
    scroll->setFocusPolicy(Qt::NoFocus);
    scroll->setFrameStyle(QFrame::NoFrame);
    QWidget *w = new QWidget;
    w->setLayout(form);
    scroll->setWidget(w);
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(scroll);
    setLayout(mainLayout);
}

QString AppearanceSettings::findFile(const QString &file)
{
    QStringList instPaths = Qtopia::installPaths();
    foreach (QString path, instPaths) {
        QString themeDataPath(path + file);
        if (QFile::exists(themeDataPath)) {
            return themeDataPath;
        }
    }

    return QString();
}


//------- methods for external settings changes via IPC follow: -------


void AppearanceSettings::pushSettingStatus()
{
    QtopiaServiceRequest e("SettingsManager", "pushSettingStatus(QString,QString,QString)");
    e << QString("appearance") << QString(windowTitle()) << settingsString();
    e.send();
}

void AppearanceSettings::pullSettingStatus()
{
    QtopiaServiceRequest e("SettingsManager", "pullSettingStatus(QString,QString,QString)");
    e << QString("appearance") << QString(windowTitle()) << settingsString();
    e.send();
}

void AppearanceSettings::changeSettings(const QString &settingsString)
{
    QStringList details = settingsString.split(',');
    m_themeCombo->setCurrentIndex(details.at(0).toInt());
    m_colorCombo->setCurrentIndex(details.at(1).toInt());
    m_backgroundCombo->setCurrentIndex(details.at(2).toInt());
    m_softKeyIconCheck->setCheckState(details.at(3).toInt() ? Qt::Checked : Qt::Unchecked);
}

QString AppearanceSettings::settingsString() const
{
    QString status;
    status += QString::number(m_themeCombo->currentIndex()) + ",";
    status += QString::number(m_colorCombo->currentIndex()) + ",";
    status += QString::number(m_backgroundCombo->currentIndex()) + ",";
    status += QString::number(m_softKeyIconCheck->isChecked()) + ",";
    return status;
}

void AppearanceSettings::receiveAppMessage(const QString &msg, const QByteArray &data)
{
    QDataStream ds(data);
    if (msg == "Settings::setStatus(bool,QString)") {
        // must show widget to keep running
        QtopiaApplication::instance()->showMainWidget();
        m_isStatusView = true;
        QSoftMenuBar::removeMenuFrom(this, QSoftMenuBar::menuFor(this));
        QSoftMenuBar::menuFor(this);
        QString details;
        bool isFromActiveProfile;
        ds >> isFromActiveProfile;
        ds >> details;
        changeSettings(details);
        applyCurrentSettings();
    } else if (msg == "Settings::activateSettings(QString)") {
        hide();
        QString details;
        ds >> details;
        changeSettings(details);
        applyCurrentSettings();
    } else if (msg == "Settings::pullSettingStatus()") {
        hide();
        pullSettingStatus();
    } else if (msg == "Settings::activateDefault()") {
        hide();
        for (int i=0; i<m_themes.size(); i++) {
            if (m_themes[i]->uniqueName() == "qtopia") {
                m_themeCombo->setCurrentIndex(i);
                break;
            }
        }
        Theme *theme = currentTheme();
        if (theme) {
            int index = theme->colorSchemeNames().indexOf("Qtopia");
            if (index != -1)
                colorChanged(index);
        }
        m_softKeyIconCheck->setChecked(true);
        applyCurrentSettings();
    }
}

void AppearanceSettings::resizeEvent(QResizeEvent *re)
{
    QDialog::resizeEvent(re);
    m_previewBox->hide();
    QTimer::singleShot(0, this, SLOT(previewColorChanges()));
    QTimer::singleShot(0, this, SLOT(previewBackgroundChanges()));
}

void AppearanceSettings::openHomescreenSettings()
{
    QtopiaIpcEnvelope env("QPE/Application/homescreen", "HomescreenSettings::configure()");
}

