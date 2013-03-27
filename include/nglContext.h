/*
 NUI3 - C++ cross-platform GUI framework for OpenGL based applications
 Copyright (C) 2002-2003 Sebastien Metrot & Vincent Caron
 
 licence: see nui3/LICENCE.TXT
 */



/*!
\file  nglContext.h
\brief OpenGL context management

This class is not available if the _NOGFX_ symbol is defined.
*/

#ifndef __nglContext_h__
#define __nglContext_h__

//#include "nui.h"

#ifndef _NOGFX_

#include "nglError.h"

#ifdef _X11_
#include <X11/Xutil.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#endif // _X11_

class nuiPainter;

//! OpenGL context description
/*!
This simple container stores the different properties of a given GL context
and provide some helpers to fill in the values.
*/
enum nglTargetAPI
{
  eTargetAPI_None,
  eTargetAPI_OpenGL,
  eTargetAPI_OpenGL2,
#ifdef _WIN32_
  eTargetAPI_Direct3D // This one is only valid under win32
#else
  eTargetAPI_Direct3D = eTargetAPI_None // If Direct3D is asked for a platform where it doesn't exists don't even create an opengl context
#endif
};

class NGL_API nglContextInfo
{
public:
  uint FrameCnt;     ///< Number of frame buffers (two means double-buffering)
  uint FrameBitsR;   ///< Bits per red component (frame buffer)
  uint FrameBitsG;   ///< Bits per green component (frame buffer)
  uint FrameBitsB;   ///< Bits per blue component (frame buffer)
  uint FrameBitsA;   ///< Bits per alpha component (frame buffer)
  uint DepthBits;    ///< Depth buffer resolution (ie. Z buffer, 0 means no Z buffer)
  uint StencilBits;  ///< Stencil buffer resolution (0 means no stencil)
  uint AccumBitsR;   ///< Bits per red component (accumulator buffer)
  uint AccumBitsG;   ///< Bits per green component (accumulator buffer)
  uint AccumBitsB;   ///< Bits per blue component (accumulator buffer)
  uint AccumBitsA;   ///< Bits per alpha component (accumulator buffer)
  uint AuxCnt;       ///< Number of auxiliary buffers
  uint AABufferCnt;  ///< Number of anti-aliasing buffers
  uint AASampleCnt;  ///< Anti-alisaing oversampling count
  bool Stereo;       ///< Stereoscopic display
  bool Offscreen;         ///< This context can render in memory instead of to a window. (false by default).
  bool RenderToTexture;   ///< This context must be able to be bound as a texture. (false by default)
  bool CopyOnSwap;        ///< This context must be able to use copy the back buffer to the front buffer instead of swaping them. (false by default)
  bool VerticalSync;      ///< Synchronize backbuffer swap to screen with the vertical sync of the screen. Makes animations much smoother.
  
  nglTargetAPI TargetAPI; 

  /** @name Life cycle */
  //@{
  nglContextInfo();
  /*!<
    Build a 'reasonable' default GL context description.

    The context info shows the following properties :

    - RGBA color model (not indexed mode)
    - the frame buffer has the current desktop's bitdepth 
    - double buffering or more
    - at least 16 bits of depth buffer
    - main plane layer (not overlay, not underlay)
  */
  nglContextInfo(const nglContextInfo& rInfo);  ///< Copy constructor
  //@}

  /** @name Logging */
  //@{
  void Dump(uint Level) const;  ///< Dumps human readable context information using verbosity level \a Level
  //@}

  /** @name Context enumeration */
  //@{
  static bool Enum (uint Index, nglContextInfo& rInfo);
  /*!< Enumerate supported GL contexts
    \param Index zero-based index
    \param rInfo context description is written here
    \return if true, \a rInfo contains valid informations

    Example :

\code
void DumpContexts()
{
  nglContextInfo info;
  uint index = 0;

  NGL_LOG(_T("context"), NGL_LOG_INFO, _T("Enumerating available GL contexts :"));
  while (nglContextInfo::Enum(index++, info))
    info.Dump(NGL_LOG_INFO);
}
\endcode

    \b Note : on multiple display systems, the enumeration is made on the
    default/main display device.
  */
  //@}

private:
#ifdef _WIN32_
  int mPFD;

  nglContextInfo (HDC hDC, int PFD);

  int GetPFD(HDC hDC) const;
  friend class nglOffscreenContext;
#endif // _WIN32_

#ifdef _X11_
  static XVisualInfo* mpXVisualInfoList;
  static uint         mXVisualInfoListCnt;

  static bool         Init();
  static void         Exit();

  XVisualInfo* mpXVisualInfo;

  nglContextInfo (Display* pDisplay, XVisualInfo* pXVisualInfo);

  XVisualInfo* GetXVisualInfo (Display* pDisplay, int Screen) const;
#endif // _X11_

  friend class nglContext;
};


#define NGL_CONTEXT_ENONE  0 ///< No error


