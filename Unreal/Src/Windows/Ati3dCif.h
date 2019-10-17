//
//  Filename: ati3dcif.h 
//
//  Description: ATI 'C'interface (CIF) header to GT 3D hardware acceleration
//
//  Trade secret of ATI Technologies, Inc.
//  Copyright 1995, ATI Technologies, Inc., (unpublished) 
//  
//  All rights reserved.  This notice is intended as a precaution against
//  inadvertent publication and does not imply publication or any waiver
//  of confidentiality.  The year included in the foregoing notice is the
//  year of creation of the work.
//  
//  

#ifndef ATI_ATI3DCIF_H
#define ATI_ATI3DCIF_H


// Current Restrictions
//          1. Only support EV_VTCF floating point vertex type
//          2. Only one context may be exist at a time
//          3. RenderSwitch does not have meaning ( consequence of 2. ) 
//          4. C3D_ETFILT_MIPTRI_MAG2BY2 is not supported yet.
//          5. Coordinates should be pre-clipped by software to:
//              -2048.0f +2047.0f in X
//              -4096.0f +4095.0f in Y

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ATI_INTERNAL
#define DLLEXPORT 
#endif

#ifdef  DOS_BUILD
#include "afxdos.h"
#include "i3debug.h"
#endif

#ifndef  DOS_BUILD
#pragma pack(push)  // push packing state
#pragma pack(8)     // CIF requires structure data is naturally alligned
#endif

// Fundemental types
//
typedef unsigned int    C3D_BOOL;       // 0 indicates FALSE, >0 indicates TRUE
typedef unsigned int    C3D_INT32;      // 32 bit integer data
typedef unsigned int    C3D_UINT32;     // unsigned 32 bit integer data
typedef unsigned short  C3D_UINT16;     // unsigned 16 bit integer data
typedef unsigned char   C3D_UINT8;      // unsigned 8  bit integer data
typedef float           C3D_FLOAT32;    // 32 bit floating point data

typedef unsigned int*   C3D_PBOOL;      // points at C3D_BOOL;   
typedef unsigned int*   C3D_PINT32;     // points at C3D_INT32;
typedef unsigned int*   C3D_PUINT32;    // points at C3D_UINT32;
typedef unsigned short* C3D_PUINT16;    // points at C3D_UINT16;
typedef unsigned char*  C3D_PUINT8;     // points at C3D_UINT8;
typedef float*          C3D_PFLOAT32;   // points at C3D_FLOAT32;
typedef void *          C3D_PVOID;      // points at generic data

// Error codes
//
typedef enum {
    C3D_EC_OK           = 0,  // success
    C3D_EC_GENFAIL      = 1,  // Generic Failure
    C3D_EC_MEMALLOCFAIL = 2,  // Memory allocation failure
    C3D_EC_BADPARAM     = 3,  // Invalid parameter passed to function
    C3D_EC_UNUSED0      = 4,  // NOT USED
    C3D_EC_BADSTATE     = 5,  // Object entered invalid state
    C3D_EC_NOTIMPYET    = 6,  // Functionality Not Implemented Yet
    C3D_EC_UNUSED1      = 7,  // NOT USED
    C3D_EC_NUM          = 8
} C3D_EC, * C3D_PEC;

// Rect: specs a rectangular region in 2 space
//
typedef struct {
    C3D_INT32 top;
    C3D_INT32 left;
    C3D_INT32 bottom;
    C3D_INT32 right;
} C3D_RECT , * C3D_PRECT;

// Color
//
typedef union {
    struct {
        unsigned r: 8;  // 8 red bits
        unsigned g: 8;  // 8 green bits
        unsigned b: 8;  // 8 blue bits
        unsigned a: 8;  // 8 alpha bits
    };
    C3D_UINT32 u32All;
} C3D_COLOR , * C3D_PCOLOR;

// Module info structure
//
typedef struct {
    C3D_UINT32 u32Size;             // sz of struct must be init'ed by client
    C3D_UINT32 u32FrameBuffBase;    // Host pointer to frame buffer base
    C3D_UINT32 u32OffScreenHeap;    // Host pointer to offscreen heap
    C3D_UINT32 u32OffScreenSize;    // size of offscreen heap
    C3D_UINT32 u32TotalRAM;         // total amount of RAM on the card
    C3D_UINT32 u32ASICID;           // ASIC Id. code
    C3D_UINT32 u32ASICRevision;     // ASIC revision
} C3D_3DCIFINFO, * PC3D_3DCIFINFO;

