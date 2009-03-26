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

#ifndef QPRINTENGINE_WIN_P_H
#define QPRINTENGINE_WIN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_NO_PRINTER

#include "QtGui/qprinter.h"
#include "QtGui/qprintengine.h"
#include "QtGui/qpaintengine.h"
#include "QtCore/qt_windows.h"
#include "private/qpaintengine_p.h"

QT_BEGIN_NAMESPACE

class QAlphaPaintEnginePrivate;
class QWin32PrintEnginePrivate;
class QPrinterPrivate;
class QPainterState;

class QAlphaPaintEngine : public QPaintEngine
{
    Q_DECLARE_PRIVATE(QAlphaPaintEngine)
public:
    ~QAlphaPaintEngine();

    virtual bool begin(QPaintDevice *pdev);
    virtual bool end();

    virtual void updateState(const QPaintEngineState &state);

    virtual void drawPath(const QPainterPath &path);

    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);

    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    virtual void drawTextItem(const QPointF &p, const QTextItem &textItem);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);

protected:
    QAlphaPaintEngine(QAlphaPaintEnginePrivate &data, PaintEngineFeatures devcaps = 0);
    QRegion alphaClipping() const;
    bool continueCall() const;
    void flushAndInit(bool init = true);
    void cleanUp();
};

class QAlphaPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QAlphaPaintEngine)
public:
    QAlphaPaintEnginePrivate();
    ~QAlphaPaintEnginePrivate();

    int m_pass;
    QPicture *m_pic;
    QPaintEngine *m_picengine;
    QPainter *m_picpainter;

    QPaintEngine::PaintEngineFeatures m_savedcaps;
    QPaintDevice *m_pdev;

    QRegion m_alphargn;
    QRegion m_cliprgn;

    bool m_hasalpha;
    bool m_alphaPen;
    bool m_alphaBrush;
    bool m_alphaOpacity;
    bool m_advancedPen;
    bool m_advancedBrush;
    bool m_complexTransform;
    bool m_continueCall;

    QTransform m_transform;
    QPen m_pen;

    void addAlphaRect(const QRectF &rect);
    QRectF addPenWidth(const QPainterPath &path);
    void drawAlphaImage(const QRectF &rect);
    QRect toRect(const QRectF &rect) const;
    bool fullyContained(const QRectF &rect) const;

    void resetState(QPainter *p);
};

class QWin32PrintEngine : public QAlphaPaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QWin32PrintEngine)
public:
    QWin32PrintEngine(QPrinter::PrinterMode mode);

    // override QWin32PaintEngine
    bool begin(QPaintDevice *dev);
    bool end();

    void updateState(const QPaintEngineState &state);

    void updateMatrix(const QTransform &matrix);
    void updateClipPath(const QPainterPath &clip, Qt::ClipOperation op);

    void drawPath(const QPainterPath &path);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawTextItem(const QPointF &p, const QTextItem &textItem);

    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &p);
    void setProperty(PrintEnginePropertyKey key, const QVariant &value);
    QVariant property(PrintEnginePropertyKey key) const;

    bool newPage();
    bool abort();
    int metric(QPaintDevice::PaintDeviceMetric) const;

    QPrinter::PrinterState printerState() const;

    QPaintEngine::Type type() const { return Windows; }

    HDC getDC() const;
    void releaseDC(HDC) const;

    HDC getPrinterDC() const { return getDC(); }
    void releasePrinterDC(HDC dc) const { releaseDC(dc); }

private:
    friend class QPrintDialog;
    friend class QPageSetupDialog;
};

