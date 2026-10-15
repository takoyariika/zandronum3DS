/*
** file_zip.cpp
**
**---------------------------------------------------------------------------
** Copyright 1998-2009 Randy Heit
** Copyright 2005-2009 Christoph Oelckers
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**---------------------------------------------------------------------------
**
**
*/

#include "resourcefile.h"
#include "cmdlib.h"
#include "templates.h"
#include "v_text.h"
#include "w_wad.h"
#include "w_zip.h"
#include "i_system.h"
#ifdef __3DS__
#include "md5.h"
#include <algorithm>
#endif
#include "ancientzip.h"
#include <set>
#include <string>

#define BUFREADCOMMENT (0x400)

//-----------------------------------------------------------------------
//
// Finds the central directory end record in the end of the file.
// Taken from Quake3 source but the file in question is not GPL'ed. ;)
//
//-----------------------------------------------------------------------

static DWORD Zip_FindCentralDir(FileReader * fin)
{
	unsigned char buf[BUFREADCOMMENT + 4];
	DWORD FileSize;
	DWORD uBackRead;
	DWORD uMaxBack; // maximum size of global comment
	DWORD uPosFound=0;

	fin->Seek(0, SEEK_END);

	FileSize = fin->Tell();
	uMaxBack = MIN<DWORD>(0xffff, FileSize);

	uBackRead = 4;
	while (uBackRead < uMaxBack)
	{
		DWORD uReadSize, uReadPos;
		int i;
		if (uBackRead + BUFREADCOMMENT > uMaxBack) 
			uBackRead = uMaxBack;
		else
			uBackRead += BUFREADCOMMENT;
		uReadPos = FileSize - uBackRead;

		uReadSize = MIN<DWORD>((BUFREADCOMMENT + 4), (FileSize - uReadPos));

		if (fin->Seek(uReadPos, SEEK_SET) != 0) break;

		if (fin->Read(buf, (SDWORD)uReadSize) != (SDWORD)uReadSize) break;

		for (i = (int)uReadSize - 3; (i--) > 0;)
		{
			if (buf[i] == 'P' && buf[i+1] == 'K' && buf[i+2] == 5 && buf[i+3] == 6)
			{
				uPosFound = uReadPos + i;
				break;
			}
		}

		if (uPosFound != 0)
			break;
	}
	return uPosFound;
}


enum
{
	LUMPFZIP_NEEDFILESTART = 128
};

//==========================================================================
//
// Zip Lump
//
//==========================================================================

struct FZipLump : public FResourceLump
{
	WORD	GPFlags;
	BYTE	Method;
	int		CompressedSize;
	int		Position;
#ifdef __3DS__
	const char *TextureHeader = NULL;
	int TextureHeaderSize = 0;
	DWORD EntryCRC;
	FileReader *OpenTextureHeader()
	{
		return TextureHeader ? new MemoryReader(TextureHeader, TextureHeaderSize) : NULL;
	}
#endif

	virtual FileReader *GetReader();
	virtual int FillCache();

private:
	void SetLumpAddress();
	virtual int GetFileOffset() 
	{ 
		if (Method != METHOD_STORED) return -1;
		if (Flags & LUMPFZIP_NEEDFILESTART) SetLumpAddress(); return Position; 
	}
};


//==========================================================================
//
// Zip file
//
//==========================================================================

class FZipFile : public FResourceFile
{
	FZipLump *Lumps;
#ifdef __3DS__
	TArray<BYTE> TextureHeaderCache;
	void LoadTextureHeaders(const BYTE *directory, unsigned size);
#endif

