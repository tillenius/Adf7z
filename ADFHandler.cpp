/* ADFHandler.cpp -- ADF format handler
2022 : Martin Tillenius : Public domain
This code is based on:
  PpmdHandler.cpp -- PPMd format handler
  2020 : Igor Pavlov : Public domain
which is based on:
  PPMd var.H (2001) / var.I (2002): Dmitry Shkarin : Public domain
  Carryless rangecoder (1999): Dmitry Subbotin : Public domain */

#include "7z/CPP/7zip/Archive/StdAfx.h"

#include "7z/C/CpuArch.h"
#include "7z/C/Alloc.h"

#include "7z/CPP/Common/ComTry.h"
#include "7z/CPP/Common/StringConvert.h"

#include "7z/CPP/Windows/PropVariant.h"
#include "7z/CPP/Windows/TimeUtils.h"

#include "7z/CPP/7zip/Common/CWrappers.h"
#include "7z/CPP/7zip/Common/ProgressUtils.h"
#include "7z/CPP/7zip/Common/RegisterArc.h"
#include "7z/CPP/7zip/Common/StreamUtils.h"

#include "ADF.h"

using namespace NWindows;

namespace NArchive {
namespace NADF {

class CHandler : public IInArchive, public IArchiveOpenSeq, public CMyUnknownImp {
  ADF::Disk m_disk;
  std::string m_method;
  std::string m_volumeName;
  std::vector<ADF::DiskEntry> m_items;
  CMyComPtr<ISequentialInStream> _stream;

public:
  MY_UNKNOWN_IMP2(IInArchive, IArchiveOpenSeq)
  INTERFACE_IInArchive(;)
  STDMETHOD(OpenSeq)(ISequentialInStream *stream);
};

static const Byte kProps[] = {
  kpidPath,
  kpidMTime,
  //kpidAttrib,
  kpidComment,
  kpidSize
};

static const Byte kArcProps[] = {
  kpidMethod,
  kpidVolumeName
};

IMP_IInArchive_Props
IMP_IInArchive_ArcProps

STDMETHODIMP CHandler::GetArchiveProperty(PROPID propID, PROPVARIANT *value) {
  NCOM::CPropVariant prop;
  switch (propID) {
    case kpidMethod:     prop = m_method.c_str(); break;
    case kpidVolumeName: prop = m_volumeName.c_str(); break;
  }
  prop.Detach(value);
  return S_OK;
}

STDMETHODIMP CHandler::GetNumberOfItems(UInt32 *numItems) {
  *numItems = m_items.size();
  return S_OK;
}

STDMETHODIMP CHandler::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value) {
  COM_TRY_BEGIN
  NCOM::CPropVariant prop;
  switch (propID) {
    case kpidPath:    prop = m_items[index].name.c_str(); break;
    case kpidComment: prop = m_items[index].comment.c_str(); break;
    case kpidMTime:   prop = m_items[index].filetime; break;
    case kpidSize:    prop = (UInt64) m_items[index].filesize; break;
    //case kpidAttrib:  prop = (UInt32) m_items[index].attrib; break;
  }
  prop.Detach(value);
  return S_OK;
  COM_TRY_END
}

STDMETHODIMP CHandler::Open(IInStream *stream, const UInt64 *, IArchiveOpenCallback *) {
  return OpenSeq(stream);
}

STDMETHODIMP CHandler::OpenSeq(ISequentialInStream *stream) {
  COM_TRY_BEGIN
  HRESULT res;
  try {
    Close();
    UInt32 size = 901120;
    m_disk.buffer.resize(size);
    res = stream->Read(m_disk.buffer.data(), size, &size);
    if (res == S_OK) {
      ADF::RootBlock rootBlock(m_disk.sector(ADF::ROOT_BLOCK));
      m_volumeName = rootBlock.GetVolumeName();
      ADF::process(m_disk, rootBlock.ht_begin(), rootBlock.ht_end(), "", m_items);
    }
  } catch(...) {
    res = S_FALSE;
  }
  return res;
  COM_TRY_END
}

STDMETHODIMP CHandler::Close() {
  m_items.clear();
  return S_OK;
}

STDMETHODIMP CHandler::Extract(const UInt32 *indices, UInt32 numItems, Int32 testMode, IArchiveExtractCallback *extractCallback) {
  bool allFilesMode = (numItems == (UInt32)(Int32)-1);
  if (allFilesMode)
    numItems = m_items.size();

  if (numItems == 0)
    return S_OK;

  UInt64 totalSize = 0;
  for (UInt32 i = 0; i < numItems; ++i) {
    const UInt32 index = (allFilesMode ? i : indices[i]);
    totalSize += m_items[index].filesize;
  }
  extractCallback->SetTotal(totalSize);

  UInt64 currentSize = 0;
  RINOK(extractCallback->SetCompleted(&currentSize));
  CMyComPtr<ISequentialOutStream> realOutStream;
  Int32 askMode = testMode ? NExtract::NAskMode::kTest : NExtract::NAskMode::kExtract;

  CLocalProgress *lps = new CLocalProgress;
  CMyComPtr<ICompressProgressInfo> progress = lps;
  lps->Init(extractCallback, true);

  std::vector<char> output;
  output.reserve(ADF::DISK_SIZE);
  for (UInt32 i = 0; i < numItems; ++i) {
    UInt32 index = (allFilesMode ? i : indices[i]);

    lps->InSize = currentSize;
    lps->OutSize = currentSize;
    RINOK(lps->SetCur());

    RINOK(extractCallback->GetStream(index, &realOutStream, askMode));

    if (m_items[index].type != ADF::ST_FILE) {
      RINOK(extractCallback->PrepareOperation(askMode));
      RINOK(extractCallback->SetOperationResult(NExtract::NOperationResult::kOK));
      continue;
    }

    bool skipMode = false;
    if (!testMode && !realOutStream) {
      skipMode = true;
      askMode = NExtract::NAskMode::kSkip;
    }
    RINOK(extractCallback->PrepareOperation(askMode));

    output.clear();
    ADF::extract(m_disk, m_items[index], output);

    if (realOutStream) {
      RINOK(WriteStream(realOutStream, output.data(), output.size()));
    }
    currentSize += output.size();
    RINOK(extractCallback->SetCompleted(&currentSize));
    realOutStream.Release();
  }

  Int32 opRes = NExtract::NOperationResult::kOK;
  return extractCallback->SetOperationResult(opRes);
}


static const Byte k_Signature[] = { 0x44, 0x4F, 0x53 }; // "DOS"

REGISTER_ARC_I(
  "ADF", "adf", 0, 0xb1, // name, extension, add-extension, id
  k_Signature,  // signature
  0,            // signature offset
  NArcInfoFlags::kFindSignature, // flags
  NULL)         // isArchive function
}}
