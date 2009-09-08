/*
 NUI3 - C++ cross-platform GUI framework for OpenGL based applications
 Copyright (C) 2002-2003 Sebastien Metrot
 
 licence: see nui3/LICENCE.TXT
 */

#include "nui.h"
#include "nuiHTMLView.h"
#include "nuiHTTP.h"
#include "nuiFontManager.h"
#include "nuiHTTP.h"
#include "nuiUnicode.h"

#include "nuiHTMLContext.h"
#include "nuiHTMLItem.h"
#include "nuiHTMLBox.h"
#include "nuiHTMLHeader.h"
#include "nuiHTMLText.h"
#include "nuiHTMLImage.h"
#include "nuiHTMLFont.h"


/////////////////////////////// nuiHTMLView
nuiHTMLView::nuiHTMLView(float IdealWidth)
{
  if (SetObjectClass(_T("nuiHTMLView")))
    InitAttributes();
  
  mTextColorSet = false;
  mFontChanged = false;
  mTextColor.Set(0,0,0);
  mpFont = NULL;
  
  InitContext();
  
  mpHTML = NULL;
  mpRootBox = NULL;
  mIdealWidth = IdealWidth;
  mVSpace = 2.0f;
  mHSpace = 0.0f;
}

nuiHTMLView::~nuiHTMLView()
{
  delete mpHTML;
}

void nuiHTMLView::InitAttributes()
{
  AddAttribute(new nuiAttribute<const nglString&>
   (nglString(_T("Font")), nuiUnitName,
    nuiMakeDelegate(this, &nuiHTMLView::_GetFont), 
    nuiMakeDelegate(this, &nuiHTMLView::_SetFont)));
  
  AddAttribute(new nuiAttribute<const nuiColor&>
   (nglString(_T("TextColor")), nuiUnitNone,
    nuiMakeDelegate(this, &nuiHTMLView::GetTextColor), 
    nuiMakeDelegate(this, &nuiHTMLView::SetTextColor)));
  
}


void nuiHTMLView::InitContext()
{
  nuiFontRequest font(mpFont, false);
  mContext.mFont = font;
  mContext.mUnderline = false;
  mContext.mStrikeThrough = false;
  mContext.mTextFgColor = mTextColor;

  mContext.mLeftMargin = 0;
  mContext.mMaxWidth = 0;
  mContext.mVSpace = 0;
  mContext.mHSpace = 0;
  mContext.mTextBgColor = nuiColor(255,255,255);
}


void nuiHTMLView::SetFont(nuiFont* pFont, bool AlreadyAcquired)
{
  if (!pFont)
  {
    AlreadyAcquired = true;
    pFont = nuiFont::GetFont(14.0f);
  }
  
  if (pFont == mpFont)
  {
    if (AlreadyAcquired)
    {
      mpFont->Release();
    }
    return;
  }
  
  if(mpFont)
    mpFont->Release();
  
  mpFont = pFont;
  if (!AlreadyAcquired)
    mpFont->Acquire();
  
  mFontChanged = true;
  InvalidateLayout();
}

void nuiHTMLView::SetFont(nuiFontRequest& rFontRequest)
{
  nuiFont* pFont = nuiFontManager::GetManager().GetFont(rFontRequest);
  if (pFont)
    SetFont(pFont, true);
}

void nuiHTMLView::SetFont(const nglString& rFontSymbol)
{
  nuiFont* pFont = nuiFont::GetFont(rFontSymbol);
  if (pFont)
    SetFont(pFont, true);
}


void nuiHTMLView::_SetFont(const nglString& rFontSymbol)
{
  SetFont(rFontSymbol);
  InitContext();
}

const nglString& nuiHTMLView::_GetFont() const
{
  if (mpFont)
    return mpFont->GetObjectName();
  return nglString::Null;
}

const nuiColor& nuiHTMLView::GetTextColor() const
{
  return mTextColor;
}

void nuiHTMLView::SetTextColor(const nuiColor& Color)
{
  mTextColorSet = true;
  mTextColor = Color;
  InitContext();
  Invalidate();
}


