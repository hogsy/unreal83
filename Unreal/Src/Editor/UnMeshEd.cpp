/*=============================================================================
	UnMeshEd.cpp: Unreal editor mesh code

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Note: See DRAGON.MAC for a sample import macro

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "Unreal.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	Data types for importing James' creature meshes
-----------------------------------------------------------------------------*/

//
// Mesh and vertex summary
//

class FJSDataHeader
	{
	public:
	WORD	NumPolys;
	WORD	NumVertices;
	WORD	BogusRot;
	WORD	BogusFrame;
	DWORD	BogusNormX,BogusNormY,BogusNormZ;
	DWORD	FixScale;
	DWORD	Unused1,Unused2,Unused3;
	};

class FJSAnivHeader
	{
	public:
	WORD	NumFrames;		// Number of animation frames
	WORD	FrameSize;		// Size of one frame of animation
	};

/*
//
// James' notes:
//
Here is the format for the 3d animation:
data**.3d
HEADER
  word #polygons, word #verticies
  word rotation about direction vector (0-3600)
  word current animation #
  dword 0,0,0  direction normal (16bit fixed int)
  dword 10000h,0,0,0    Scale(fixed int), unused,unused,unused
 
(note:  the directional vector and rotation about it was for compatibility
with 3d studio.  I will change this in the future)
 
MAIN DATA (poly info list)
  dw 0,1,2   ;offset in vertex list of 3 verticies of current frame
             ; of animation
  db 0       ;poly type 0=textured, 1=flat, 2 translucent, 3=0masked
                        (only textured and masked used)
  db 0       ;polygon color (only for flat and gouraud poly)
  db 1,1, 2,2, 3,3   ;texture map coords
  db 0               ;#of 256x256 source texture
  db 0               ;unused
 
at end of poly list is a bunch of bytes for each vertex indicating the
gouraud value for that vertex.  (this list may or may not exist on the
current meshes cuz I haven't used gouraud much yet)
 
VERTEX DATA
word number of frames of animation
word side of a frame animation
 
each vertex is stored in 4bytes
x=first 11 bits
y=next 11 bits
z=last 10 bits
*/


#define UPDATE_PACKED_MIN(min,v)\
	{\
	if (((FLOAT)(v)->X) < ((min)->X)) (min)->X = (FLOAT)(v)->X;\
	if (((FLOAT)(v)->Y) < ((min)->Y)) (min)->Y = (FLOAT)(v)->Y;\
	if (((FLOAT)(v)->Z) < ((min)->Z)) (min)->Z = (FLOAT)(v)->Z;\
	};
#define UPDATE_PACKED_MAX(max,v)\
	{\
	if (((FLOAT)(v)->X) > ((max)->X)) (max)->X = (FLOAT)(v)->X;\
	if (((FLOAT)(v)->Y) > ((max)->Y)) (max)->Y = (FLOAT)(v)->Y;\
	if (((FLOAT)(v)->Z) > ((max)->Z)) (max)->Z = (FLOAT)(v)->Z;\
	};

