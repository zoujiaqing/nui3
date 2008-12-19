/*
 NUI3 - C++ cross-platform GUI framework for OpenGL based applications
 Copyright (C) 2002-2003 Sebastien Metrot
 
 licence: see nui3/LICENCE.TXT
 */

#pragma once

#include "nui.h"
#include "nuiSampleInfo.h"

/// Generic Class that reads in a Sound File
class nuiSampleReader
{
public:
  nuiSampleReader(nglIStream& rStream); //< Constructor \param rStream The nglIStream in which we read
  nuiSampleReader(const nuiSampleReader& rReader, nglIStream& rStream);
  virtual ~nuiSampleReader();  //< Destructor
  
  virtual nuiSampleReader* Clone(nglIStream& rStream) const = 0;
  //
  virtual bool ReadInfo(nuiSampleInfo& rInfos) = 0;  ///< Method that reads Infos of the Sound \param rInfos Variable that receives Sample infos read in stream \return True if method succeeded
  virtual uint32 Read(void* pBuffer, uint32 SampleFrames, nuiSampleBitFormat format) = 0;  ///< Method that reads samples from stream \param pBuffer Pointer to a buffer that retrieves samples \param SampleFrames Number of sample frames to read \param SampleBitFormat Wanted format of samples in pBuffer \return True if method succeeded
  //
  bool SetPosition(uint64 SamplePos); ///< Method that sets the position of read cursor in sound \param SamplePos Position in sample frames \return True if method succeeded
  uint64 GetPosition() const; ///< Method that gets position of read cursor in sound \return The position
  
  bool IsInfoRead();
  const nuiSampleInfo& GetSampleInfo() const;

protected:
  nglIStream& mrStream;
  nuiSampleInfo mSampleInfo;
  bool mIsChunksScanned;
  uint64 mPosition; ///< Position of Read cursor
  uint8 mDataChunkIndex;  ///< Index of the Data Chunk in the Chunk Management Vectors
  
  bool mIsSampleInfoRead;

  //Chunk Management
  std::vector<nglFileOffset> mChunkIDPosition;   ///< Vector that contains positions of the chunk IDs
  std::vector<nglFileOffset> mChunkDataPosition; ///< Vector that contains positions of datas in selected chunk
  std::vector<nglFileOffset> mChunkSize;         ///< Vector that contains Sizes of chunks
  std::vector< std::vector<char> > mChunkID;     ///< Vector that contains Chunk IDs
  
  bool ScanAllChunks(); ///< Method that Scans the nglIStream and fills Chunk Management vectors \return True if method succeeded
  bool GoToChunk(uint8 ChunkNum); ///< Method that sets the position in stream to the data position in choosen chunk\param ChunkNum Index of the choosen chunk \return True if method succeeded
  bool ReadChunkSize(uint8 ChunkNum, uint32& ChunkSize);  ///< Method that reads choosen chunk Size \param ChunkNum Index of the choosen chunk \para ChunkSize variable that receives the size
  
private:
  nglFileOffset mPositionInStream;
};



