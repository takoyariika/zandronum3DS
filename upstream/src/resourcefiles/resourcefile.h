

#ifndef __RESFILE_H
#define __RESFILE_H

#include "files.h"

class FResourceFile;

struct FResourceLump
{
	friend class FResourceFile;

	int				LumpSize;
	char *			FullName;		// only valid for files loaded from a .zip file
	union
	{
		char		Name[9];

		DWORD		dwName;			// These are for accessing the first 4 or 8 chars of
		QWORD		qwName;			// Name as a unit without breaking strict aliasing rules
	};
	BYTE			Flags;
	SBYTE			RefCount;
	char *			Cache;
	FResourceFile *	Owner;
	int				Namespace;
#ifdef __3DS__
	DWORD ResourceCacheKey;
	long ResourceCacheOffset;
	DWORD ResourceCacheCRC;
#endif

	FResourceLump()
	{
		FullName = NULL;
		Cache = NULL;
		Owner = NULL;
		Flags = 0;
		RefCount = 0;
		Namespace = 0;	// ns_global
		*Name = 0;
#ifdef __3DS__
		ResourceCacheKey = ~0u;
		ResourceCacheOffset = -1;
		ResourceCacheCRC = 0;
#endif
	}

	virtual ~FResourceLump();
	virtual FileReader *GetReader();
	virtual FileReader *NewReader();
#ifdef __3DS__
	virtual FileReader *OpenTextureHeader() { return NULL; }
#endif
	virtual int GetFileOffset() { return -1; }
	virtual int GetIndexNum() const { return 0; }
	void LumpNameSetup(const char *iname);
	void CheckEmbedded();

	void *CacheLump();
	int ReleaseCache();

protected:
	virtual int FillCache() = 0;

};

class FResourceFile
{
public:
	FileReader *Reader;
	const char *Filename;
	bool IsOptional; // [TP]
	bool ContainsAuthenticatedLumps; // [SB]
#ifdef __3DS__
	FileReader *ResourceCacheReader;
#endif
protected:
	DWORD NumLumps;

	FResourceFile(const char *filename, FileReader *r);

private:
	DWORD FirstLump;

public:
	static FResourceFile *OpenResourceFile(const char *filename, FileReader *file, bool quiet = false);
	static FResourceFile *OpenDirectory(const char *filename, bool quiet = false);
	virtual ~FResourceFile();
	FileReader *GetReader() const { return Reader; }
	DWORD LumpCount() const { return NumLumps; }
	DWORD GetFirstLump() const { return FirstLump; }
	void SetFirstLump(DWORD f) { FirstLump = f; }

	virtual void FindStrifeTeaserVoices ();
	virtual bool Open(bool quiet) = 0;
	virtual FResourceLump *GetLump(int no) = 0;
};

#ifdef __3DS__
void Z3DS_LoadResourceCache(FResourceFile *, const BYTE *, unsigned, unsigned);
bool Z3DS_ReadResourceCache(FResourceLump *);
bool Z3DS_RetainResource(FResourceLump *);
void Z3DS_ForgetResource(FResourceLump *);
#endif

struct FUncompressedLump : public FResourceLump
{
	int				Position;

	virtual FileReader *GetReader();
	virtual int FillCache();
	virtual int GetFileOffset() { return Position; }

};


// Base class for uncompressed resource files (WAD, GRP, PAK and single lumps)
class FUncompressedFile : public FResourceFile
{
protected:
	FUncompressedLump * Lumps;


	FUncompressedFile(const char *filename, FileReader *r);
	virtual ~FUncompressedFile();
	virtual FResourceLump *GetLump(int no) { return ((unsigned)no < NumLumps)? &Lumps[no] : NULL; }

public:
};


struct FExternalLump : public FResourceLump
{
	const char *filename;	// the actual file name. This is not necessarily the same as the lump name!

	FExternalLump(const char *_filename, int filesize = -1);
	~FExternalLump();
	virtual int FillCache();

};






#endif