//
// Import a mesh from James' editor.  Uses file commands instead of resource
// manager.  Slow but works fine.
//
void meshImport (const char *MeshName, const char *AnivFname, const char *DataFname)
	{
	UMesh			*Mesh;
	FMeshVertex		*TempVertex;
	FBoundingVolume	*MeshBound;
	FMeshTriangle	*MeshTriangle;
	FILE			*AnivFile,*DataFile;
	BYTE			*MeshData;
	IMesh			MeshInfo;
	FJSDataHeader	JSDataHdr;			// James' DATA*.3D file header
	FJSAnivHeader	JSAnivHdr;			// James' ANIV*.3D file header
	int				i,j,k;
	int				Ok = 0;
	//
	GUARD;
	debugf(LOG_Info,"Importing %s",MeshName);
	GApp->BeginSlowTask ("Importing mesh",1,0);
	GApp->StatusUpdate  ("Reading files",0,0);
	//
	// Open James' animation vertex file and read header:
	//
	AnivFile = fopen (AnivFname,"r+b");
	if (AnivFile==NULL) {debugf (LOG_Info,"Error opening %s",AnivFname); goto Out1;};
	if (fread (&JSAnivHdr,sizeof(FJSAnivHeader),1,AnivFile)!=1) {debugf (LOG_Info,"Error reading %s",AnivFname); goto Out2;};
	//
	// Open James' mesh data file and read header:
	//
	DataFile = fopen (DataFname,"r+b");
	if (DataFile==NULL) {debugf (LOG_Info,"Error opening %s",DataFile); goto Out2;};
	if (fread (&JSDataHdr,sizeof(FJSDataHeader),1,DataFile)!=1) {debugf (LOG_Info,"Error reading %s",DataFile); goto Out3;};
	//
	//	Allocate mesh resource and set its header:
	//
	Mesh = new(MeshName,CREATE_Replace)UMesh;
	//
	Mesh->MaxTriangles	= JSDataHdr.NumPolys;
	Mesh->MaxVertices	= JSDataHdr.NumVertices;
	Mesh->MaxVertLinks	= JSDataHdr.NumVertices * 6; // Overestimate
	Mesh->MaxAnimFrames	= JSAnivHdr.NumFrames;
	Mesh->MaxAnimSeqs		= 96; // Predefined limit of animation sequences (reasonable)
	//
	debugf (LOG_Info," * Triangles  %i",Mesh->MaxTriangles);
	debugf (LOG_Info," * Vertex     %i",Mesh->MaxVertices);
	debugf (LOG_Info," * AnimFrames %i",Mesh->MaxAnimFrames);
	debugf (LOG_Info," * FrameSize  %i",JSAnivHdr.FrameSize);
	debugf (LOG_Info," * AnimSeqs   %i",Mesh->MaxAnimSeqs);
	//
	// Allocate mesh data and distribute it:
	//
	MeshData = (BYTE *)Mesh->AllocData(1);
	//
	Mesh->NumTriangles  = Mesh->MaxTriangles;   // Just imported, never manipulated
	Mesh->NumVertices   = Mesh->MaxVertices;
	Mesh->NumVertLinks  = 0;
	Mesh->NumAnimFrames = Mesh->MaxAnimFrames;
	Mesh->NumAnimSeqs 	= 0;
	Mesh->Origin		= GMath.ZeroVector;
	Mesh->RotOrigin		= GMath.ZeroRotation;
	//
	Mesh->GetInfo(&MeshInfo);
	//
	// Import mesh triangles:
	//
	debugf (LOG_Info,"Importing triangles");
	MeshTriangle = &MeshInfo.Triangles [0];
	fseek (DataFile,12,SEEK_CUR); // Skip empty stuff
	//
	for (i=0; i<Mesh->NumTriangles; i++)
		{
		if (!(i&15)) GApp->StatusUpdate  ("Importing Triangles",i,Mesh->NumTriangles);
		if (fread (MeshTriangle,sizeof(FMeshTriangle),1,DataFile)!=1) {debugf (LOG_Info,"Error processing %s",DataFile); goto Out4;};
		//
		MeshTriangle->Flags = 0;
		MeshTriangle++;
		};
	//
	// Import mesh vertices:
	//
	debugf (LOG_Info,"Importing vertices");
	TempVertex = &MeshInfo.Vertex[0];
	for (i=0; i<Mesh->NumAnimFrames; i++)
		{
		if (!(i&3)) GApp->StatusUpdate  ("Importing Vertices",i,Mesh->NumAnimFrames);
		if (fread (TempVertex,sizeof(FMeshVertex),Mesh->NumVertices,AnivFile)!=(size_t)Mesh->NumVertices)
			{
			debugf (LOG_Info,"Vertex error in %s",AnivFname);
			goto Out4;
			};
		fseek (AnivFile,JSAnivHdr.FrameSize - Mesh->NumVertices*sizeof(FMeshVertex),SEEK_CUR); // Skip empty stuff
		TempVertex += Mesh->NumVertices;
		};
	//
	// Compute per-frame bounding volumes plus overall bounding volume:
	//
	Mesh->Bound.Min		= GMath.VectorMax;
	Mesh->Bound.Max		= GMath.VectorMin;
	//
	TempVertex 	        = &MeshInfo.Vertex [0];
	MeshBound           = &MeshInfo.Bound  [0];
	//
	for (i=0; i<Mesh->NumAnimFrames; i++)
		{
		if (i&1) GApp->StatusUpdate("Bounding mesh",i,Mesh->NumVertices);
		//
		MeshBound->Min = GMath.VectorMax;
		MeshBound->Max = GMath.VectorMin;
		//		
		for (j=0; j<Mesh->NumVertices; j++)
			{
			UPDATE_PACKED_MIN(&MeshBound->Min,TempVertex);
			UPDATE_PACKED_MAX(&MeshBound->Max,TempVertex);
			TempVertex++;
			};
		UPDATE_PACKED_MIN(&Mesh->Bound.Min,&MeshBound->Min);
		UPDATE_PACKED_MAX(&Mesh->Bound.Max,&MeshBound->Max);
		//
		MeshBound++;
		};
	debugf (LOG_Info,"Bound %f,%f %f,%f %f,%f",Mesh->Bound.Min.X,Mesh->Bound.Max.X,Mesh->Bound.Max.Y,Mesh->Bound.Max.Y,Mesh->Bound.Min.Z,Mesh->Bound.Max.Z);
	//
	// Build reverse vertex-triangle links:
	//
	Mesh->NumVertLinks = 0;
	for (i=0; i<Mesh->NumVertices; i++)
		{
		if (i&1) GApp->StatusUpdate("Linking mesh",i,Mesh->NumVertices);
		MeshInfo.VertLinkIndex[i].NumVertTriangles   = 0;
		MeshInfo.VertLinkIndex[i].TriangleListOffset = Mesh->NumVertLinks;
		for (j=0; j<Mesh->NumTriangles; j++)
			{
			for (k=0; k<3; k++)
				{
				if (MeshInfo.Triangles[j].iVertex[k] == i)
					{
					MeshInfo.VertLinks[Mesh->NumVertLinks++] = (WORD)j;
					MeshInfo.VertLinkIndex[i].NumVertTriangles++;
					};
				};
			};
		};	
	debugf (LOG_Info,"Made %i links",Mesh->NumVertLinks);
	//
	// Exit labels:
	//
	Ok = 1;
	Out4: if (!Ok) Mesh->Kill();
	Out3: fclose  (DataFile);
	Out2: fclose  (AnivFile);
	Out1: GApp->EndSlowTask ();
	UNGUARD("meshImport");
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