	static int STACK_ARGS lumpcmp(const void * a, const void * b);

public:
	FZipFile(const char * filename, FileReader *file);
	virtual ~FZipFile();
	bool Open(bool quiet);
	void ReleaseTextureHeaders();
	virtual FResourceLump *GetLump(int no) { return ((unsigned)no < NumLumps)? &Lumps[no] : NULL; }
};
#ifdef __3DS__
static TArray<FZipFile *> &textureHeaderOwners = *new TArray<FZipFile *>;
void Z3DS_ReleaseTextureHeaders()
{
	for (unsigned i=0; i<textureHeaderOwners.Size(); ++i) textureHeaderOwners[i]->ReleaseTextureHeaders();
}
void FZipFile::ReleaseTextureHeaders()
{
	for (unsigned i=0; i<NumLumps; ++i) { Lumps[i].TextureHeader=NULL; Lumps[i].TextureHeaderSize=0; }
	TextureHeaderCache.Clear(); TextureHeaderCache.ShrinkToFit();
}
#else
void FZipFile::ReleaseTextureHeaders() {}
#endif



int STACK_ARGS FZipFile::lumpcmp(const void * a, const void * b)
{
	FZipLump * rec1 = (FZipLump *)a;
	FZipLump * rec2 = (FZipLump *)b;

	return stricmp(rec1->FullName, rec2->FullName);
}


//==========================================================================
//
// Zip file
//
//==========================================================================

FZipFile::FZipFile(const char * filename, FileReader *file)
: FResourceFile(filename, file)
{
	Lumps = NULL;
#ifdef __3DS__
	textureHeaderOwners.Push(this);
#endif
}

