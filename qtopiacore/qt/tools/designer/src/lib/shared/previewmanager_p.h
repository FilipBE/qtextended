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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef PREVIEWMANAGER_H
#define PREVIEWMANAGER_H

#include "shared_global_p.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QWidget;
class QPixmap;
class QAction;
class QActionGroup;
class QMenu;
class QWidget;
class QSettings;

namespace qdesigner_internal {

// ----------- PreviewConfiguration

class PreviewConfigurationData;

class QDESIGNER_SHARED_EXPORT PreviewConfiguration {
public:
    PreviewConfiguration();
    explicit PreviewConfiguration(const QString &style, const QString &applicationStyleSheet = QString(), const QString &deviceSkin = QString());

    PreviewConfiguration(const PreviewConfiguration&);
    PreviewConfiguration& operator=(const PreviewConfiguration&);
    ~PreviewConfiguration();

    QString style() const;
    void setStyle(const QString &);

    // Style sheet to prepend (to simulate the effect od QApplication::setSyleSheet()).
    QString applicationStyleSheet() const;
    void setApplicationStyleSheet(const QString &);

    QString deviceSkin() const;
    void setDeviceSkin(const QString &);

    void clear();
    void toSettings(const QString &prefix, QSettings &settings) const;
    void fromSettings(const QString &prefix, const QSettings &settings);

private:
    QSharedDataPointer<PreviewConfigurationData> m_d;
};

QDESIGNER_SHARED_EXPORT bool operator<(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2);
QDESIGNER_SHARED_EXPORT bool operator==(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2);
QDESIGNER_SHARED_EXPORT bool operator!=(const PreviewConfiguration &pc1, const PreviewConfiguration &pc2);

// ----------- Preview window manager.
// Maintains a list of preview widgets with their associated form windows and configuration.

class PreviewManagerPrivate;

class QDESIGNER_SHARED_EXPORT PreviewManager : public QObject
{
    Q_OBJECT
public:
    enum PreviewMode {
        // Modal preview. Do not use on Macs as dialogs would have no close button
        ApplicationModalPreview,
        // Non modal previewing of one form in different configurations (closes if form window changes)
        SingleFormNonModalPreview,
        // Non modal previewing of several forms in different configurations
        MultipleFormNonModalPreview };

    explicit PreviewManager(PreviewMode mode, QObject *parent);
    virtual ~PreviewManager();

    // Show preview. Raise existing preview window if there is one with a matching
    // configuration, else create a new preview.
    QWidget *showPreview(const QDesignerFormWindowInterface *, const PreviewConfiguration &pc, QString *errorMessage);

    int previewCount() const;

    // Create a pixmap for printing.
    QPixmap createPreviewPixmap(const QDesignerFormWindowInterface *fw, const PreviewConfiguration &pc, QString *errorMessage);

    // convenience function to create a style action group and add it to a menu. Style names will be set as Data on the actions.
    static QActionGroup *createStyleActionGroup(QObject *parent, QMenu *subMenu = 0);

    virtual bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void closeAllPreviews();

signals:
    void firstPreviewOpened();
    void lastPreviewClosed();

private:

    virtual Qt::WindowFlags previewWindowFlags(const QWidget *widget) const;
    virtual QWidget *createDeviceSkinContainer(const QDesignerFormWindowInterface *) const;

    QWidget *raise(const QDesignerFormWindowInterface *, const PreviewConfiguration &pc);
    QWidget *createPreview(const QDesignerFormWindowInterface *, const PreviewConfiguration &pc, QString *errorMessage);

    void updatePreviewClosed(QWidget *w);

    PreviewManagerPrivate *d;

    PreviewManager(const PreviewManager &other);
    PreviewManager &operator =(const PreviewManager &other);
};
}

QT_END_NAMESPACE

#endif // PREVIEWMANAGER_H
