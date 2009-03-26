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

#ifndef QCATEGORYSELECTOR_H
#define QCATEGORYSELECTOR_H

#include <qtopiaglobal.h>
#include <qcategorymanager.h>

#include <QWidget>
#include <QDialog>
#include <QItemSelection>

class QCategorySelectData;
class QTOPIA_EXPORT QCategorySelector : public QWidget
{
    Q_OBJECT

public:
    enum ContentFlag
    {
        IncludeAll = 0x04,     // Adds "All" option
        IncludeUnfiled = 0x08, // Adds "Unfiled" option
        ListView = 0x10,      // Forces it to appear as a list
        ComboView = 0x20,     // Forces it to appear as a combobox
        DialogView = 0x40,    // Forces it to appear as a button that displays a dialog.
        SingleSelection = 0x80,    // Forces it to allow only selecting a single category

        Filter = IncludeAll | IncludeUnfiled,
        Editor = IncludeUnfiled,
        ViewMask = ListView | ComboView | DialogView
    };
    Q_DECLARE_FLAGS(ContentFlags, ContentFlag)

    explicit QCategorySelector(QWidget *parent = 0);
    explicit QCategorySelector(const QString &scope, ContentFlags f = Editor, QWidget *parent = 0);
    virtual ~QCategorySelector();

    QStringList selectedCategories() const;
    QCategoryFilter selectedFilter() const;

    virtual QSize sizeHint () const;

    void setListFrameStyle(int style);

public slots:
    void selectCategories(const QString &id);
    void selectCategories(const QStringList &id);
    void selectFilter(const QCategoryFilter &);

    void selectAll();
    void selectUnfiled();

signals:
    void categoriesSelected(const QList<QString> &);
    void filterSelected(const QCategoryFilter &);
    void listActivated(int pos);

private slots:
    void comboSelection(int index);
    void listActivated(const QModelIndex &);

    void showDialog();
    void dialogAccepted();

private:
    QCategorySelectData *d;
};

class QCategoryDialogData;
class QTOPIA_EXPORT QCategoryDialog : public QDialog
{
    Q_OBJECT

public:
    enum ContentFlag
    {
        IncludeAll = 0x04,
        IncludeUnfiled = 0x08,
        SingleSelection = 0x80,

        Filter = IncludeAll | IncludeUnfiled,
        Editor = IncludeUnfiled

    };
    Q_DECLARE_FLAGS(ContentFlags, ContentFlag)

    explicit QCategoryDialog(const QString &scope, ContentFlags f = Filter, QWidget *parent = 0);
    ~QCategoryDialog();

    void setText(const QString &);
    QString text() const;

    QList<QString> selectedCategories() const;
    QCategoryFilter selectedFilter() const;

public slots:
    void selectCategories(const QString &id);
    void selectCategories(const QList<QString> &id);
    void selectFilter(const QCategoryFilter &);

    void selectAll();
    void selectUnfiled();

protected:
    virtual QSize sizeHint() const;
    void keyPressEvent(QKeyEvent* e);

private:
    QCategoryDialogData *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCategorySelector::ContentFlags)

Q_DECLARE_OPERATORS_FOR_FLAGS(QCategoryDialog::ContentFlags)

#endif
