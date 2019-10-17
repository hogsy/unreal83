/*=============================================================================
   UnFile.cpp: Various file-management functions.

	Copyright 1996 Epic MegaGames, Inc. This software is a trade secret.
	Compiled with Visual C++ 4.0. Best viewed with Tabs=4.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

#include "UnPort.h"
#include "UnFile.h"

#ifdef __MSVC__
#include <direct.h>
#endif

/*-----------------------------------------------------------------------------
   fsize: Size of a file.  Returns -1 if doesn't exist.
   This is a good function to tell you whether a file exists.
-----------------------------------------------------------------------------*/

long fsize (const char *fname)
	{
	FILE *f;
	long result;
	//
	f=fopen(fname,"rb");
	if (f==NULL) return -1;
	//
	if (fseek(f,0,SEEK_END)!=0)
		{
		fclose (f);
		return -1;
		};
	result=ftell(f);
	fclose (f);
	return result;
	};

/*-----------------------------------------------------------------------------
   fdelete: Deletes a file
   Returns: 0 if success, nonzero if error.
-----------------------------------------------------------------------------*/

int fdelete (const char *fname)
	{
	return (unlink(fname));
	};

/*-----------------------------------------------------------------------------
   frename: Renames a file
   Returns: 0 if success, nonzero if error.
-----------------------------------------------------------------------------*/

int frename (const char *oldname, const char *newname)
	{
	return (rename(oldname,newname));
	};

/*-----------------------------------------------------------------------------
   fgetdir: Gets the name of the current working directory
   example: "C:\UNREAL\"
-----------------------------------------------------------------------------*/

char *fgetdir(char *dir)
	{
	_getcwd(dir,256);
	strcat (dir,"\\");
	return dir;
	};

/*-----------------------------------------------------------------------------
   fsetdir: Sets the current working drive and/or directory.
   Returns 0 if ok, nonzero if error.
   This will accept directories like either "C:\UNREAL\" or "C:\UNREAL".
-----------------------------------------------------------------------------*/

int fsetdir(const char *dir)
	{
	char temp[256];
	char *newdir;
	int i;
	//
	strcpy(temp,dir); newdir=temp;
	i=strlen(newdir)-1;
	if (i>0) if (newdir[i]=='\\') newdir[i]='\0';
	if (*newdir) {
		if (newdir[1]==':') {
			#ifdef __WATCOMC__
			_dos_setdrive (newdir[0]-'A',&total_drives);
			#endif
			#ifdef __BORLANDC__
			setdisk (newdir[0]-'A');
			#endif
			newdir+=2;
			};
		}
	if (!*newdir) return 0;
	return (_chdir (newdir));
	};

/*-----------------------------------------------------------------------------
   fmkdir: Makes a directory
   Returns 0 if ok, nonzero if error.
-----------------------------------------------------------------------------*/

int fmkdir (const char *dir)
	{
	return _mkdir(dir);
	};

/*-----------------------------------------------------------------------------
   fext: Gets the extension of a file, such as "H".  Returns NULL if none.
   string if there's no extension.
-----------------------------------------------------------------------------*/

const char *fext(const char *fname)
	{
	if (strchr(fname,':'))		fname = strchr(fname,':')+1;
	while (strchr(fname,'/'))	fname = strchr(fname,'/')+1;
	do	{
		fname = strchr(fname,'.')+1;
		} while (fname && strchr(fname,'.'));
	return fname;
	};

/*-----------------------------------------------------------------------------
   fgetstartupdir: Returns the directory that the program was run from.
   Example of directory: "C:\UNREAL\"
-----------------------------------------------------------------------------*/

void fgetstartupdir (char *run_path)
	{
	strcpy(run_path,"");
	};

/*-----------------------------------------------------------------------------
   mystrncpy: Copy a string with length checking
-----------------------------------------------------------------------------*/

char *mystrncpy (char *dest, const char *src, int maxlen)
	{
	strncpy(dest,src,maxlen);
	dest[maxlen-1]=0;
	return dest;
	};

/*-----------------------------------------------------------------------------
   mystrncpy: Concatenate a string with length checking
-----------------------------------------------------------------------------*/

