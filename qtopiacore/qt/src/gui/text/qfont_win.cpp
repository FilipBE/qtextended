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

// the miscrosoft platform SDK says that the Unicode versions of
// TextOut and GetTextExtentsPoint32 are supported on all platforms, so we use them
// exclusively to simplify code, save a lot of conversions into the local encoding
// and have generally better unicode support :)

#include "qfont.h"
#include "qfont_p.h"
#include "qfontengine_p.h"
#include "qtextengine_p.h"
#include "qfontmetrics.h"
#include "qfontinfo.h"

#include "qwidget.h"
#include "qpainter.h"
#include <limits.h>
#include "qt_windows.h"
#include <private/qapplication_p.h>
#include "qapplication.h"
#include <private/qunicodetables_p.h>
#include <qfontdatabase.h>

QT_BEGIN_NAMESPACE
        
extern HDC   shared_dc();                // common dc for all fonts

// ### maybe move to qapplication_win
QFont qt_LOGFONTtoQFont(LOGFONT& lf, bool /*scale*/)
{
    QString family = QT_WA_INLINE(QString::fromUtf16((ushort*)lf.lfFaceName),
                                   QString::fromLocal8Bit((char*)lf.lfFaceName));
    QFont qf(family);
    qf.setItalic(lf.lfItalic);
    if (lf.lfWeight != FW_DONTCARE) {
        int weight;
        if (lf.lfWeight < 400)
            weight = QFont::Light;
        else if (lf.lfWeight < 600)
            weight = QFont::Normal;
        else if (lf.lfWeight < 700)
            weight = QFont::DemiBold;
        else if (lf.lfWeight < 800)
            weight = QFont::Bold;
        else
            weight = QFont::Black;
        qf.setWeight(weight);
    }
    int lfh = qAbs(lf.lfHeight);
    qf.setPointSizeF(lfh * 72.0 / GetDeviceCaps(shared_dc(),LOGPIXELSY));
    qf.setUnderline(false);
    qf.setOverline(false);
    qf.setStrikeOut(false);
    return qf;
}


static inline float pixelSize(const QFontDef &request, int dpi)
{
    float pSize;
    if (request.pointSize != -1)
        pSize = request.pointSize * dpi/ 72.;
    else
        pSize = request.pixelSize;
    return pSize;
}

static inline float pointSize(const QFontDef &fd, int dpi)
{
    float pSize;
    if (fd.pointSize < 0)
        pSize = fd.pixelSize * 72. / ((float)dpi);
    else
        pSize = fd.pointSize;
    return pSize;
}

/*****************************************************************************
  QFont member functions
 *****************************************************************************/

void QFont::initialize()
{
}

void QFont::cleanup()
{
    QFontCache::cleanup();
}

HFONT QFont::handle() const
{
    QFontEngine *engine = d->engineForScript(QUnicodeTables::Common);
    Q_ASSERT(engine != 0);
    if (engine->type() == QFontEngine::Multi)
        engine = static_cast<QFontEngineMulti *>(engine)->engine(0);
    if (engine->type() == QFontEngine::Win)
	return static_cast<QFontEngineWin *>(engine)->hfont;
    return 0;
}

QString QFont::rawName() const
{
    return family();
}

void QFont::setRawName(const QString &name)
{
    setFamily(name);
}

QString QFont::defaultFamily() const
{
    switch(d->request.styleHint) {
        case QFont::Times:
            return QString::fromLatin1("Times New Roman");
        case QFont::Courier:
            return QString::fromLatin1("Courier New");
        case QFont::Decorative:
            return QString::fromLatin1("Bookman Old Style");
        case QFont::Helvetica:
            return QString::fromLatin1("Arial");
        case QFont::System:
        default:
            return QString::fromLatin1("MS Sans Serif");
    }
}

QString QFont::lastResortFamily() const
{
    return QString::fromLatin1("helvetica");
}

QString QFont::lastResortFont() const
{
    return QString::fromLatin1("arial");
}

QT_END_NAMESPACE
