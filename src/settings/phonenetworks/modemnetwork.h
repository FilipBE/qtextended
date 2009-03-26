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

#ifndef MODEMNETWORK_H
#define MODEMNETWORK_H

#include <qnetworkregistration.h>
#include <qpreferrednetworkoperators.h>
#include <qbandselection.h>

#include <QDialog>
#include <QListWidget>

class QModemNetworkRegistration;
class QWaitWidget;
class TwoLineFloatingTextList;

class ModemNetworkRegister : public QListWidget
{
    Q_OBJECT
public:
    ModemNetworkRegister( QWidget *parent = 0 );
    ~ModemNetworkRegister();

private slots:
    void operationSelected( QListWidgetItem * );
    void selectOperator( const QList<QNetworkRegistration::AvailableOperator> & );
    void band( QBandSelection::BandMode, const QString & );
    void selectBand( const QStringList & );
    void setBandResult( QTelephony::Result );
    void acceptOpDlg();
    void setCurrentOperatorResult( QTelephony::Result result );

private:
    QNetworkRegistration *m_client;
    QBandSelection *m_bandSel;
    QString m_curBand;
    QBandSelection::BandMode m_curBandMode;
    QWaitWidget *m_waitWidget;
    QDialog *m_opDlg;
    TwoLineFloatingTextList *m_opList;
    QListWidgetItem *m_originalOp;
    QList<QNetworkRegistration::AvailableOperator> m_result;

    void init();
    void selectSearchMode();
    void preferredOperators();
    void showCurrentOperator();
};

class PreferredOperatorsDialog : public QDialog
{
    Q_OBJECT
public:
    PreferredOperatorsDialog( QWidget *parent = 0, Qt::WFlags fl = 0 );
    ~PreferredOperatorsDialog();

protected:
    void accept();
    void showEvent( QShowEvent * );

private slots:
    void operatorNames( const QList<QPreferredNetworkOperators::NameInfo> & );
    void preferredOperators( QPreferredNetworkOperators::List,
            const QList<QPreferredNetworkOperators::Info> & );
    void addNetwork();
    void networkSelected( QListWidgetItem * );
    void removeNetwork();
    void moveUp();
    void moveDown();
    void rowChanged( int );
    void requestOperatorInfo();

private:
    void init();
    void populateList();
    void swap( int , int );
    void updateIndex( int, bool );
    void checkIndex();
    bool isPreferred( unsigned int );
    void initAddNetworkDlg();

    TwoLineFloatingTextList *m_list;
    QPreferredNetworkOperators *m_PNOClient;
    QList<QPreferredNetworkOperators::NameInfo> m_operatorNames;
    QList<QPreferredNetworkOperators::Info> m_currentOpers;
    QList<QPreferredNetworkOperators::Info> m_originalOpers;
    QWaitWidget *m_waitWidget;
    QAction *m_add, *m_remove, *m_up, *m_down;
    QDialog *m_addNetworkDlg;
    TwoLineFloatingTextList *m_addNetworkList;

    // ugly hack to make the Select Position dialog sane.
    bool eventFilter(QObject *obj, QEvent *event);
    int savedPrefNetOpLoc;
};

#endif