bool FZipFile::Open(bool quiet)
{
	DWORD centraldir = Zip_FindCentralDir(Reader);
	FZipEndOfCentralDirectory info;
	int skipped = 0;

	Lumps = NULL;

	if (centraldir == 0)
	{
		if (!quiet) Printf(TEXTCOLOR_RED "\n%s: ZIP file corrupt!\n", Filename);
		return false;
	}

	// Read the central directory info.
	Reader->Seek(centraldir, SEEK_SET);
	Reader->Read(&info, sizeof(FZipEndOfCentralDirectory));

	// No multi-disk zips!
	if (info.NumEntries != info.NumEntriesOnAllDisks ||
		info.FirstDisk != 0 || info.DiskNumber != 0)
	{
		if (!quiet) Printf(TEXTCOLOR_RED "\n%s: Multipart Zip files are not supported.\n", Filename);
		return false;
	}

	NumLumps = LittleShort(info.NumEntries);
	Lumps = new FZipLump[NumLumps];

	// Load the entire central directory. Too bad that this contains variable length entries...
	int dirsize = LittleLong(info.DirectorySize);
	void *directory = malloc(dirsize);
	Reader->Seek(LittleLong(info.DirectoryOffset), SEEK_SET);
	Reader->Read(directory, dirsize);

	// [AK] Store the names of all the lumps in the current directory.
	std::set<std::string> oldNames;
	FString currentDirectory;

	char *dirptr = (char*)directory;
	FZipLump *lump_p = Lumps;
	for (DWORD i = 0; i < NumLumps; i++)
	{
		FZipCentralDirectoryInfo *zip_fh = (FZipCentralDirectoryInfo *)dirptr;

		int len = LittleShort(zip_fh->NameLength);
		FString name(dirptr + sizeof(FZipCentralDirectoryInfo), len);
		dirptr += sizeof(FZipCentralDirectoryInfo) + 
				  LittleShort(zip_fh->NameLength) + 
				  LittleShort(zip_fh->ExtraLength) + 
				  LittleShort(zip_fh->CommentLength);

		if (dirptr > ((char*)directory) + dirsize)	// This directory entry goes beyond the end of the file.
		{
			free(directory);
			if (!quiet) Printf(TEXTCOLOR_RED "\n%s: Central directory corrupted.", Filename);
			return false;
		}
		
		// skip Directories
		if (name[len - 1] == '/' && LittleLong(zip_fh->UncompressedSize) == 0) 
		{
			skipped++;
			continue;
		}

		// Ignore unknown compression formats
		zip_fh->Method = LittleShort(zip_fh->Method);
		if (zip_fh->Method != METHOD_STORED &&
			zip_fh->Method != METHOD_DEFLATE &&
			zip_fh->Method != METHOD_LZMA &&
			zip_fh->Method != METHOD_BZIP2 &&
			zip_fh->Method != METHOD_IMPLODE &&
			zip_fh->Method != METHOD_SHRINK)
		{
			if (!quiet) Printf(TEXTCOLOR_YELLOW "\n%s: '%s' uses an unsupported compression algorithm (#%d).\n", Filename, name.GetChars(), zip_fh->Method);
			skipped++;
			continue;
		}
		// Also ignore encrypted entries
		zip_fh->Flags = LittleShort(zip_fh->Flags);
		if (zip_fh->Flags & ZF_ENCRYPTED)
		{
			if (!quiet) Printf(TEXTCOLOR_YELLOW "\n%s: '%s' is encrypted. Encryption is not supported.\n", Filename, name.GetChars());
			skipped++;
			continue;
		}

		FixPathSeperator(name);
		name.ToLower();

		lump_p->LumpNameSetup(name);
		lump_p->LumpSize = LittleLong(zip_fh->UncompressedSize);
		lump_p->Owner = this;
		// The start of the Reader will be determined the first time it is accessed.
		lump_p->Flags = LUMPF_ZIPFILE | LUMPFZIP_NEEDFILESTART;
		lump_p->Method = BYTE(zip_fh->Method);
		lump_p->GPFlags = zip_fh->Flags;
		lump_p->CompressedSize = LittleLong(zip_fh->CompressedSize);
		lump_p->Position = LittleLong(zip_fh->LocalHeaderOffset);
#ifdef __3DS__
		lump_p->ResourceCacheKey = lump_p->Position;
		lump_p->EntryCRC = LittleLong(zip_fh->CRC32);
#endif
		lump_p->CheckEmbedded();

		// Ignore some very specific names
		if (0 == stricmp("dehacked.exe", name))
		{
			memset(lump_p->Name, 0, sizeof(lump_p->Name));
		}

		// [AK] Get the full name of the directory that the lump is located in.
		const char *directory = strrchr(name, '/');
		FString directoryName = "/";

		if (directory != NULL)
		{
			directoryName = name.Left(ULONG(directory - name.GetChars()));
		}

		// [AK] Check if we're in the same directory. If not, clear the list of old lump names.
		if ((currentDirectory.IsEmpty()) || (currentDirectory.CompareNoCase(directoryName) != 0))
		{
			oldNames.clear();
			currentDirectory = directoryName;
		}

		// [AK] Check for any duplicate lumps in the current directory.
		{
			if (oldNames.find(name.GetChars()) != oldNames.end())
			{
				const char *shortenedFilename = strrchr(Filename, '/');
				if (shortenedFilename != NULL) shortenedFilename++;

				const char *actualFilename = shortenedFilename != NULL ? shortenedFilename : Filename;
				bool addDuplicate = true;

				// [AK] To keep the list of duplicate lumps as small as possible, first check if this
				// particular lump isn't already on the list. If not, add it to the list then.
				for (unsigned int j = 0; j < DuplicateLumps.Size(); j++)
				{
					if ((DuplicateLumps[j].CompareNoCase(lump_p->FullName) == 0) && (DuplicateLumpFilenames[j].CompareNoCase(actualFilename) == 0 ))
					{
						addDuplicate = false;
						break;
					}
				}

				if (addDuplicate)
				{
					DuplicateLumps.Push(lump_p->FullName);
					DuplicateLumpFilenames.Push(actualFilename);
				}

			}
		}

		// [AK] Add this lump name to the list.
		oldNames.insert(name.GetChars());
		lump_p++;
	}
	// Resize the lump record array to its actual size
	NumLumps -= skipped;
#ifdef __3DS__
	LoadTextureHeaders((const BYTE *)directory, dirsize);
	Z3DS_LoadResourceCache(this, (const BYTE *)directory, dirsize, 1);
#endif
	free(directory);

	if (!quiet) Printf(TEXTCOLOR_NORMAL ", %d lumps\n", NumLumps);
	
	// Entries in Zips are sorted alphabetically.
	qsort(Lumps, NumLumps, sizeof(FZipLump), lumpcmp);
	return true;
}

