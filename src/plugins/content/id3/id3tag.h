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

#ifndef ID3TAG_H
#define ID3TAG_H

#include <QList>
#include <QPair>
#include <QDataStream>
#include <QBuffer>
#include <QVariant>
#include <QtCore/qendian.h>

#define ID3_FRAME_ID(a,b,c,d) (int(a) << 24) | (int(b) << 16) | (int(c) << 8) | (int(d))

class Id3Tag
{
public:
    enum FrameId
    {
        AENC = ID3_FRAME_ID('A','E','N','C'),
        APIC = ID3_FRAME_ID('A','P','I','C'),
        ASPI = ID3_FRAME_ID('A','S','P','I'),
        COMM = ID3_FRAME_ID('C','O','M','M'),
        COMR = ID3_FRAME_ID('C','O','M','R'),
        ENCR = ID3_FRAME_ID('E','N','C','R'),
        EQU2 = ID3_FRAME_ID('E','Q','U','2'),
        ETCO = ID3_FRAME_ID('E','T','C','O'),
        GEOB = ID3_FRAME_ID('G','E','O','B'),
        GRID = ID3_FRAME_ID('G','R','I','D'),
        LINK = ID3_FRAME_ID('L','I','N','K'),
        MCDI = ID3_FRAME_ID('M','C','D','I'),
        MLLT = ID3_FRAME_ID('M','L','L','T'),
        OWNE = ID3_FRAME_ID('O','W','N','E'),
        PRIV = ID3_FRAME_ID('P','R','I','V'),
        PCNT = ID3_FRAME_ID('P','C','N','T'),
        POPM = ID3_FRAME_ID('P','O','P','M'),
        POSS = ID3_FRAME_ID('P','O','S','S'),
        RBUF = ID3_FRAME_ID('R','B','U','F'),
        RVA2 = ID3_FRAME_ID('R','V','A','2'),
        RVRB = ID3_FRAME_ID('R','V','R','B'),
        SEEK = ID3_FRAME_ID('S','E','E','K'),
        SIGN = ID3_FRAME_ID('S','I','G','N'),
        SYLT = ID3_FRAME_ID('S','Y','L','T'),
        SYTC = ID3_FRAME_ID('S','Y','T','C'),
        TALB = ID3_FRAME_ID('T','A','L','B'),
        TBPM = ID3_FRAME_ID('T','B','P','M'),
        TCOM = ID3_FRAME_ID('T','C','O','M'),
        TCON = ID3_FRAME_ID('T','C','O','N'),
        TCOP = ID3_FRAME_ID('T','C','O','P'),
        TDEN = ID3_FRAME_ID('T','D','E','N'),
        TDLY = ID3_FRAME_ID('T','D','L','Y'),
        TDOR = ID3_FRAME_ID('T','D','O','R'),
        TDRC = ID3_FRAME_ID('T','D','R','C'),
        TDRL = ID3_FRAME_ID('T','D','R','L'),
        TDTG = ID3_FRAME_ID('T','D','T','G'),
        TENC = ID3_FRAME_ID('T','E','N','C'),
        TEXT = ID3_FRAME_ID('T','E','X','T'),
        TFLT = ID3_FRAME_ID('T','F','L','T'),
        TIPL = ID3_FRAME_ID('T','I','P','L'),
        TIT1 = ID3_FRAME_ID('T','I','T','1'),
        TIT2 = ID3_FRAME_ID('T','I','T','2'),
        TIT3 = ID3_FRAME_ID('T','I','T','3'),
        TKEY = ID3_FRAME_ID('T','K','E','Y'),
        TLAN = ID3_FRAME_ID('T','L','A','N'),
        TLEN = ID3_FRAME_ID('T','L','E','N'),
        TMCL = ID3_FRAME_ID('T','M','C','L'),
        TMED = ID3_FRAME_ID('T','M','E','D'),
        TMOO = ID3_FRAME_ID('T','M','O','O'),
        TOAL = ID3_FRAME_ID('T','O','A','L'),
        TOFN = ID3_FRAME_ID('T','O','F','N'),
        TOLY = ID3_FRAME_ID('T','O','L','Y'),
        TOPE = ID3_FRAME_ID('T','O','P','E'),
        TOWN = ID3_FRAME_ID('T','O','W','N'),
        TPE1 = ID3_FRAME_ID('T','P','E','1'),
        TPE2 = ID3_FRAME_ID('T','P','E','2'),
        TPE3 = ID3_FRAME_ID('T','P','E','3'),
        TPE4 = ID3_FRAME_ID('T','P','E','4'),
        TPOS = ID3_FRAME_ID('T','P','O','S'),
        TPRO = ID3_FRAME_ID('T','P','R','O'),
        TPUB = ID3_FRAME_ID('T','P','U','B'),
        TRCK = ID3_FRAME_ID('T','R','C','K'),
        TRSN = ID3_FRAME_ID('T','R','S','N'),
        TRSO = ID3_FRAME_ID('T','R','S','O'),
        TSOA = ID3_FRAME_ID('T','S','O','A'),
        TSOP = ID3_FRAME_ID('T','S','O','P'),
        TSOT = ID3_FRAME_ID('T','S','O','T'),
        TSRC = ID3_FRAME_ID('T','S','R','C'),
        TSSE = ID3_FRAME_ID('T','S','S','E'),
        TSST = ID3_FRAME_ID('T','S','S','T'),
        TXXX = ID3_FRAME_ID('T','X','X','X'),
        UFID = ID3_FRAME_ID('U','F','I','D'),
        USER = ID3_FRAME_ID('U','S','E','R'),
        USLT = ID3_FRAME_ID('U','S','L','T'),
        WCOM = ID3_FRAME_ID('W','C','O','M'),
        WCOP = ID3_FRAME_ID('W','C','O','P'),
        WOAF = ID3_FRAME_ID('W','O','A','F'),
        WOAR = ID3_FRAME_ID('W','O','A','R'),
        WOAS = ID3_FRAME_ID('W','O','A','S'),
        WORS = ID3_FRAME_ID('W','O','R','S'),
        WPAY = ID3_FRAME_ID('W','P','A','Y'),
        WPUB = ID3_FRAME_ID('W','P','U','B'),
        WXXX = ID3_FRAME_ID('W','X','X','X'),