class QWin32PrintEnginePrivate : public QAlphaPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWin32PrintEngine)
public:
    QWin32PrintEnginePrivate() :
        hPrinter(0),
        globalDevMode(0),
        devMode(0),
        pInfo(0),
        hdc(0),
        mode(QPrinter::ScreenResolution),
        state(QPrinter::Idle),
        resolution(0),
        pageMarginsSet(false),
        num_copies(1),
        printToFile(false),
        fullPage(false),
        reinit(false),
        has_custom_paper_size(false)
    {
    }

    ~QWin32PrintEnginePrivate();


    /* Reads the default printer name and its driver (printerProgram) into
       the engines private data. */
    void queryDefault();

    /* Initializes the printer data based on the current printer name. This
       function creates a DEVMODE struct, HDC and a printer handle. If these
       structures are already in use, they are freed using release
    */
    void initialize();

    /* Initializes data in the print engine whenever the HDC has been renewed
    */
    void initHDC();

    /* Releases all the handles the printer currently holds, HDC, DEVMODE,
       etc and resets the corresponding members to 0. */
    void release();

    /* Queries the resolutions for the current printer, and returns them
       in a list. */
    QList<QVariant> queryResolutions() const;

    /* Resets the DC with changes in devmode. If the printer is active
       this function only sets the reinit variable to true so it
       is handled in the next begin or newpage. */
    void doReinit();

    /* Used by print/page setup dialogs */
    HGLOBAL *createDevNames();

    void readDevmode(HGLOBAL globalDevmode);
    void readDevnames(HGLOBAL globalDevnames);

    inline DEVMODEW *devModeW() const { return (DEVMODEW*) devMode; }
    inline DEVMODEA *devModeA() const { return (DEVMODEA*) devMode; }

    inline PRINTER_INFO_2W *pInfoW() { return (PRINTER_INFO_2W*) pInfo; };
    inline PRINTER_INFO_2A *pInfoA() { return (PRINTER_INFO_2A*) pInfo; };

    inline bool resetDC() {
        QT_WA( {
            hdc = ResetDCW(hdc, devModeW());
        }, {
            hdc = ResetDCA(hdc, devModeA());
        } );
        return hdc != 0;
    }

    void strokePath(const QPainterPath &path, const QColor &color);
    void fillPath(const QPainterPath &path, const QColor &color);

    void composeGdiPath(const QPainterPath &path);
    void fillPath_dev(const QPainterPath &path, const QColor &color);
    void strokePath_dev(const QPainterPath &path, const QColor &color, qreal width);

    void updateOrigin();

    void initDevRects();
    void setPageMargins(int margin_left, int margin_top, int margin_right, int margin_bottom);
    QRect getPageMargins() const;

    // Windows GDI printer references.
    HANDLE hPrinter;

    HGLOBAL globalDevMode;
    void *devMode;
    void *pInfo;
    HGLOBAL hMem;

    HDC hdc;

    QPrinter::PrinterMode mode;

    // Printer info
    QString name;
    QString program;
    QString port;

    // Document info
    QString docName;
    QString fileName;

    QPrinter::PrinterState state;
    int resolution;

    // This QRect is used to store the exact values
    // entered into the PageSetup Dialog because those are
    // entered in mm but are since converted to device coordinates.
    // If they were to be converted back when displaying the dialog
    // again, there would be inaccuracies so when the user entered 10
    // it may show up as 9.99 the next time the dialog is opened.
    // We don't want that confusion.
    QRect previousDialogMargins;

    bool pageMarginsSet;
    QRect devPageRect;
    QRect devPhysicalPageRect;
    QRect devPaperRect;
    qreal stretch_x;
    qreal stretch_y;
    int origin_x;
    int origin_y;

    int dpi_x;
    int dpi_y;
    int dpi_display;
    int num_copies;

    uint printToFile : 1;
    uint fullPage : 1;
    uint reinit : 1;

    uint complex_xform : 1;
    uint has_pen : 1;
    uint has_brush : 1;
    uint has_custom_paper_size : 1;

    uint txop;

    QColor brush_color;
    QPen pen;
    QColor pen_color;
    QSizeF paper_size;

    QTransform painterMatrix;
    QTransform matrix;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_WIN_P_H
