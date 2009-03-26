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
#ifndef CGEN_H
#define CGEN_H

#include <QWidget>

class QProgressBar;
class QLabel;
class WorkerThread;
class ContactGeneratorWidget : public QWidget
{
    Q_OBJECT
public:
    ContactGeneratorWidget(QWidget *parent = 0, Qt::WFlags f = 0);
    ~ContactGeneratorWidget();

private slots:
    void processRequest(const QString &, const QByteArray &);

private:
    void generateData(int, int, int);

    void exportRecords(const QString &pimType, const QString &file);
    void importRecords(const QString &pimType, const QString &file);
    void compareRecords(const QString &pimType, const QString &file, bool compareId = false);

    void clearRecords(const QString &pimType);

    void sendProgress(int);
    void sendFail();
    void sendCompleted();
};

#endif