char *mystrncat (char *dest, const char *src, int maxlen)
	{
	char *newdest;
	int  len;
	//
	len      = strlen(dest);
	maxlen  -= len;
	newdest  = dest+len;
	//
	if (maxlen>0)
		{
		strncpy(newdest,src,maxlen);
		newdest[maxlen-1]=0;
		};
	return dest;
	};

/*-----------------------------------------------------------------------------
   strcrc: Compute the 32-bit CRC of a data buffer
-----------------------------------------------------------------------------*/

unsigned long CRCTable[256] =
	{
    0x00000000,  0x77073096, 0x0EE0E612C,  0x990951BA,
    0x076DC419,  0x706AF48F, 0x0E963A535,  0x9E6495A3,
    0x0EDB8832,  0x79DCB8A4, 0x0E0D5E91E,  0x97D2D988,
    0x09B64C2B,  0x7EB17CBD, 0x0E7B82D07,  0x90BF1D91,
    0x1DB71064,  0x6AB020F2, 0x0F3B97148,  0x84BE41DE,
    0x1ADAD47D,  0x6DDDE4EB, 0x0F4D4B551,  0x83D385C7,
    0x136C9856,  0x646BA8C0, 0x0FD62F97A,  0x8A65C9EC,
    0x14015C4F,  0x63066CD9, 0x0FA0F3D63,  0x8D080DF5,
    0x3B6E20C8,  0x4C69105E, 0x0D56041E4,  0x0A2677172,
    0x3C03E4D1,  0x4B04D447, 0x0D20D85FD,  0x0A50AB56B,
    0x35B5A8FA,  0x42B2986C, 0x0DBBBC9D6,  0x0ACBCF940,
    0x32D86CE3,  0x45DF5C75, 0x0DCD60DCF,  0x0ABD13D59,
    0x26D930AC,  0x51DE003A, 0x0C8D75180,  0x0BFD06116,
    0x21B4F4B5,  0x56B3C423, 0x0CFBA9599,  0x0B8BDA50F,
    0x2802B89E,  0x5F058808, 0x0C60CD9B2,  0x0B10BE924,
    0x2F6F7C87,  0x58684C11, 0x0C1611DAB,  0x0B6662D3D,
    0x76DC4190,  0x01DB7106, 0x98D220BC,   0x0EFD5102A,
    0x71B18589,  0x06B6B51F, 0x9FBFE4A5,   0x0E8B8D433,
    0x7807C9A2,  0x0F00F934, 0x9609A88E,   0x0E10E9818,
    0x7F6A0DBB,  0x086D3D2D, 0x91646C97,   0x0E6635C01,
    0x6B6B51F4,  0x1C6C6162, 0x856530D8,   0x0F262004E,
    0x6C0695ED,  0x1B01A57B, 0x8208F4C1,   0x0F50FC457,
    0x65B0D9C6,  0x12B7E950, 0x8BBEB8EA,   0x0FCB9887C,
    0x62DD1DDF,  0x15DA2D49, 0x8CD37CF3,   0x0FBD44C65,
    0x4DB26158,  0x3AB551CE, 0x0A3BC0074,  0x0D4BB30E2,
    0x4ADFA541,  0x3DD895D7, 0x0A4D1C46D,  0x0D3D6F4FB,
    0x4369E96A,  0x346ED9FC, 0x0AD678846,  0x0DA60B8D0,
    0x44042D73,  0x33031DE5, 0x0AA0A4C5F,  0x0DD0D7CC9,
    0x5005713C,  0x270241AA, 0x0BE0B1010,  0x0C90C2086,
    0x5768B525,  0x206F85B3, 0x0B966D409,  0x0CE61E49F,
    0x5EDEF90E,  0x29D9C998, 0x0B0D09822,  0x0C7D7A8B4,
    0x59B33D17,  0x2EB40D81, 0x0B7BD5C3B,  0x0C0BA6CAD,
    0x0EDB88320, 0x9ABFB3B6, 0x03B6E20C,   0x74B1D29A,
    0x0EAD54739, 0x9DD277AF, 0x04DB2615,   0x73DC1683,
    0x0E3630B12, 0x94643B84, 0x0D6D6A3E,   0x7A6A5AA8,
    0x0E40ECF0B, 0x9309FF9D, 0x0A00AE27,   0x7D079EB1,
    0x0F00F9344, 0x8708A3D2, 0x1E01F268,   0x6906C2FE,
    0x0F762575D, 0x806567CB, 0x196C3671,   0x6E6B06E7,
    0x0FED41B76, 0x89D32BE0, 0x10DA7A5A,   0x67DD4ACC,
    0x0F9B9DF6F, 0x8EBEEFF9, 0x17B7BE43,   0x60B08ED5,
    0x0D6D6A3E8, 0x0A1D1937E, 0x38D8C2C4,  0x4FDFF252,
    0x0D1BB67F1, 0x0A6BC5767, 0x3FB506DD,  0x48B2364B,
    0x0D80D2BDA, 0x0AF0A1B4C, 0x36034AF6,  0x41047A60,
    0x0DF60EFC3, 0x0A867DF55, 0x316E8EEF,  0x4669BE79,
    0x0CB61B38C, 0x0BC66831A, 0x256FD2A0,  0x5268E236,
    0x0CC0C7795, 0x0BB0B4703, 0x220216B9,  0x5505262F,
    0x0C5BA3BBE, 0x0B2BD0B28, 0x2BB45A92,  0x5CB36A04,
    0x0C2D7FFA7, 0x0B5D0CF31, 0x2CD99E8B,  0x5BDEAE1D,
    0x9B64C2B0,  0x0EC63F226, 0x756AA39C,  0x026D930A,
    0x9C0906A9,  0x0EB0E363F, 0x72076785,  0x05005713,
    0x95BF4A82,  0x0E2B87A14, 0x7BB12BAE,  0x0CB61B38,
    0x92D28E9B,  0x0E5D5BE0D, 0x7CDCEFB7,  0x0BDBDF21,
    0x86D3D2D4,  0x0F1D4E242, 0x68DDB3F8,  0x1FDA836E,
    0x81BE16CD,  0x0F6B9265B, 0x6FB077E1,  0x18B74777,
    0x88085AE6,  0x0FF0F6A70, 0x66063BCA,  0x11010B5C,
    0x8F659EFF,  0x0F862AE69, 0x616BFFD3,  0x166CCF45,
    0x0A00AE278, 0x0D70DD2EE, 0x4E048354,  0x3903B3C2,
    0x0A7672661, 0x0D06016F7, 0x4969474D,  0x3E6E77DB,
    0x0AED16A4A, 0x0D9D65ADC, 0x40DF0B66,  0x37D83BF0,
    0x0A9BCAE53, 0x0DEBB9EC5, 0x47B2CF7F,  0x30B5FFE9,
    0x0BDBDF21C, 0x0CABAC28A, 0x53B39330,  0x24B4A3A6,
    0x0BAD03605, 0x0CDD70693, 0x54DE5729,  0x23D967BF,
    0x0B3667A2E, 0x0C4614AB8, 0x5D681B02,  0x2A6F2B94,
    0x0B40BBE37, 0x0C30C8EA1, 0x5A05DF1B,  0x2D02EF8D
	};

