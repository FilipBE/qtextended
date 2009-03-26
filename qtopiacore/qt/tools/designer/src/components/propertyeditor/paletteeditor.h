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

#ifndef PALETTEEDITOR_H
#define PALETTEEDITOR_H

#include "ui_paletteeditor.h"
#include <QtGui/QItemDelegate>

QT_BEGIN_NAMESPACE

class QListView;
class QLabel;
class QtColorButton;
class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class PaletteEditor: public QDialog
{
    Q_OBJECT
public:
    virtual ~PaletteEditor();

    static QPalette getPalette(QDesignerFormEditorInterface *core,
                QWidget* parent, const QPalette &init = QPalette(),
                const QPalette &parentPal = QPalette(), int *result = 0);

    QPalette palette() const;
    void setPalette(const QPalette &palette);
    void setPalette(const QPalette &palette, const QPalette &parentPalette);

private slots:

    void on_buildButton_colorChanged(const QColor &);
    void on_activeRadio_clicked();
    void on_inactiveRadio_clicked();
    void on_disabledRadio_clicked();
    void on_computeRadio_clicked();
    void on_detailsRadio_clicked();

    void paletteChanged(const QPalette &palette);

protected:

private:
    PaletteEditor(QDesignerFormEditorInterface *core, QWidget *parent);
    void buildPalette();

    void updatePreviewPalette();
    void updateStyledButton();

    QPalette::ColorGroup currentColorGroup() const
        { return m_currentColorGroup; }

    Ui::PaletteEditor ui;
    QPalette m_editPalette;
    QPalette m_parentPalette;
    QPalette::ColorGroup m_currentColorGroup;
    class PaletteModel *m_paletteModel;
    bool m_modelUpdated;
    bool m_paletteUpdated;
    bool m_compute;
    QDesignerFormEditorInterface *m_core;
};


class PaletteModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QPalette::ColorRole colorRole READ colorRole)
public:
    explicit PaletteModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                int role = Qt::DisplayRole) const;

    QPalette getPalette() const;
    void setPalette(const QPalette &palette, const QPalette &parentPalette);

    QPalette::ColorRole colorRole() const { return QPalette::NoRole; }
    void setCompute(bool on) { m_compute = on; }
signals:
    void paletteChanged(const QPalette &palette);
private:

    QPalette::ColorGroup columnToGroup(int index) const;
    int groupToColumn(QPalette::ColorGroup group) const;

    QPalette m_palette;
    QPalette m_parentPalette;
    QMap<QPalette::ColorRole, QString> m_roleNames;
    bool m_compute;
};

class BrushEditor : public QWidget
{
    Q_OBJECT
public:
    explicit BrushEditor(QDesignerFormEditorInterface *core, QWidget *parent = 0);

    void setBrush(const QBrush &brush);
    QBrush brush() const;
    bool changed() const;
signals:
    void changed(QWidget *widget);
private slots:
    void brushChanged();
private:
    QtColorButton *m_button;
    bool m_changed;
    QDesignerFormEditorInterface *m_core;
};

class RoleEditor : public QWidget
{
    Q_OBJECT
public:
    explicit RoleEditor(QWidget *parent = 0);

    void setLabel(const QString &label);
    void setEdited(bool on);
    bool edited() const;
signals:
    void changed(QWidget *widget);
private slots:
    void emitResetProperty();
private:
    QLabel *m_label;
    bool    m_edited;
};

class ColorDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit ColorDelegate(QDesignerFormEditorInterface *core, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    void setEditorData(QWidget *ed, const QModelIndex &index) const;
    void setModelData(QWidget *ed, QAbstractItemModel *model,
                const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *ed,
                const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &opt,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const;
private:
    QDesignerFormEditorInterface *m_core;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // PALETTEEDITOR_H