        BUF = ID3_FRAME_ID('B','U','F','\0'),
        CNT = ID3_FRAME_ID('C','N','T','\0'),
        COM = ID3_FRAME_ID('C','O','M','\0'),
        CRA = ID3_FRAME_ID('C','R','A','\0'),
        CRM = ID3_FRAME_ID('C','R','M','\0'),
        ETC = ID3_FRAME_ID('E','T','C','\0'),
        EQU = ID3_FRAME_ID('E','Q','U','\0'),
        GEO = ID3_FRAME_ID('G','E','O','\0'),
        IPL = ID3_FRAME_ID('I','P','L','\0'),
        LNK = ID3_FRAME_ID('L','N','K','\0'),
        MCI = ID3_FRAME_ID('M','C','I','\0'),
        MLL = ID3_FRAME_ID('M','L','L','\0'),
        PIC = ID3_FRAME_ID('P','I','C','\0'),
        POP = ID3_FRAME_ID('P','O','P','\0'),
        REV = ID3_FRAME_ID('R','E','V','\0'),
        RVA = ID3_FRAME_ID('R','V','A','\0'),
        SLT = ID3_FRAME_ID('S','L','T','\0'),
        STC = ID3_FRAME_ID('S','T','C','\0'),
        TAL = ID3_FRAME_ID('T','A','L','\0'),
        TBP = ID3_FRAME_ID('T','B','P','\0'),
        TCM = ID3_FRAME_ID('T','C','M','\0'),
        TCO = ID3_FRAME_ID('T','C','O','\0'),
        TCR = ID3_FRAME_ID('T','C','R','\0'),
        TDA = ID3_FRAME_ID('T','D','A','\0'),
        TDY = ID3_FRAME_ID('T','D','Y','\0'),
        TEN = ID3_FRAME_ID('T','E','N','\0'),
        TFT = ID3_FRAME_ID('T','F','T','\0'),
        TIM = ID3_FRAME_ID('T','I','M','\0'),
        TKE = ID3_FRAME_ID('T','K','E','\0'),
        TLA = ID3_FRAME_ID('T','L','A','\0'),
        TLE = ID3_FRAME_ID('T','L','E','\0'),
        TMT = ID3_FRAME_ID('T','M','T','\0'),
        TOA = ID3_FRAME_ID('T','O','A','\0'),
        TOF = ID3_FRAME_ID('T','O','F','\0'),
        TOL = ID3_FRAME_ID('T','O','L','\0'),
        TOR = ID3_FRAME_ID('T','O','R','\0'),
        TOT = ID3_FRAME_ID('T','O','T','\0'),
        TP1 = ID3_FRAME_ID('T','P','1','\0'),
        TP2 = ID3_FRAME_ID('T','P','2','\0'),
        TP3 = ID3_FRAME_ID('T','P','3','\0'),
        TP4 = ID3_FRAME_ID('T','P','4','\0'),
        TPA = ID3_FRAME_ID('T','P','A','\0'),
        TPB = ID3_FRAME_ID('T','P','B','\0'),
        TRC = ID3_FRAME_ID('T','R','C','\0'),
        TRD = ID3_FRAME_ID('T','R','D','\0'),
        TRK = ID3_FRAME_ID('T','R','K','\0'),
        TSI = ID3_FRAME_ID('T','S','I','\0'),
        TSS = ID3_FRAME_ID('T','S','S','\0'),
        TT1 = ID3_FRAME_ID('T','T','1','\0'),
        TT2 = ID3_FRAME_ID('T','T','2','\0'),
        TT3 = ID3_FRAME_ID('T','T','3','\0'),
        TXT = ID3_FRAME_ID('T','X','T','\0'),
        TXX = ID3_FRAME_ID('T','X','X','\0'),
        TYE = ID3_FRAME_ID('T','Y','E','\0'),
        UFI = ID3_FRAME_ID('U','F','I','\0'),
        ULT = ID3_FRAME_ID('U','L','T','\0'),
        WAF = ID3_FRAME_ID('W','A','F','\0'),
        WAR = ID3_FRAME_ID('W','A','R','\0'),
        WAS = ID3_FRAME_ID('W','A','S','\0'),
        WCM = ID3_FRAME_ID('W','C','M','\0'),
        WPB = ID3_FRAME_ID('W','P','B','\0'),
        WXX = ID3_FRAME_ID('W','X','X','\0')
    };