nuiRect nuiHTMLView::CalcIdealSize()
{
  float IdealWidth = mIdealWidth;
  if (mRect.GetWidth() > 0)
    IdealWidth = mRect.GetWidth();
  Clear();
//  context.mSetLayout = true;
//  WalkTree(mpHTML, context);
  //  return nuiRect(context.mMaxWidth, context.mH);

  mContext.mMaxWidth = IdealWidth;
  if (!mpRootBox)
    return nuiRect(IdealWidth, 400.0f);
  mpRootBox->Layout(mContext);
  return nuiRect(mpRootBox->GetIdealRect().GetWidth(), mpRootBox->GetIdealRect().GetHeight());
}

bool nuiHTMLView::SetRect(const nuiRect& rRect)
{
  nuiWidget::SetRect(rRect);
  if (!mpRootBox)
    return true;

  mContext.mMaxWidth = mRect.GetWidth();
  mpRootBox->Layout(mContext);
  mpRootBox->SetRect(mpRootBox->GetIdealRect());
  return true;
}

bool nuiHTMLView::Draw(nuiDrawContext* pContext)
{
  nuiSimpleContainer::Draw(pContext);
  pContext->SetBlendFunc(nuiBlendTransp);
  pContext->EnableBlending(true);
  if (mpRootBox)
  {
    mpRootBox->CallDraw(pContext);
    
  }
  return true;
}

void nuiHTMLView::SetIdealWidth(float IdealWidth)
{
  mIdealWidth = IdealWidth;
  InvalidateLayout();
}

float nuiHTMLView::GetIdealWidth() const
{
  return mIdealWidth;
}

float nuiHTMLView::GetVSpace() const
{
  return mVSpace;
}

float nuiHTMLView::GetHSpace() const
{
  return mHSpace;
}

void nuiHTMLView::SetVSpace(float VSpace)
{
  mVSpace = VSpace;
  InvalidateLayout();
}

void nuiHTMLView::SetHSpace(float HSpace)
{
  mHSpace = HSpace;
  InvalidateLayout();
}

bool nuiHTMLView::SetText(const nglString& rHTMLText)
{
  Clear();
  nuiHTML* pHTML = new nuiHTML();
  
  std::string str(rHTMLText.GetStdString());
  nglIMemory mem(&str[0], str.size());
  bool res = pHTML->Load(mem);
  
  if (res)
  {
    Clear();
    delete mpHTML;
    mpHTML = pHTML;
    mpRootBox = new nuiHTMLBox(mpHTML, false);
    ParseTree(mpHTML, mpRootBox);
    nuiHTMLContext context;
    mpRootBox->Layout(context);
    InvalidateLayout();
  }
  return res;
}

bool nuiHTMLView::SetURL(const nglString& rURL)
{
  nglString url(rURL);
  nuiHTTPRequest request(rURL);
  nuiHTTPResponse* pResponse = request.SendRequest();
  if (!pResponse)
    return false;

  //NGL_OUT(_T("\n\nHTTP Headers:\n%ls\n\n"), pResponse->GetHeadersRep().GetChars());
  
  const nuiHTTPHeaderMap& rHeaders(pResponse->GetHeaders());
  nuiHTTPHeaderMap::const_iterator it = rHeaders.find(_T("location"));
  if (it != rHeaders.end())
  {
    nglString newurl = it->second;
    if (newurl[0] == '/')
    {
      url.TrimRight('/');
      url += newurl;
    }
    else
    {
      url = newurl;
    }
    //NGL_OUT(_T("\n\nNew location: %ls\n\n"), url.GetChars());
    
    delete pResponse;
    return SetURL(url);
  }

  it = rHeaders.find(_T("content-type"));

  nglTextEncoding encoding = eEncodingUnknown;
  if (it != rHeaders.end())
  {  
    nglString contents(it->second);
    contents.ToUpper();
    int32 pos = contents.Find(_T("CHARSET="));
    if (pos >= 0)
    {
      nglString enc(contents.Extract(pos + 8));
      enc.Trim();
      encoding = nuiGetTextEncodingFromString(enc);
      //NGL_OUT(_T("\n\nHTTP Encoding: %ls - %d\n\n"), enc.GetChars(), encoding);

    }
  }
  
  
  nuiHTML* pHTML = new nuiHTML();
  pHTML->SetSourceURL(url);
  nglIMemory mem(&pResponse->GetBody()[0], pResponse->GetBody().size());
  
  bool res = pHTML->Load(mem, encoding);
  
  if (res)
  {
    Clear();
    delete mpHTML;
    mpHTML = pHTML;
    mpRootBox = new nuiHTMLBox(mpHTML, false);
    ParseTree(mpHTML, mpRootBox);
    nuiHTMLContext context;
    mpRootBox->Layout(context);
    InvalidateLayout();
  }
  return res;
}

