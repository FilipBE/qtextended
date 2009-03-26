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
#include <stdlib.h>
#include "samplebuffer.h"


SampleBuffer::SampleBuffer( unsigned int _blockSize, unsigned int _maxBlocks )
{
    blockSize = _blockSize;
    maxBlocks = _maxBlocks;
    init();
}


SampleBuffer::~SampleBuffer()
{
    clearAll();
}


bool SampleBuffer::nextWriteBuffer( short *& buf, unsigned int& length )
{
    if ( writePosn ) {
        buf = writePosn->data;
        length = blockSize;
        return true;
    } else {
        // The sample buffer ran out of memory or reached the block limit.
        return false;
    }
}


void SampleBuffer::commitWriteBuffer ( unsigned int length )
{
    // Bail out if nothing to commit.
    if ( !writePosn || !length )
        return;

    // Update the current block size.
    writePosn->size = length;

    // Allocate space for the next block.
    if ( !maxBlocks || numBlocks < maxBlocks ) {

        BufferBlock *block;
        block = (BufferBlock *)malloc( sizeof(BufferBlock) +
                                       ( blockSize - 1 ) * sizeof( short ) );
        if ( block ) {
            writePosn->next = block;
            block->next = 0;
            block->size = 0;
            writePosn = block;
            ++numBlocks;
        } else {
            // The process is out of memory.
            writePosn = 0;
        }

    } else {
        // We've reached the maximum block limit.
        writePosn = 0;
    }
}


void SampleBuffer::rewind()
{
    readPosn = first;
}


bool SampleBuffer::nextReadBuffer( short *& buf, unsigned int& length )
{
    while ( readPosn ) {
        buf = readPosn->data;
        length = readPosn->size;
        readPosn = readPosn->next;
        if ( length > 0 )
            return true;
    }
    return false;
}


void SampleBuffer::clear()
{
    clearAll();
    init();
}


void SampleBuffer::init()
{
    first = (BufferBlock *)malloc( sizeof(BufferBlock) +
                                   ( blockSize - 1 ) * sizeof( short ) );
    readPosn = first;
    writePosn = first;
    if ( first ) {
        first->next = 0;
        first->size = 0;
        numBlocks = 1;
    } else {
        numBlocks = 0;
    }
}


void SampleBuffer::clearAll()
{
    BufferBlock *block, *next;
    block = first;
    while ( block ) {
        next = block->next;
        free( block );
        block = next;
    }
    first = 0;
    readPosn = 0;
    writePosn = 0;
    numBlocks = 0;
}

