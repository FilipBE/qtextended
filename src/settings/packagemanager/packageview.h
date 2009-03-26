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

#ifndef PACKAGEVIEW_H
#define PACKAGEVIEW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTabWidget>
#include <QtopiaItemDelegate>

class PackageModel;
class QTreeView;
class QMenu;
class QActionGroup;
class QTextEdit;
class QAction;
class QShowEvent;
class QWaitWidget;
class QTabWidget;

#include "ui_packagedetails.h"
class PackageDetails : public QDialog, public Ui::PackageDetails
{
    Q_OBJECT
public:
    enum Type { Info, Confirm };
    enum Result { Proceed = QDialog::Accepted + 1};
    enum Option {
                    //used for Info type
                    None        = 0x0,
                    Install     = 0x1,
                    Uninstall   = 0x2,

                    //used for Confirm type
                    Allow       = 0x4,
                    Disallow    = 0x8
                };
    Q_DECLARE_FLAGS( Options, Option );

    PackageDetails(QWidget *parent,
                   const QString &title,
                   const QString &text,
                   Type type,
                   Options options);
    void init();

private:
    Type m_type;
    Options m_options;

    QAction *m_acceptAction;
    QAction *m_rejectAction;
    QMenu *m_contextMenu;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(PackageDetails::Options);

class ViewDelegate: public QtopiaItemDelegate
{
    public:
        ViewDelegate( QObject *parent = 0 );
        virtual void paint( QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const;
};

class DownloadViewDelegate:public ViewDelegate
{
    public:
        DownloadViewDelegate(QObject *parent=0);
        virtual void paint(QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;
};

class PackageView : public QMainWindow
{
    Q_OBJECT
    friend class PackageManagerService;
    friend class PackageServiceInstaller;
public:

    PackageView(QWidget* parent = 0, Qt::WFlags flags = 0);
    ~PackageView();

signals:
    void targetChoiceChanged( const QString & );
    void serverListUpdate( const QStringList &, const QStringList & );

private slots:
    void init();
    void editServers();
    void serversChanged( const QStringList & );
    void serverChoice( QAction* a );
    void targetsChanged( const QStringList & );
    void targetChoice( QAction* );
    void showDetails( const QModelIndex &, PackageDetails::Type type );
    void displayDetails();
    void startInstall();
    void startUninstall();
    void confirmReenable();
    void activateItem( const QModelIndex & );
    void contextMenuShow();
    void postServerStatus( const QString & );
    void selectNewlyInstalled( const QModelIndex & );
private:
    QTreeView *installedView;
    QTreeView *downloadView;
    QTabWidget *tabWidget;
    PackageModel *model;
    QString prevTarget;
    QLabel *statusLabel;
    QWaitWidget *waitWidget;

    QMenu *menuServers;
    QMenu *menuTarget;
    QActionGroup *targetActionGroup;
    QActionGroup *serversActionGroup;
    QAction *reenableAction;
    QAction *detailsAction;
    QAction *installAction;
    QAction *uninstallAction;

    static const int InstalledIndex;
    static const int DownloadIndex;
};

class KeyFilter:public QObject
{
    Q_OBJECT

    public:
        KeyFilter(QTabWidget * tab, QObject *parent=0);

    protected:
        bool eventFilter(QObject *obj, QEvent *event);
        QTabWidget *m_tab;
};

#endif