//
// Vertex enumeration type : used to select vertex data format to be referenced 
//                           when rendering primitives
//  
// Nomenclature:             V for Vertex (Always X,Y,Z)
//                           C for Color  (Always R,G,B,A)
//                           T for Texture(Always S,T,W)
// 

typedef enum {
    C3D_EV_VF       = 0,
    C3D_EV_VCF      = 1,
    C3D_EV_VTF      = 2,
    C3D_EV_VTCF     = 3,
    C3D_EV_NUM      = 4
} C3D_EVERTEX, * C3D_PEVERTEX;

//
// Vertex data types
//
typedef struct {                        
    C3D_FLOAT32 x, y, z;                // FLOATING point type
} C3D_VF, * C3D_PVF;                    // identified by C3D_EV_VF

typedef struct { 
    C3D_FLOAT32 x, y, z;                // FLOATING point type
    C3D_FLOAT32 r, g, b, a;             // identified by C3D_EV_VCF
} C3D_VCF, * C3D_PVCF;

typedef struct { 
    C3D_FLOAT32 x, y, z;                // FLOATING point type
    C3D_FLOAT32 s, t, w;                // identified by C3D_EV_VTF
} C3D_VTF, * C3D_PVTF;

typedef struct { 
    C3D_FLOAT32 x, y, z;                // FLOATING point type
    C3D_FLOAT32 s, t, w;                // identified by C3D_EV_VTCF
    C3D_FLOAT32 r, g, b, a; 
} C3D_VTCF, * C3D_PVTCF;


// Primitives Sets:  A primitive set is a sequence of vertex data used
// to represent a stream of geometric primitives:  Quads,Tris or Lines.  
// A Primitive Set can take two basic forms: Strips or Lists.  
// A Primitive List is a doubly indirect array of pointers to actual vertex data.  
// A Primitive Strip is  a pointer to contiguous array of vertex data 
// where adjacent primives share vertex data.
//
// (e.g. a triangle strip with four vertices represents 2 triangles 
//

// Primitive Set Pointers
typedef void *  C3D_VSTRIP;
typedef void ** C3D_VLIST;

// Primitive Type:  Specifies the geometric interpretation of a vertex set
// during rasterization. eg: if C3D_EPRIM_TRI is set in a rendering context
// then subsequent calls to ATI3DCIF_RenderPrimList will interpret the specified
// verticies as triangles and will consume 3 vertices for each triangle drawn.
//
typedef enum {
    C3D_EPRIM_LINE  = 0,
    C3D_EPRIM_TRI   = 1,
    C3D_EPRIM_QUAD  = 2,
    C3D_EPRIM_NUM   = 3     // invalid enumeration val
} C3D_EPRIM, * C3D_PEPRIM;

// Shading: 
//
typedef enum {
    C3D_ESH_NONE     = 0,
    C3D_ESH_SOLID    = 1,   // shade using the clrSolid from the RC
    C3D_ESH_FLAT     = 2,   // shade using the last vertex to flat shade.
    C3D_ESH_SMOOTH   = 3,   // shade using linearly interpolating vert clr
    C3D_ESH_NUM      = 4    // invalid enumeration val
} C3D_ESHADE, * C3D_PESHADE;

// Alpha Blending
//
//      clr_dst = clr_src * f(eAlphaSrcFactor) + clr_dst * f(eAlphaDstFactor)

// alpha blending source factor select
typedef enum {
    C3D_EASRC_ZERO              = 0,// Blend factor is (0, 0, 0)
    C3D_EASRC_ONE               = 1,// Blend factor is (1, 1, 1)
    C3D_EASRC_DSTCLR            = 2,// Blend factor is (Rd, Gd, Bd)
    C3D_EASRC_INVDSTCLR         = 3,// Blend factor is (1-Rd, 1-Gd, 1-Bd)
    C3D_EASRC_SRCALPHA          = 4,// Blend factor is (As, As, As)
    C3D_EASRC_INVSRCALPHA       = 5,// Blend factor is (1-As, 1-As, 1-As)
    C3D_EASRC_NUM               = 6 // invalid enumeration val
 } C3D_EASRC, *C3D_PEASRC;

