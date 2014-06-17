/*
 * Common Plugin code for OpenH323/OPAL
 *
 * This code is based on the following files from the OPAL project which
 * have been removed from the current build and distributions but are still
 * available in the CVS "attic"
 * 
 *    src/codecs/h263codec.cxx 
 *    include/codecs/h263codec.h 

 * The original files, and this version of the original code, are released under the same 
 * MPL 1.0 license. Substantial portions of the original code were contributed
 * by Salyens and March Networks and their right to be identified as copyright holders
 * of the original code portions and any parts now included in this new copy is asserted through 
 * their inclusion in the copyright notices below.
 *
 * Copyright (C) 2006 Post Increment
 * Copyright (C) 2005 Salyens
 * Copyright (C) 2001 March Networks Corporation
 * Copyright (C) 1999-2000 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Open H323 Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): Guilhem Tardy (gtardy@salyens.com)
 *                 Craig Southeren (craigs@postincrement.com)
 *                 Matthias Schneider (ma30002000@yahoo.de)
 */

#ifndef __DYNA_H__
#define __DYNA_H__ 1

//#include "ffmpeg.h"

#include <windows.h>
#include <malloc.h>

#define STRCMPI  _strcmpi
#include <string.h>
#include "critsect.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/mathematics.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
//#include <avcodec.h>
//#include <mathematics.h>
//#include <avformat.h>
}	  
/////////////////////////////////////////////////////////////////
//
// define a class to simplify handling a DLL library
// based on PDynaLink from PWLib



class FFMPEGLibrary 
{
  public:
    FFMPEGLibrary();
    ~FFMPEGLibrary();

    bool Load();

    AVCodec *AvcodecFindEncoder(enum AVCodecID id);
    AVCodec *AvcodecFindDecoder(enum AVCodecID id);
    AVCodecContext *AvcodecAllocContext(const AVCodec* avcodec);
    AVFrame *AvcodecAllocFrame(void);
    int AvcodecOpen(AVCodecContext *ctx, AVCodec *codec);
    int AvcodecClose(AVCodecContext *ctx);
    int AvcodecEncodeVideo(AVCodecContext *ctx, BYTE *buf, int buf_size, const AVFrame *pict);
    int AvcodecDecodeVideo(AVCodecContext *ctx, AVFrame *pict, int *got_picture_ptr, BYTE *buf, int buf_size);
    void AvcodecFree(void * ptr);
    void AvSetDimensions(AVCodecContext *s, int width, int height);

    void * AvMalloc(int size);
    void AvFree(void * ptr);

    void AvLogSetLevel(int level);
    void AvLogSetCallback(void (*callback)(void*, int, const char*, va_list));
    int FFCheckAlignment(void);

    bool IsLoaded();
    CriticalSection processLock;

  protected:
    bool isLoadedOK;
};



#endif /* __DYNA_H__ */