//
// CRC32 computer based on polynomial:
// x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
//
unsigned long strcrc (const unsigned char *Data, int Length)
	{
	unsigned long CRC, Index;
	int i;
	//
	CRC = 0xFFFFFFFF;
	for (i=0; i<Length; i++)
		{
		Index = (CRC ^ *(Data++)) & 0x000000FF;
		CRC   = ((CRC >> 8) & 0x00FFFFFF) ^ CRCTable[Index];
		};
	return ~CRC;
	};

unsigned long strhash (char *Data, int NumKeys)
	{
	return strcrc((unsigned char *)Data,strlen(Data)) % NumKeys;
	};

inline char UPPER(char c) {return ((c<'a')||(c>'z')) ? c : (c+'A'-'a');};
//
// Find string in string, case insensitive, requires non-alphanumeric lead-in
//
const char *mystrstr (const char *str, const char *find)
	{
	int alnum,length;
	char f,c;
	//
	alnum  = 0;
	f      = UPPER(*find);
	length = strlen(find)-1;
	find   ++;
	c      = *str++;
	while(c)
		{
		c = UPPER(c);
		if ((!alnum) && (c==f) && !strnicmp(str,find,length)) return str-1;
		alnum = ((c>='A')&&(c<='Z')) || ((c>='0')&&(c<='9'));
		c = *str++;
		};
	return NULL;
	};

