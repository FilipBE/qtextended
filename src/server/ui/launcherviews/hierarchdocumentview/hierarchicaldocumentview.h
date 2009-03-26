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

#ifndef HIERARCHICALDOCUMENTVIEW_H
#define HIERARCHICALDOCUMENTVIEW_H

#include "documentview.h"

class QLabel;
class QCategoryManager;
class QVBoxLayout;
class QSoftMenuBar;

// Stores a name, content filter & icon. To be used in a model.
class ContentFilterProperty
{
public:
    ContentFilterProperty (const QString &name, const QContentFilter &value, const QIcon &icon)
        : mName(name), mValue(value), mIcon(icon) {}
    QString name() const {return mName;}
    QContentFilter value() const {return mValue;}
    QIcon icon() const {return mIcon;}
private:
    QString mName;
    QContentFilter mValue;
    QIcon mIcon;
};

// A model to store & display ContentFilterProperty-objects (incl. icon) in a list view.
class AbstractContentFilterPropertyListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    AbstractContentFilterPropertyListModel(QObject *parent=0): QAbstractListModel( parent ) {};
    virtual QVariant data ( const QModelIndex &index, int role) const;
    virtual int rowCount ( const QModelIndex &parent) const;
    virtual QContentFilter value(int index) const;
    QModelIndex createIndex(const QContentFilter &filter) const;
    virtual QString findPropertyName(const QContentFilter &filter) const;

protected:
    virtual int findIndex(const QContentFilter &filter) const;

    QList<ContentFilterProperty> mList;

public slots:
    virtual void reload() = 0;
};

// A model to store content filters on mime types.
class MimeTypeFilterModel : public AbstractContentFilterPropertyListModel
{
    Q_OBJECT
public:
    MimeTypeFilterModel(QObject *parent=0): AbstractContentFilterPropertyListModel( parent ){};

public slots:
    virtual void reload();

protected:
    void addProperty(const QString &label, const QMimeType &mimeType,const QIcon &icon);
};

// A model to store content filters on categories.
class CategoryFilterListModel : public  AbstractContentFilterPropertyListModel
{
    Q_OBJECT
public:
    CategoryFilterListModel(QObject *parent=0);

public slots:
    virtual void reload();

protected:
    QCategoryManager* categoryManager;
};

// Extends the DocumentLauncherView by making selection of mime type and category compulsory.
// After selection of these the expected document list is shown filtered by mime type and category.
class HierarchicalDocumentLauncherView : public DocumentLauncherView
{
    Q_OBJECT
public:
    HierarchicalDocumentLauncherView(QWidget* parent = 0, Qt::WFlags fl = 0);
    virtual ~HierarchicalDocumentLauncherView();

    virtual void resetSelection();

protected:
    enum FilterNavigation { NavigateForward, NavigateBackward };

    QList<QContentFilter> selectedFilters;

    MimeTypeFilterModel *modelTypes;
    CategoryFilterListModel *modelCategories;
    QCategoryManager *categoryManager;

    void filterNavigate(FilterNavigation navigation, QModelIndex selectedItem = QModelIndex());
    void setCurrentItem(const QContentFilter& pFilter);

    bool eventFilter(QObject *obj, QEvent *event);

    QLabel* selectedFilterLabel;

    virtual void enterNavigationMode();
    virtual void exitNavigationMode();

    virtual void handleReturnPressed(const QModelIndex &item);
    virtual void handleItemClicked(const QModelIndex &item, bool setCurrentIndex);

    QMenu* dummyMenu;

private:
    void init();
};

#endif