#ifdef __3DS__
void FZipFile::LoadTextureHeaders(const BYTE *directory, unsigned size)
{
	FString path(Filename); path += ".3ds-cache";
	FileReader cache;
	if (!cache.Open(path)) return;
	long length = cache.GetLength();
	// Limit auxiliary heap usage and reject malformed/truncated cache files.
	if (length < 48 || length > 16 * 1024 * 1024) return;
	TextureHeaderCache.Resize(length);
	if (cache.Read(&TextureHeaderCache[0], length) != length) { TextureHeaderCache.Clear(); return; }
	const BYTE *base = &TextureHeaderCache[0], *end = base + length;
	BYTE digest[16]; MD5Context md5; md5.Update(directory, size); md5.Final(digest);
	DWORD sourceSize, count;
	memcpy(&sourceSize, base + 24, 4); memcpy(&count, base + 28, 4);
	if (memcmp(base, "Z3DTH003", 8) || memcmp(base + 8, digest, 16) ||
		LittleLong(sourceSize) != (DWORD)Reader->GetLength() || LittleLong(count) > NumLumps)
	{ TextureHeaderCache.Clear(); Printf("3DS texture cache ignored (archive changed): %s\n", Filename); return; }
	md5.Init(); md5.Update(base + 48, length - 48); md5.Final(digest);
	if (memcmp(base + 32, digest, 16) || !NumLumps) { TextureHeaderCache.Clear(); return; }
	TArray<FZipLump *> ordered;
	for (unsigned i = 0; i < NumLumps; ++i) ordered.Push(&Lumps[i]);
	std::sort(&ordered[0], &ordered[0] + ordered.Size(), [](FZipLump *a, FZipLump *b) { return a->Position < b->Position; });
	const BYTE *record = base + 48; unsigned index = 0, matched = 0;
	DWORD previous = 0;
	for (unsigned n = 0; n < LittleLong(count); ++n)
	{
		if (end - record < 20) goto invalid;
		DWORD fields[5]; memcpy(fields, record, 20); record += 20;
		for (int f = 0; f < 5; ++f) fields[f] = LittleLong(fields[f]);
		if ((n && fields[0] <= previous) || fields[4] < 41 || fields[4] > 65536 || (unsigned)(end - record) < fields[4]) goto invalid;
		previous = fields[0];
		while (index < ordered.Size() && (DWORD)ordered[index]->Position < fields[0]) ++index;
		if (index >= ordered.Size()) goto invalid;
		FZipLump *lump = ordered[index];
		if ((DWORD)lump->Position != fields[0] || (DWORD)lump->LumpSize != fields[1] ||
			(DWORD)lump->CompressedSize != fields[2] || lump->EntryCRC != fields[3] ||
			memcmp(record, "\x89PNG\r\n\x1a\n", 8)) goto invalid;
		lump->TextureHeader = (const char *)record; lump->TextureHeaderSize = fields[4]; ++matched;
		record += fields[4];
	}
	if (record != end) goto invalid;
	Printf("3DS texture cache: %u PNG headers, %lu KiB (%s)\n", matched, (unsigned long)length/1024, Filename);
	return;
invalid:
	for (unsigned i = 0; i < NumLumps; ++i) { Lumps[i].TextureHeader = NULL; Lumps[i].TextureHeaderSize = 0; }
	TextureHeaderCache.Clear(); Printf("3DS texture cache ignored (invalid data): %s\n", Filename);
}
#endif

//==========================================================================
//
// Zip file
//
//==========================================================================

FZipFile::~FZipFile()
{
#ifdef __3DS__
	for (unsigned i=0; i<textureHeaderOwners.Size(); ++i)
		if (textureHeaderOwners[i]==this) { textureHeaderOwners.Delete(i); break; }
#endif
	if (Lumps != NULL) delete [] Lumps;
}

//==========================================================================
//
// SetLumpAddress
//
//==========================================================================