// alpha blending destination factor select
//
typedef enum {
    C3D_EADST_ZERO          = 0,    // Blend factor is (0, 0, 0)
    C3D_EADST_ONE           = 1,    // Blend factor is (1, 1, 1)
    C3D_EADST_SRCCLR        = 2,    // Blend factor is (Rs, Gs, Bs)
    C3D_EADST_INVSRCCLR     = 3,    // Blend factor is (1-Rs, 1-Gs, 1-Bs)
    C3D_EADST_SRCALPHA      = 4,    // Blend factor is (As, As, As)
    C3D_EADST_INVSRCALPHA   = 5,    // Blend factor is (1-As, 1-As, 1-As)
    C3D_EADST_NUM           = 6     // invalid enumeration val
} C3D_EADST, *C3D_PEADST;

// Texture mapping
//
typedef void * C3D_HTX;             // handle to a texture in the frambuffer
typedef C3D_HTX * C3D_PHTX;         // pointer to handle to texture

// Texel lighting
typedef enum {
    C3D_ETL_NONE        = 0,        //  TEXout = Tclr
    C3D_ETL_MODULATE    = 1,        //  TEXout = Tclr*CInt
    C3D_ETL_ALPHA_DECAL = 2,        //  TEXout = (Tclr*Talp)+(CInt*(1-Talp))
    C3D_ETL_NUM         = 3         //  invalid enumeration
} C3D_ETLIGHT, * C3D_PETLIGHT;

// specifies the level of perspective correction done by the hardware.  This
// setting trades off image quality for better frame rates 
typedef enum {
    C3D_ETPC_NONE       = 0,        // no correction , best frame rate
    C3D_ETPC_ONE        = 1,
    C3D_ETPC_TWO        = 2,
    C3D_ETPC_THREE      = 3,        // recommended correction
    C3D_ETPC_FOUR       = 4,
    C3D_ETPC_FIVE       = 5,
    C3D_ETPC_SIX        = 6,        // full correction, worst frame rate
    C3D_ETPC_NUM        = 7
} C3D_ETPERSPCOR, * C3D_PETPERSPCOR;

// texture filtering modes
// ( NB: that GT supports 2x2 MIN only if 2x2 MAG is turned on )
// ( NB: modes identified by MIP are only valid with mip mapped textures )
typedef enum {
    C3D_ETFILT_MINPNT_MAGPNT    = 0,// pick nearest texel (pnt) min/mag
    C3D_ETFILT_MINPNT_MAG2BY2   = 1,// pnt min/bi-linear mag
    C3D_ETFILT_MIN2BY2_MAG2BY2  = 2,// 2x2 blend min/bi-linear mag
    C3D_ETFILT_MIPLIN_MAGPNT    = 3,// 1x1 blend min(between maps)/pnt mag
    C3D_ETFILT_MIPLIN_MAG2BY2   = 4,// 1x1 blend min(between maps)/bi-linear mag
    C3D_ETFILT_MIPTRI_MAG2BY2   = 5,// (2x2)x(2x2)(between maps)/bi-linear mag
    C3D_ETFILT_NUM              = 6 //
} C3D_ETEXFILTER, *C3D_PETEXFILTER;

// texel op modes: determines how which operations are applied to texels as 
//      they are read from the texture.  By design theses are orthogonal to 
//      filtering modes. ( not available for use with filtering on the GT-A2)
//
typedef enum {
    C3D_ETEXOP_NONE          = 0,  // 
    C3D_ETEXOP_CHROMAKEY     = 1,  // select texels not equal to the chroma key
    C3D_ETEXOP_ALPHA         = 2,  // pass texel alpha to the alpha blender
    C3D_ETEXOP_ALPHA_MASK    = 3,  // lw bit 0: tex not drawn otw: alpha int
    C3D_ETEXOP_NUM           = 4   //
} C3D_ETEXOP, * C3D_PETEXOP;

// specifies the pixel format
typedef enum {                      //                                     (sz)
    C3D_EPF_RGB1555 = 3,            // 1b Alpha, 5b Red, 5b Green, 5b Blue (16)
    C3D_EPF_RGB565  = 4,            // 0b Alpha, 5b Red, 6b Green, 5b Blue (16) 
    C3D_EPF_RGB8888 = 6,            // 8b Alpha, 8b Red, 8b Green, 8b Blue (32) 
    C3D_EPF_RGB332  = 7,            // 0b Alpha, 3b Red, 3b Green, 2b Blue (08) 
    C3D_EPF_Y8      = 8,            // 8b Y                                (08) 
    C3D_EPF_YUV422  = 11,           // YUV 422 Packed (YUYV) MS FOURCC_UYVY(16)
} C3D_EPIXFMT, * C3D_PEPIXFMT;

