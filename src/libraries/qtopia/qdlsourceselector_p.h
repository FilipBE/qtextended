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

#ifndef QDLSOURCESELECTOR_P_H
#define QDLSOURCESELECTOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QDialog>
#include <QList>
#include <QString>

// Qtopia includes
#include <QDSServiceInfo>

// Forward class declarations
class QDLSourceSelectorPrivate;

// ============================================================================
//
// QDLSourceSelector
//
// ============================================================================

class QDLSourceSelector : public QDialog
{
    Q_OBJECT
public:
    explicit QDLSourceSelector( const QMimeType& responseDataType,
                                QWidget *parent = 0,
                                Qt::WindowFlags fl = 0 );

    ~QDLSourceSelector();

    QList<QDSServiceInfo> selected() const;
    QSize sizeHint() const;

protected slots:
    void accept();

signals:
    void selected( const QList<QDSServiceInfo>& );

private:
    QDLSourceSelectorPrivate* d;
};

#endif
