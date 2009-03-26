/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QPRINTERINFO_UNIX_P_H
#define QPRINTERINFO_UNIX_P_H

#ifndef QT_NO_NIS
#  ifndef BOOL_DEFINED
#    define BOOL_DEFINED
#  endif

#  include <sys/types.h>
#  include <rpc/rpc.h>
#  include <rpcsvc/ypclnt.h>
#  include <rpcsvc/yp_prot.h>
#endif // QT_NO_NIS

#ifdef Success
#  undef Success
#endif

#include <ctype.h>

QT_BEGIN_NAMESPACE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_NO_PRINTER

struct QPrinterDescription {
    QPrinterDescription(const QString &n, const QString &h, const QString &c, const QStringList &a)
    : name(n), host(h), comment(c), aliases(a) {}
    QString name;
    QString host;
    QString comment;
    QStringList aliases;
    bool samePrinter(const QString& printer) const {
        return name == printer || aliases.contains(printer);
    }
};

enum { Success = 's', Unavail = 'u', NotFound = 'n', TryAgain = 't' };
enum { Continue = 'c', Return = 'r' };

void qt_perhapsAddPrinter(QList<QPrinterDescription> *printers, const QString &name,
                          QString host, QString comment,
                          QStringList aliases = QStringList());
void qt_parsePrinterDesc(QString printerDesc, QList<QPrinterDescription> *printers);

int qt_parsePrintcap(QList<QPrinterDescription> *printers, const QString& fileName);
QString qt_getDefaultFromHomePrinters();
void qt_parseEtcLpPrinters(QList<QPrinterDescription> *printers);
char *qt_parsePrintersConf(QList<QPrinterDescription> *printers, bool *found = 0);

#ifndef QT_NO_NIS
#if defined(Q_C_CALLBACKS)
extern "C" {
#endif
int qt_pd_foreach(int /*status */, char * /*key */, int /*keyLen */,
                  char *val, int valLen, char *data);

#if defined(Q_C_CALLBACKS)
}
#endif
int qt_retrieveNisPrinters(QList<QPrinterDescription> *printers);
#endif // QT_NO_NIS
char *qt_parseNsswitchPrintersEntry(QList<QPrinterDescription> *printers, char *line);
char *qt_parseNsswitchConf(QList<QPrinterDescription> *printers);
void qt_parseEtcLpMember(QList<QPrinterDescription> *printers);
void qt_parseSpoolInterface(QList<QPrinterDescription> *printers);
void qt_parseQconfig(QList<QPrinterDescription> *printers);
int qt_getLprPrinters(QList<QPrinterDescription>& printers);

#endif // QT_NO_PRINTER

QT_END_NAMESPACE

#endif // QPRINTERINFO_UNIX_P_H
