/*
   Copyright (C) 2014, Rudolf Sikorski <rudolf.sikorski@freenet.de>

   This file is part of the `drwebmirror' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
   This code based on LZMA SDK v4.65, see lzma465.tar.bz2/C/LzmaUtil/LzmaUtil.c
*/

#include "drwebmirror.h"
#include "lzma/Alloc.h"
#include "lzma/7zFile.h"
#include "lzma/LzmaDec.h"

/* Begin of functions from LzmaUtil */
static void *SzAlloc(void *p, size_t size) { /*p = p*/(void)p; return MyAlloc(size); }
static void SzFree(void *p, void *address) { /*p = p*/(void)p; MyFree(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

#define IN_BUF_SIZE (1 << 16)
#define OUT_BUF_SIZE (1 << 16)

static SRes Decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
    UInt64 unpackSize)
{
  int thereIsSize = (unpackSize != (UInt64)(Int64)-1);
  Byte inBuf[IN_BUF_SIZE];
  Byte outBuf[OUT_BUF_SIZE];
  size_t inPos = 0, inSize = 0, outPos = 0;
  LzmaDec_Init(state);
  for (;;)
  {
    if (inPos == inSize)
    {
      inSize = IN_BUF_SIZE;
      RINOK(inStream->Read(inStream, inBuf, &inSize));
      inPos = 0;
    }
    {
      SRes res;
      SizeT inProcessed = inSize - inPos;
      SizeT outProcessed = OUT_BUF_SIZE - outPos;
      ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
      ELzmaStatus status;
      if (thereIsSize && outProcessed > unpackSize)
      {
        outProcessed = (SizeT)unpackSize;
        finishMode = LZMA_FINISH_END;
      }

      res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessed,
        inBuf + inPos, &inProcessed, finishMode, &status);
      inPos += inProcessed;
      outPos += outProcessed;
      unpackSize -= outProcessed;

      if (outStream)
        if (outStream->Write(outStream, outBuf, outPos) != outPos)
          return SZ_ERROR_WRITE;

      outPos = 0;

      if (res != SZ_OK || (thereIsSize && unpackSize == 0))
        return res;

      if (inProcessed == 0 && outProcessed == 0)
      {
        if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
          return SZ_ERROR_DATA;
        return res;
      }
    }
  }
}

static SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream)
{
  UInt64 unpackSize;
  int i;
  SRes res = 0;

  CLzmaDec state;

  /* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
  unsigned char header[LZMA_PROPS_SIZE + 8];

  /* Read and parse header */

  RINOK(SeqInStream_Read(inStream, header, sizeof(header)));

  unpackSize = 0;
  for (i = 0; i < 8; i++)
    unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

  LzmaDec_Construct(&state);
  RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));
  res = Decode2(&state, outStream, inStream, unpackSize);
  LzmaDec_Free(&state, &g_Alloc);
  return res;
}
/* End of functions from LzmaUtil */

/* USE_WINDOWS_FILE is not implemented */
#if defined(USE_WINDOWS_FILE)
#error USE_WINDOWS_FILE is not implemented!
#endif

/* Decompress LZMA archive <input> to file <output> */
int decompress_lzma(FILE * input, FILE * output)
{
    CFileSeqInStream inStream;
    CFileOutStream outStream;
    int result;

    if(!input || !output)
        return EXIT_FAILURE;

    FileSeqInStream_CreateVTable(&inStream);
    File_Construct(& inStream.file);
    FileOutStream_CreateVTable(&outStream);
    File_Construct(& outStream.file);

    inStream.file.file = input;
    outStream.file.file = output;

    result = Decode(& outStream.s, & inStream.s);

    if(result != SZ_OK)
    {
        if(result == SZ_ERROR_MEM)
            fprintf(ERRFP, "Error: Can not allocate memory\n");
        else if(result == SZ_ERROR_DATA)
            fprintf(ERRFP, "Error: Data error\n");
        else if(result == SZ_ERROR_WRITE)
            fprintf(ERRFP, "Error: Can not write output file\n");
        else if(result == SZ_ERROR_READ)
            fprintf(ERRFP, "Error: Can not read input file\n");
        else
            fprintf(ERRFP, "Error: LZMA error %x\n", (unsigned)result);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