// specifies the texel format
typedef enum {                      //                                     (sz)
    C3D_ETF_RGB1555 = 3,            // 1b Alpha, 5b Red, 5b Green, 5b Blue (16)
    C3D_ETF_RGB565  = 4,            // 0b Alpha, 5b Red, 6b Green, 5b Blue (16) 
    C3D_ETF_RGB8888 = 6,            // 8b Alpha, 8b Red, 8b Green, 8b Blue (32) 
    C3D_ETF_RGB332  = 7,            // 0b Alpha, 3b Red, 3b Green, 2b Blue (08)     
    C3D_ETF_Y8      = 8,            // 8b Y                                (08) 
    C3D_ETF_YUV422  = 11,           // YUV 422 Packed (YUYV) MS FOURCC_UYVY(16)
    C3D_ETF_RGB4444 = 15            // 4b Alpha, 4b Red, 4b Green, 4b Blue (16) 
} C3D_ETEXFMT, * C3D_PETEXFMT;


// the TMAP structure specifies how the hardare should interpret a texture 
// stored in the frambuffer.
//
// apv32Offsets is an array of host pointers to the individual maps which 
// compose a texture. Where if (bMipMap==TRUE) apv32Offsets contains one or 
// more elements the first (index 0) points at the base map and subsequent 
// elements point at the sequentially smaller maps.  For the case of 
// bMipMap == FALSE the array contains one valid element(the first) 
// and the rest are ignored.
//
#define cu32MAX_TMAP_LEV 11

typedef struct {
    C3D_UINT32      u32Size;                    // size of structure
    C3D_BOOL        bMipMap;                    // is texture a mip map
    C3D_PVOID       apvLevels[cu32MAX_TMAP_LEV];// array of pointer to map level
    C3D_UINT32      u32MaxMapXSizeLg2;          // log 2 X size of largest map
    C3D_UINT32      u32MaxMapYSizeLg2;          // log 2 Y size of largest map
    C3D_ETEXFMT     eTexFormat;                 // texel format
    C3D_COLOR       clrTexChromaKey;            // specify texel transp. clr
} C3D_TMAP, * C3D_PTMAP;

//
// Rendering Context

typedef void* C3D_HRC;
typedef void* C3D_PRSDATA;

// Rendering context field enumerations 
//                                  // DATA TYPE       DEFAULT VALUE
typedef enum {                  
    C3D_ERS_BG_CLR          = 0,    // C3D_COLOR       {0,0,0,0,0}
    C3D_ERS_VERTEX_TYPE     = 1,    // C3D_EVERTEX     C3D_EV_VTCF
    C3D_ERS_PRIM_TYPE       = 2,    // C3D_EPRIM       C3D_EPRIM_TRI
    C3D_ERS_SOLID_CLR       = 3,    // C3D_COLOR       {0,0,0,0,0}
    C3D_ERS_SHADE_MODE      = 4,    // C3D_ESHADE      C3D_ESH_SMOOTH
    C3D_ERS_TMAP_EN         = 5,    // C3D_BOOL        FALSE
    C3D_ERS_TMAP_SELECT     = 6,    // C3D_HTX         NULL
    C3D_ERS_TMAP_LIGHT      = 7,    // C3D_ETLIGHT     C3D_ETL_NONE
    C3D_ERS_TMAP_PERSP_COR  = 8,    // C3D_ETPERSPCOR  C3D_ETPC_THREE
    C3D_ERS_TMAP_FILTER     = 9,    // C3D_ETEXFILTER  C3D_ETFILT_MINPNT_MAG2BY2
    C3D_ERS_TMAP_TEXOP      = 10,   // C3D_ETEXOP      C3D_ETEXOP_NONE
    C3D_ERS_ALPHA_SRC       = 11,   // C3D_EASRC       C3D_EASRC_ONE
    C3D_ERS_ALPHA_DST       = 12,   // C3D_EADST       C3D_EADST_ZERO
    C3D_ERS_SURF_DRAW_PTR   = 13,   // C3D_PVOID       Desktop Surface
    C3D_ERS_SURF_DRAW_PITCH = 14,   // C3D_UINT32      Desktop Surface
    C3D_ERS_SURF_DRAW_PF    = 15,   // C3D_EPIXFMT     Desktop Surface
    C3D_ERS_SURF_VPORT      = 16,   // C3D_RECT        Desktop Surface
    C3D_ERS_NUM             = 17    // invalid enumeration
} C3D_ERSID, * C3D_PERSID;