const char *spc(int num)
	{
	static char spacing[256];
	static int  oldnum=-1;
	//
	if (num!=oldnum)
		{
		mymemset (spacing,' ',num); // Indentation for text file
		spacing [num]=0;
		oldnum=num;
		};
	return spacing;
	};

/*-----------------------------------------------------------------------------
	mymemset
-----------------------------------------------------------------------------*/

//
// Set memory to a value
//
void UNREAL_API mymemset(void *Dest,char c,int Count)
	{
	#ifdef ASM
	__asm
		{
		mov al,[c]
		mov edi,[Dest]
		mov ah,al
		mov esi,[Count]
		mov ebx,eax
		cmp esi,0
		jle LoopOut
		shl eax,16
		dec esi
		mov ax,bx
		;
		PreLoop1:
		test edi,15
		jz Loop4Enter
		mov [edi],al
		inc edi
		sub esi,1
		jnc PreLoop1
		jmp LoopOut
		;
		Loop4Enter:
		sub esi,15
		jnc Loop16
		jmp PostLoop1Enter
		;
		/*
		mov ecx,[FGlobalPlatform.MMX]
		cmp ecx,0
		jnz DoMMX
		*/
		ALIGN 16
		Loop16:
		mov [edi   ],eax
		mov [edi+4 ],eax
		mov [edi+8 ],eax
		mov [edi+12],eax
		add edi,16
		sub esi,16
		jnc Loop16
		jmp PostLoop1Enter
		;
		ALIGN 16
		;DoMMX:
		;...
		Loop16MMX:
		mov [edi   ],eax
		mov [edi+4 ],eax
		mov [edi+8 ],eax
		mov [edi+12],eax
		add edi,16
		sub esi,16
		jnc Loop16MMX
		;
		PostLoop1Enter:
		add esi,16
		jz  LoopOut
		;
		PostLoop1:
		mov [edi],al
		inc edi
		sub esi,1
		jnz PostLoop1
		;
		LoopOut:
		};
	#else
		memset(Dest,c,Count);
	#endif
	};

/*-----------------------------------------------------------------------------
	mymemeq
-----------------------------------------------------------------------------*/

//
// Compare memory.  Fast for large blocks.  Returns 1 if equal, 0 if nonequal.
//
UNREAL_API int mymemeq(void *P1, void *P2, int n)
	{
	BYTE *B1 = (BYTE *)P1;
	BYTE *B2 = (BYTE *)P2;
	while (n&15)
		{
		if (*B1 != *B2) return 0;
		B1++; B2++; n--;
		};
	int Result=1;
	if (n>0) __asm
		{
		mov esi,[n]
		mov eax,[B1]
		shr esi,4
		mov ebx,[B2]
		mov [Result],0
		;
		CmpLoop:
		;1
		mov ecx,[eax]
		mov edx,[ebx]
		cmp ecx,edx
		jne NotEq
		;2
		mov ecx,[eax+4]
		mov edx,[ebx+4]
		cmp ecx,edx
		jne NotEq
		;3
		mov ecx,[eax+8]
		mov edx,[ebx+8]
		cmp ecx,edx
		jne NotEq
		;4
		mov ecx,[eax+12]
		mov edx,[ebx+12]
		cmp ecx,edx
		jne NotEq
		;
		add eax,16
		add ebx,16
		dec esi
		jg  CmpLoop
		;
		mov [Result],1
		;
		NotEq:
		};
	return Result;
	};

/*-----------------------------------------------------------------------------
	The End
-----------------------------------------------------------------------------*/