//! OpenGL context management
/*!
This is an abstract class which stores per GL context informations for a
rendering surface such as nglWindow. This is also the place from where GL
extensions are reachable. You can query and initialize an extension with
checkExtension() and use extensions functions as if they were methods of
this context (they are really pointers to functions which are initially
NULL).

Developer's note : the extension support code is generated by a Perl script
which parses <em>glext.h</em>, see <em>src/core/glext</em> in the source
distribution for more info.
*/
class NGL_API nglContext : public nglError
{
friend class nglOffscreenContext;
public:
  /** @name GL extensions */
  //@{
  bool CheckExtension(const nglChar* pExtName);
  /*!< Check presence of a GL extension and initialize it if found
    \param pExtName GL extension name
    \return true if extension present and ready

    This method checks if a GL extension is supported (by both client and
    server sides) and prepares it for use. Example :

\code
if (HasExtension(_T("GL_ARB_texture_compression")))
{
  GLint value;
  glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &value);
  NGL_OUT(_T("%d texture compression formats supported"), value);
}
\endcode

    Note that you need an up-to-date <GL/glext.h> file from your OpenGL
    implementation.
    For the full extension list and documentation, see the centralized
    registry at http://oss.sgi.com/projects/ogl-sample/registry/
  */
  //@}

#ifndef _OPENGL_ES_
  // Include extension methods as members here
  #include "ngl_glext.h"
#endif

  /** @name  */
  //@{
  bool GetContextInfo(nglContextInfo& rInfo) const;  ///< Retrieve context description
  void Dump(uint Level) const;
  //@}

  virtual void BeginSession() = 0;
  virtual bool MakeCurrent() const = 0;
  virtual void EndSession() = 0;

  nglTargetAPI mTargetAPI;

  nuiPainter* GetPainter() const;
protected:
  /** @name Life cycle */
  //@{
  nglContext();  ///< Build an unitialized context
  virtual ~nglContext();
  void InitPainter();
  //@}

  virtual const nglChar* OnError (uint& rError) const;

  bool mValidBackBufferRequestedNotGranted;
  nuiPainter* mpPainter;
private:
  typedef void (*GLExtFunc)(void);

  nglContext(const nglContext&) {} // Undefined copy constructor

  bool      InitExtension (const nglChar* pExtName);
  GLExtFunc LookupExtFunc (const char* pFuncName);

  //nglTargetAPI mTargetAPI;

#ifdef _WIN32_
public:
  LPDIRECT3DDEVICE9 GetDirect3DDevice();
protected:
  HDC    mDC;
  HGLRC  mRC;
  LPDIRECT3DDEVICE9 mpDirect3DDevice;

  virtual bool Build(HWND hWnd, const nglContextInfo& rInfo, const nglContext* pShared);
  virtual bool BuildOpenGL(HWND hWnd, const nglContextInfo& rInfo, const nglContext* pShared);
  virtual bool BuildDirect3D(HWND hWnd, const nglContextInfo& rInfo, const nglContext* pShared);
  virtual bool BuildNone(HWND hWnd, const nglContextInfo& rInfo, const nglContext* pShared);
  bool ShareWith(nglContext* pShare);

  bool BuildOpenGLFromExisting(HWND hWnd, HGLRC rc);
private:
  HWND   mCtxWnd;
#endif // _WIN32_

protected:

#ifdef _X11_
  Display*     mpDisplay;
  XVisualInfo* mpXVisualInfo;
  Visual*      mpXVisual;
  GLXContext   mGlxCtx;
  int          mDepth;
  int          mGlxErrorBase, mGlxEventBase;

  bool Build(int Screen, const nglContextInfo& rInfo, const nglContext* pShared);
  bool MakeCurrent(Window Win) const;
  bool BuildOpenGLFromExisting(GLXContext ctx);
#endif // _X11_

#ifdef _CARBON_
  AGLContext     mCtx;
  bool mFullscreen; ///< AGL is a Mac API so as any Apple done API it's full of shit.
  bool Build(WindowRef Win, const nglContextInfo& rInfo, const nglContext* pShared, bool Fullscreen);
  bool BuildOpenGLFromExisting(WindowRef Win, AGLContext Ctx);
  bool Destroy();
  bool MakeCurrent(WindowRef Win) const;
    
  CFBundleRef mpBundleRefOpenGL;
  OSStatus aglInitEntryPoints (void);
  void aglDellocEntryPoints (void);
  void* aglGetProcAddress (const char * pszProc);
  nglContextInfo mContextInfo;
#endif

#ifdef _UIKIT_
  void Build(const nglContextInfo& rInfo);

  nglContextInfo mContextInfo;

  bool mFullscreen;

  friend class nglContextInfo;
#endif

#ifdef _COCOA_
  nglContextInfo mContextInfo;
  void Build(const nglContextInfo& rInfo);
#endif
};

// Add some not yet officially defined values:
#ifndef GL_DEPTH_STENCIL_EXT
  #define GL_DEPTH_STENCIL_EXT                              0x84F9
  #define GL_UNSIGNED_INT_24_8_EXT                          0x84FA
  #define GL_DEPTH24_STENCIL8_EXT                           0x88F0
  #define GL_TEXTURE_STENCIL_SIZE_EXT                       0x88F1
#endif


#endif // !_NOGFX_
#endif // __nglContext_h__