// 
// Inlines
//
#define SET_CIF_COLOR(exp, red, grn, blu, alp)           \
    (exp).r             = (C3D_UINT8)(red);              \
    (exp).g             = (C3D_UINT8)(grn);              \
    (exp).b             = (C3D_UINT8)(blu);              \
    (exp).a             = (C3D_UINT8)(alp);

//
// CIF3D Module Functions
//

// initializes the ATI3DCIF driver module
//
C3D_EC DLLEXPORT WINAPI ATI3DCIF_Init( void );
#ifdef  DOS_BUILD
C3D_EC DLLEXPORT WINAPI ATI3DCIF_DosInit( PVOID pPHXINFO );
#endif

// terminates the ATI3DCIF driver module
//
C3D_EC DLLEXPORT WINAPI ATI3DCIF_Term(  void  );

// returns information about the 3DCIF module
//         
C3D_EC DLLEXPORT WINAPI ATI3DCIF_GetInfo( PC3D_3DCIFINFO p3DCIFInfo );


//
// Texture Management Functions
//

// registers a texture with the CIF module.  
//
C3D_EC
DLLEXPORT WINAPI
ATI3DCIF_TextureReg( 
    C3D_PTMAP ptmapToReg,       // client specified info on texture to be reg'ed
    C3D_PHTX  phtmap            // returned handle to the registered texture 
    );

// Unregisters a texture with the CIF module.  
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI
ATI3DCIF_TextureUnreg( 
    C3D_HTX htxToUnreg          // specifies texture to unregister
    );


//
// Context Functions
//

// Creates a ATI3DCIF rendering context initialized to the default state
// specified above
C3D_HRC                         // returns handle to RC or NULL on failure
DLLEXPORT WINAPI 
ATI3DCIF_ContextCreate( void );

// Destroys a ATI3DCIF rendering context. 
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI
ATI3DCIF_ContextDestroy(
    C3D_HRC     hRC             //  handle to a created rendering context
    );

// Set State in a ATI3DCIF rendering context  
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI
ATI3DCIF_ContextSetState(
    C3D_HRC     hRC,            // handle to a created rendering context
    C3D_ERSID   eRStateID,      // id of state to set
    C3D_PRSDATA pRStateData     // data to set
    );


//
// Rendering Functions
//

// Prepares hardware to draw using the context refered to by the specified 
// handle.  RenderBegin must be called prior to any other Render* functions.
// RenderBegin will(may?) fail if the Primary Surface has been locked through 
// DirectDraw.(?)  Typically called prior to rendering each frame
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI
ATI3DCIF_RenderBegin(
    C3D_HRC     hRC             // handle to a rendering context
    );

// Ends 3D hardware drawing operations.  Frees up the graphics hardware for 2D
// operations. 
// Typically called after rendering each frame
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI ATI3DCIF_RenderEnd( void );

// Switches to a new rendering context.  ATI3DCIF_RenderSwitch(hrc) is a 
// lighter weight, functional equivalent to the the combination of 
// ATI3DCIF_RenderEnd() followed by ATI3DCIF_RenderBegin(hrc)
// Only valid while in the 3D rendering state.
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI
ATI3DCIF_RenderSwitch(
    C3D_HRC     hRC             // handle to a rendering context to switch into
    );

// RenderPrimStrip in the rendering context specified with the render begin call
// Only valid while in the 3D rendering state.
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI 
ATI3DCIF_RenderPrimStrip(
    C3D_VSTRIP  vStrip,         // strip to draw
    C3D_UINT32  u32NumVert      // number of verticies in the list
    );

// RenderPrimList in the rendering context specified with the render begin call
// Only valid while in the 3D rendering state.
//
C3D_EC                          // 3DCIF error code, C3D_EC_OK on success
DLLEXPORT WINAPI
ATI3DCIF_RenderPrimList(
    C3D_VLIST   vList,          // List to draw
    C3D_UINT32  u32NumVert      // number of verticies in the strip
    );

#ifndef  DOS_BUILD
#pragma pack(pop)               // restore packing state
#endif

#ifdef __cplusplus
}
#endif


#endif // ATI_ATI3DCIF_H