void FZipLump::SetLumpAddress()
{
	// This file is inside a zip and has not been opened before.
	// Position points to the start of the local file header, which we must
	// read and skip so that we can get to the actual file data.
	FZipLocalFileHeader localHeader;
	int skiplen;

	FileReader *file = Owner->Reader;

	file->Seek(Position, SEEK_SET);
	file->Read(&localHeader, sizeof(localHeader));
	skiplen = LittleShort(localHeader.NameLength) + LittleShort(localHeader.ExtraLength);
	Position += sizeof(localHeader) + skiplen;
	Flags &= ~LUMPFZIP_NEEDFILESTART;
}

//==========================================================================
//
// Get reader (only returns non-NULL if not encrypted)
//
//==========================================================================

FileReader *FZipLump::GetReader()
{
#ifdef __3DS__
	if (ResourceCacheOffset >= 0) return NULL;
#endif
	// Don't return the reader if this lump is encrypted
	// In that case always force caching of the lump
	if (Method == METHOD_STORED)
	{
		if (Flags & LUMPFZIP_NEEDFILESTART) SetLumpAddress();
		Owner->Reader->Seek(Position, SEEK_SET);
		return Owner->Reader;
	}
	else return NULL;	
}

//==========================================================================
//
// Fills the lump cache and performs decompression
//
//==========================================================================

int FZipLump::FillCache()
{
	if (Flags & LUMPFZIP_NEEDFILESTART) SetLumpAddress();
	const char *buffer;

	if (Method == METHOD_STORED && (buffer = Owner->Reader->GetBuffer()) != NULL)
	{
		// This is an in-memory file so the cache can point directly to the file's data.
		Cache = const_cast<char*>(buffer) + Position;
		RefCount = -1;
		return -1;
	}

	Owner->Reader->Seek(Position, SEEK_SET);
	Cache = new char[LumpSize];
	switch (Method)
	{
		case METHOD_STORED:
		{
			Owner->Reader->Read(Cache, LumpSize);
			break;
		}

		case METHOD_DEFLATE:
		{
#ifdef __3DS__
			// Small PNG/script entries are common in BYOC. Read exactly one
			// compressed entry instead of repeatedly fetching 4 KiB chunks
			// (including bytes from the following entry) through SD IPC.
			if (CompressedSize > 0 && CompressedSize <= 65536)
			{
				TArray<char> compressed;
				compressed.Resize(CompressedSize);
				if (Owner->Reader->Read(&compressed[0], CompressedSize) != CompressedSize)
					I_Error("Truncated ZIP entry: %s", FullName);
				MemoryReader input(&compressed[0], CompressedSize);
				FileReaderZ frz(input, true);
				frz.Read(Cache, LumpSize);
				break;
			}
#endif
			FileReaderZ frz(*Owner->Reader, true);
			frz.Read(Cache, LumpSize);
			break;
		}

		case METHOD_BZIP2:
		{
			FileReaderBZ2 frz(*Owner->Reader);
			frz.Read(Cache, LumpSize);
			break;
		}

		case METHOD_LZMA:
		{
			FileReaderLZMA frz(*Owner->Reader, LumpSize, true);
			frz.Read(Cache, LumpSize);
			break;
		}

		case METHOD_IMPLODE:
		{
			FZipExploder exploder;
			exploder.Explode((unsigned char *)Cache, LumpSize, Owner->Reader, CompressedSize, GPFlags);
			break;
		}

		case METHOD_SHRINK:
		{
			ShrinkLoop((unsigned char *)Cache, LumpSize, Owner->Reader, CompressedSize);
			break;
		}

		default:
			assert(0);
			return 0;
	}
	RefCount = 1;
	return 1;
}


//==========================================================================
//
// File open
//
//==========================================================================

FResourceFile *CheckZip(const char *filename, FileReader *file, bool quiet)
{
	char head[4];

	if (file->GetLength() >= (long)sizeof(FZipLocalFileHeader))
	{
		file->Seek(0, SEEK_SET);
		file->Read(&head, 4);
		file->Seek(0, SEEK_SET);
		if (!memcmp(head, "PK\x3\x4", 4))
		{
			FResourceFile *rf = new FZipFile(filename, file);
			if (rf->Open(quiet)) return rf;

			rf->Reader = NULL; // to avoid destruction of reader
			delete rf;
		}
	}
	return NULL;
}



