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

#ifndef SELECTEDITEMCONNECTOR_H
#define SELECTEDITEMCONNECTOR_H

#include <QObject>
#include <QTimeLine>
#include <QImageReader>

class SelectedItem;
class GridItem;


class SelectedItemConnector : public QObject
{
    Q_OBJECT

public:

    explicit SelectedItemConnector(SelectedItem *selectedItem);

    void triggerItemPressed(GridItem *);
    void triggerItemSelected(GridItem *);

signals:

    void itemPressed(GridItem *);
    void itemSelected(GridItem *);

    void selectionChanged(GridItem *);

public slots:

    void moving(int);

    void movingStateChanged(QTimeLine::State);

    void startAnimation();

    void animationStateChanged(QTimeLine::State);

    void animationChanged();

    void animationFinished();

    void animationError(QImageReader::ImageReaderError);

    void triggerSelectionChanged(GridItem *);

    void playing(int);

private:

    // Unimplemented methods to prevent copying and assignment.
    SelectedItemConnector(const SelectedItemConnector &);
    SelectedItemConnector & operator=(const SelectedItemConnector &);

    SelectedItem *selectedItem;
};

#endif
