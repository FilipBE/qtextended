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
#ifndef QCONTENTFILTERSELECTOR_H
#define QCONTENTFILTERSELECTOR_H

#include <qcontentfiltermodel.h>

class QContentFilterSelectorPrivate;

class QTOPIA_EXPORT QContentFilterSelector : public QWidget
{
    Q_OBJECT
public:
    explicit QContentFilterSelector( QWidget *parent = 0 );
    explicit QContentFilterSelector( const QContentFilterModel::Template &modelTemplate,
                                     QWidget *parent = 0 );
    explicit QContentFilterSelector( QContent::Property property,
                                     QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                     const QStringList &checked = QStringList(),
                                     QWidget *parent = 0 );
    explicit QContentFilterSelector( QContentFilter::FilterType type,
                                     const QString &scope = QString(),
                                     QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                     const QStringList &checked = QStringList(),
                                     QWidget *parent = 0 );
    virtual ~QContentFilterSelector();

    QContentFilter filter();
    void setFilter( const QContentFilter &filter );

    QContentFilterModel::Template modelTemplate() const;
    void setModelTemplate( const QContentFilterModel::Template &modelTemplate );
    void setModelTemplate( QContent::Property property,
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );
    void setModelTemplate( QContentFilter::FilterType type,
                           const QString &scope = QString(),
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );

    QContentFilter checkedFilter() const;

    QString checkedLabel() const;

signals:
    void filterSelected( const QContentFilter &filter );

private:
    void init();

    QContentFilterSelectorPrivate *d;
};

class QContentFilterDialogPrivate;

class QTOPIA_EXPORT QContentFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QContentFilterDialog( QWidget *parent = 0 );
    explicit QContentFilterDialog( const QContentFilterModel::Template &modelTemplate,
                                   QWidget *parent = 0 );
    explicit QContentFilterDialog( QContent::Property property,
                                   QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                   const QStringList &checked = QStringList(),
                                   QWidget *parent = 0 );
    explicit QContentFilterDialog( QContentFilter::FilterType type,
                                   const QString &scope = QString(),
                                   QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                                   const QStringList &checked = QStringList(),
                                   QWidget *parent = 0 );
    virtual ~QContentFilterDialog();

    QContentFilter filter();
    void setFilter( const QContentFilter &filter );

    QContentFilterModel::Template modelTemplate() const;
    void setModelTemplate( const QContentFilterModel::Template &modelTemplate );
    void setModelTemplate( QContent::Property property,
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );
    void setModelTemplate( QContentFilter::FilterType type,
                           const QString &scope = QString(),
                           QContentFilterModel::TemplateOptions options = QContentFilterModel::SelectAll,
                           const QStringList &checked = QStringList() );

    QContentFilter selectedFilter() const;
    QContentFilter checkedFilter() const;

    QString checkedLabel() const;

private slots:
    void filterSelected( const QContentFilter &filter );

private:
    void init();

    QContentFilterDialogPrivate *d;
};

#endif
