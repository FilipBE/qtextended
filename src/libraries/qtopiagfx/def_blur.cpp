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

#include "def_blur.h"
#include <QRgb>

template<int aprec, int zprec>
static inline void blurinner(unsigned char *bptr, int &zR, int &zG, int &zB, int &zA, int alpha)
{
  int R,G,B,A;
  R = *bptr;
  G = *(bptr+1);
  B = *(bptr+2);
  A = *(bptr+3);

  zR += (alpha * ((R<<zprec)-zR))>>aprec;
  zG += (alpha * ((G<<zprec)-zG))>>aprec;
  zB += (alpha * ((B<<zprec)-zB))>>aprec;
  zA += (alpha * ((A<<zprec)-zA))>>aprec;

  *bptr =     zR>>zprec;
  *(bptr+1) = zG>>zprec;
  *(bptr+2) = zB>>zprec;
  *(bptr+3) = zA>>zprec;
}

template<int aprec,int zprec>
static inline void blurrow( unsigned int *im, int width, int alpha)
{
  int zR,zG,zB,zA;

  QRgb *ptr = (QRgb *)im;

  zR = *((unsigned char *)ptr    )<<zprec;
  zG = *((unsigned char *)ptr + 1)<<zprec;
  zB = *((unsigned char *)ptr + 2)<<zprec;
  zA = *((unsigned char *)ptr + 3)<<zprec;

  for(int index = 1; index < width; index++)
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }
  for(int index = width - 2; index>=0; index--)
  {
    blurinner<aprec,zprec>((unsigned char *)&ptr[index],zR,zG,zB,zA,alpha);
  }
}

template<int aprec, int zprec>
static inline void blurcol(unsigned int *im, int height, int step, int alpha)
{
  int zR,zG,zB,zA;

  QRgb *ptr = (QRgb *)im;

  zR = *((unsigned char *)ptr    )<<zprec;
  zG = *((unsigned char *)ptr + 1)<<zprec;
  zB = *((unsigned char *)ptr + 2)<<zprec;
  zA = *((unsigned char *)ptr + 3)<<zprec;

  for(int index = 1; index < height; ++index)
  {
      blurinner<aprec,zprec>((unsigned char *)&ptr[index * step],zR,zG,zB,zA,alpha);
  }

  for(int index=height - 2; index >= 0; index--)
  {
      blurinner<aprec,zprec>((unsigned char *)&ptr[index * step],zR,zG,zB,zA,alpha);
  }

}

void def_blur32(unsigned int *data, int width, int height, int step_width, int alpha)
{
  for(int row = 0; row < height; row++)
  {
    blurrow<15,8>(data + row * step_width, width, alpha);
  }

  for(int col = 0; col< width; col++)
  {
    blurcol<15,8>(data + col, height, step_width, alpha);
  }
}

template<int aprec, int zprec>
static inline void blurinner_16(unsigned short *bptr, int &zR, int &zG, int &zB, int alpha)
{
  int R,G,B;
  R = *bptr >> 8 | 0x07;
  G = (*bptr & 0x07E0) >> 3 | 0x03;
  B = (*bptr & 0x001F) << 3 | 0x07;

  zR += (alpha * ((R<<zprec)-zR))>>aprec;
  zG += (alpha * ((G<<zprec)-zG))>>aprec;
  zB += (alpha * ((B<<zprec)-zB))>>aprec;

  *bptr = (zR>>zprec<<8) & 0xF800 |
          (zG>>zprec<<3) & 0x07E0 |
          (zB>>zprec>>3);
}

template<int aprec,int zprec>
static inline void blurrow_16(unsigned short *im, int width, int alpha)
{
  int zR,zG,zB;

  zR = (*im >> 8 | 0x07)<<zprec;
  zG = ((*im & 0x07E0) >> 3 | 0x03)<<zprec;
  zB = ((*im & 0x001F) << 3 | 0x07)<<zprec;

  for(int index = 1; index < width; index++)
  {
    blurinner_16<aprec,zprec>(&im[index],zR,zG,zB,alpha);
  }
  for(int index = width - 2; index>=0; index--)
  {
    blurinner_16<aprec,zprec>(&im[index],zR,zG,zB,alpha);
  }
}

template<int aprec, int zprec>
static inline void blurcol_16(unsigned short *im, int height, int step, int alpha)
{
  int zR,zG,zB;

  zR = (*im >> 8 | 0x07)<<zprec;
  zG = ((*im & 0x07E0) >> 3 | 0x03)<<zprec;
  zB = ((*im & 0x001F) << 3 | 0x07)<<zprec;

  for(int index = 1; index < height; ++index)
  {
      blurinner_16<aprec,zprec>(&im[index * step],zR,zG,zB,alpha);
  }

  for(int index=height - 2; index >= 0; index--)
  {
      blurinner_16<aprec,zprec>(&im[index * step],zR,zG,zB,alpha);
  }

}

void def_blur16(unsigned short *data, int width, int height, int step_width, int alpha)
{
    alpha >>= 1;
    for(int row = 0; row < height; row++)
    {
        blurrow_16<15,8>(data + row * step_width, width, alpha);
    }

    for(int col = 0; col< width; col++)
    {
        blurcol_16<15,8>(data + col, height, step_width, alpha);
    }
}