    Id3Tag( QIODevice * );
    ~Id3Tag();

    quint32 size() const;

    bool isValid() const;

    bool nextFrame();

    int frameId() const;

    QVariantList frameValues() const;

private:

    enum TagHeaderFlag
    {
        TagUnsynchronisation = 0x80,
        TagHasExtendedHeader = 0x40,
        TagExperimental = 0x20,
        TagHasFooter = 0x10
    };

    enum TagExtendedHeaderFlag
    {
        Update = 0x40,
        Crc = 0x20,
        Restrictions = 0x10
    };

    enum TagRestrictionFlag
    {
        TagSizeMask = 0xC0,
        TagSize128_1024 = 0x00,
        TagSize64_128 = 0x40,
        TagSize32_40 =  0x80,
        TagSize32_4 = 0xC0,

        TextEncodingUtf8 = 0x20,

        TextLengthMask = 0x18,
        TextLength1024 = 0x08,
        TextLength128  = 0x10,
        TextLength30   = 0x18,

        ImageEncodingPngJpeg = 0x04,

        ImageSizeMask = 0x03,
        ImageSizeLe256 = 0x01,
        ImageSizeLe64 = 0x02,
        ImageSizeEq64 = 0x03
    };

    struct TagHeader
    {
        char id[ 3 ];
        union
        {
            quint16 version;
            struct
            {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
                quint8 majorVersion;
                quint8 minorVersion;
#else
                quint8 minorVersion;
                quint8 majorVersion;
#endif
            };
        };
        quint8 flags;
        quint32 size;
    };

    struct FrameHeader
    {
        union
        {
            quint32 id;
            char idBytes[ 4 ];
        };
        quint32 size;
        quint16 flags;
    };

    enum FrameFlag
    {
        FrameTagAlter  = 0x4000,
        FrameFileAlter = 0x0200,
        FrameReadOnly  = 0x0100,

        FrameGrouping = 0x0040,

        FrameCompression         = 0x0008,
        FrameEncryption          = 0x0004,
        FrameUnsynchronisation   = 0x0002,
        FrameDataLengthIndicator = 0x0001
    };

    void readExtendedHeaders();
    bool isValidFrame(const FrameHeader &header) const;
    bool readFrameHeader(FrameHeader *header);

    TagHeader m_tagHeader;
    FrameHeader m_frameHeader;

    QBuffer m_tagBuffer;
    mutable QDataStream m_stream;

    mutable QVariantList m_frameValues;
    mutable bool m_frameParsed;

    bool m_unsynchroniseFrames;
    bool m_isSyncSafe;
    bool m_isNotSyncSafe;
};

#undef ID3_FRAME_ID

#endif