const nglString& nuiHTMLView::GetURL() const
{
  if (mpHTML)
    return mpHTML->GetSourceURL();
  return nglString::Null;
}

void nuiHTMLView::ParseTree(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  uint32 count = pNode->GetNbChildren();
  for (uint32 i = 0; i < count; i++)
  {
    nuiHTMLNode* pChild = pNode->GetChild(i);
    switch (pChild->GetTagType())
    {
      case nuiHTML::eTag_HTML:
        {
          ParseHTML(pChild, pBox);
          return;
        }
        break;
      default:
        {
//          printf("tree??? '%ls'\n", pChild->GetName().GetChars());
//          ParseTree(pChild, pBox); // Try all children!!!
        }
        break;
    }
  }
}

void nuiHTMLView::ParseHTML(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  uint32 count = pNode->GetNbChildren();
  for (uint32 i = 0; i < count; i++)
  {
    nuiHTMLNode* pChild = pNode->GetChild(i);
    switch (pChild->GetTagType())
    {
      case nuiHTML::eTag_HEAD:
        ParseHead(pChild, pBox);
        break;
        
      case nuiHTML::eTag_BODY:
        ParseBody(pChild, pBox);
        break;
        
      default:
        {        
          //printf("html??? '%ls'\n", pChild->GetName().GetChars());
        }
        break;
    }
  }
}

void nuiHTMLView::ParseHead(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  uint32 count = pNode->GetNbChildren();
  for (uint32 i = 0; i < count; i++)
  {
    nuiHTMLNode* pChild = pNode->GetChild(i);
    switch (pChild->GetTagType())
    {
      case nuiHTML::eTag_TITLE:
        ParseTitle(pChild, pBox);
        break;
      default:
        {        
          //printf("head??? '%ls'\n", pChild->GetName().GetChars());
        }
        break;
    }
  }
}

void nuiHTMLView::ParseTitle(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
}

void nuiHTMLView::ParseBody(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  uint32 count = pNode->GetNbChildren();
  for (uint32 i = 0; i < count; i++)
  {
    nuiHTMLNode* pChild = pNode->GetChild(i);
    switch (pChild->GetTagType())
    {
      case nuiHTML::eTag_DIV:
        ParseDiv(pChild, pBox);
        break;
      case nuiHTML::eTag_TABLE:
        ParseTable(pChild, pBox);
        break;
      case nuiHTML::eTag_IMG:
        ParseImage(pChild, pBox);
        break;
      case nuiHTML::eTag_UL:
      case nuiHTML::eTag_OL:
      case nuiHTML::eTag_DL:
        ParseList(pChild, pBox);
        break;
      case nuiHTML::eTag_P:
        ParseP(pChild, pBox);
        break;
      case nuiHTML::eTag_H1:
      case nuiHTML::eTag_H2:
      case nuiHTML::eTag_H3:
      case nuiHTML::eTag_H4:
      case nuiHTML::eTag_H5:
      case nuiHTML::eTag_H6:
        ParseHeader(pChild, pBox);
        break;
      case nuiHTML::eTag_I:
      case nuiHTML::eTag_B:
      case nuiHTML::eTag_U:
      case nuiHTML::eTag_STRIKE:
      case nuiHTML::eTag_STRONG:
      case nuiHTML::eTag_EM:
        ParseFormatTag(pChild, pBox);
        break;
      case nuiHTML::eTag_BR:
        ParseBr(pChild, pBox);
        break;
      case nuiHTML::eTag_A:
        ParseA(pChild, pBox);
        break;
      case nuiHTML::eTag_SPAN:
        ParseSpan(pChild, pBox);
        break;
      case nuiHTML::eTag_FONT:
        ParseFont(pChild, pBox);
        break;
      case nuiHTML::eTag_SCRIPT:
      case nuiHTML::eTag_COMMENT:
        // Skip those tags
        break;
      default:
        if (pChild->GetName().IsEmpty())
          ParseText(pChild, pBox);
        else
        {
          //printf("body??? '%ls'\n", pChild->GetName().GetChars());
          ParseBody(pChild, pBox);
        }
        break;
    }
  }
}

