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

#ifndef APPEARANCE_H
#define APPEARANCE_H

#include <QDialog>
#include <QStringList>

#include <QSoftMenuBar>

class Theme;
class QComboBox;
class QCheckBox;
class QLabel;
class QGroupBox;
template <class T> class QList;

class AppearanceSettings : public QDialog
{
    Q_OBJECT
public:
    AppearanceSettings(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~AppearanceSettings();

    static QString findFile(const QString &file);

public slots:
    void accept();

private slots:
    void themeChanged(int index);
    void colorChanged(int index);
    void backgroundChanged(int index);
    void softKeyOptionChanged();

    void previewColorChanges();
    void previewSoftMenuBarChanges();
    void previewBackgroundChanges();

    void pushSettingStatus();
    void openHomescreenSettings();
    void receiveAppMessage(const QString &msg, const QByteArray &data);

protected:
    void resizeEvent(QResizeEvent *re);

private:
    Theme *currentTheme() const;
    bool themeNeedsRestart(Theme *theme) const;
    void applyCurrentSettings();
    void applyBackgroundImage();
    void applySoftKeyLabels();

    void loadThemes();
    void loadColorSchemes();
    void initUi();
    void loadSavedTheme();

    void pullSettingStatus();
    void changeSettings(const QString &settingsString);
    QString settingsString() const;

    QList<Theme *> m_themes;
    Theme *m_activeTheme;
    QComboBox *m_themeCombo;
    QComboBox *m_colorCombo;
    QComboBox *m_backgroundCombo;
    QCheckBox *m_softKeyIconCheck;
    QLabel *m_previewTitle;
    QLabel *m_previewSoftMenuBar;
    QLabel *m_previewBackground;
    QGroupBox *m_previewBox;

    QString m_savedTheme;
    QString m_savedColorScheme;
    QString m_savedBackground;
    QSoftMenuBar::LabelType m_savedLabelType;

    bool m_isStatusView;
};

#endif
