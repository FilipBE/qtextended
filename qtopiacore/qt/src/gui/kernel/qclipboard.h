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

#ifndef QCLIPBOARD_H
#define QCLIPBOARD_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_CLIPBOARD

class QMimeSource;
class QMimeData;
class QImage;
class QPixmap;

class QClipboardPrivate;

class Q_GUI_EXPORT QClipboard : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QClipboard)
private:
    QClipboard(QObject *parent);
    ~QClipboard();

public:
    enum Mode { Clipboard, Selection, FindBuffer, LastMode = FindBuffer };

    void clear(Mode mode = Clipboard);

    bool supportsSelection() const;
    bool supportsFindBuffer() const;

    bool ownsSelection() const;
    bool ownsClipboard() const;
    bool ownsFindBuffer() const;

    QString text(Mode mode = Clipboard) const;
    QString text(QString& subtype, Mode mode = Clipboard) const;
    void setText(const QString &, Mode mode = Clipboard);

#ifdef QT3_SUPPORT
    QT3_SUPPORT QMimeSource *data(Mode mode = Clipboard) const;
    QT3_SUPPORT void setData(QMimeSource*, Mode mode = Clipboard);
#endif
    const QMimeData *mimeData(Mode mode = Clipboard ) const;
    void setMimeData(QMimeData *data, Mode mode = Clipboard);

    QImage image(Mode mode = Clipboard) const;
    QPixmap pixmap(Mode mode = Clipboard) const;
    void setImage(const QImage &, Mode mode  = Clipboard);
    void setPixmap(const QPixmap &, Mode mode  = Clipboard);

Q_SIGNALS:
    void changed(QClipboard::Mode mode);
    void selectionChanged();
    void findBufferChanged();
    void dataChanged();
private Q_SLOTS:
    void ownerDestroyed();

protected:
    void connectNotify(const char *);
    bool event(QEvent *);

    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QBaseApplication;
    friend class QDragManager;
    friend class QMimeSource;

private:
    Q_DISABLE_COPY(QClipboard)

    bool supportsMode(Mode mode) const;
    bool ownsMode(Mode mode) const;
    void emitChanged(Mode mode);
};

#endif // QT_NO_CLIPBOARD

QT_END_NAMESPACE

QT_END_HEADER

#endif // QCLIPBOARD_H