void nuiHTMLView::ParseText(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  const nglString& rText(pNode->GetText());
  std::vector<nglString> words;
  rText.Tokenize(words);
  
  for (uint32 i = 0; i < words.size(); i++)
  {
    pBox->AddItem(new nuiHTMLText(pNode, words[i]));
  }
  //ParseBody(pNode, pBox);
}

void nuiHTMLView::ParseDiv(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  nuiHTMLBox* pNewBox = new nuiHTMLBox(pNode, false);
  pBox->AddItem(pNewBox);
  
  ParseBody(pNode, pNewBox);
}

void nuiHTMLView::ParseTable(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  nuiHTMLBox* pNewBox = new nuiHTMLBox(pNode, false);
  pBox->AddItem(pNewBox);
  
  uint32 count = pNode->GetNbChildren();
  for (uint32 i = 0; i < count; i++)
  {
    nuiHTMLNode* pChild = pNode->GetChild(i);
    switch (pChild->GetTagType())
    {
      case nuiHTML::eTag_TR:
        ParseTableRow(pChild, pNewBox);
        break;
    }
  }
}

void nuiHTMLView::ParseImage(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  nuiHTMLImage* pImg = new nuiHTMLImage(pNode);
  pBox->AddItem(pImg);
}

void nuiHTMLView::ParseTableRow(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  nuiHTMLBox* pNewBox = new nuiHTMLBox(pNode, false);
  pBox->AddItem(pNewBox);
  
  uint32 count = pNode->GetNbChildren();
  for (uint32 i = 0; i < count; i++)
  {
    nuiHTMLNode* pChild = pNode->GetChild(i);
    switch (pChild->GetTagType())
    {
      case nuiHTML::eTag_TD:
        ParseBody(pChild, pNewBox);
        break;
    }
  }
}


void nuiHTMLView::ParseList(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  //printf("html list\n");
  nuiHTMLBox* pListBox = new nuiHTMLBox(pNode, false);
  pBox->AddItem(pListBox);
  
  uint32 count = pNode->GetNbChildren();
  for (uint32 i = 0; i < count; i++)
  {
    nuiHTMLNode* pListItem = pNode->GetChild(i);
    switch (pListItem->GetTagType())
    {
      case nuiHTML::eTag_LI:
      {
        nuiHTMLBox* pListItemBox = new nuiHTMLBox(pListItem, false);
        pListBox->AddItem(pListItemBox);
        ParseBody(pListItem, pListItemBox);
      }
      break;
    }
  }
  //printf("html /list\n");
}

void nuiHTMLView::ParseP(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  nuiHTMLBox* pNewBox = new nuiHTMLBox(pNode, false);
  pNewBox->ForceLineBreak(true);
  pBox->AddItem(pNewBox);
  
  ParseBody(pNode, pNewBox);
}

void nuiHTMLView::ParseHeader(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  nuiHTMLBox* pNewBox = new nuiHTMLHeader(pNode);
  pBox->AddItem(pNewBox);
  
  ParseBody(pNode, pNewBox);
}

void nuiHTMLView::ParseFormatTag(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  pBox->AddItem(new nuiHTMLItem(pNode, true));
  ParseBody(pNode, pBox);
  pBox->AddItemEnd(new nuiHTMLItem(pNode, true));
}

void nuiHTMLView::ParseA(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  pBox->AddItem(new nuiHTMLItem(pNode, true));
  ParseBody(pNode, pBox);
  pBox->AddItemEnd(new nuiHTMLItem(pNode, true));
}

void nuiHTMLView::ParseBr(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  pBox->AddItem(new nuiHTMLItem(pNode, false));
}

void nuiHTMLView::ParseSpan(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  ParseBody(pNode, pBox);
}

void nuiHTMLView::ParseFont(nuiHTMLNode* pNode, nuiHTMLBox* pBox)
{
  pBox->AddItem(new nuiHTMLFont(pNode));
  
  ParseBody(pNode, pBox);
  pBox->AddItemEnd(new nuiHTMLFont(pNode));
}

