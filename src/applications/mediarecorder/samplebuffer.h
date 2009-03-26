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
#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

#include <qglobal.h>


class SampleBuffer
{
public:
    SampleBuffer( unsigned int blockSize, unsigned int maxBlocks = 0 );
    ~SampleBuffer();

    bool nextWriteBuffer( short *& buf, unsigned int& length );
    void commitWriteBuffer ( unsigned int length );
    void rewind();
    bool nextReadBuffer( short *& buf, unsigned int& length );
    void clear();

private:
    struct BufferBlock
    {
        struct BufferBlock *next;
        unsigned int size;
        short data[1];
    };

private:
    BufferBlock *first;
    BufferBlock *readPosn;
    BufferBlock *writePosn;
    unsigned int blockSize;
    unsigned int numBlocks;
    unsigned int maxBlocks;

    void init();
    void clearAll();
};


#endif

