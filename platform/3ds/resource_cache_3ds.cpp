// All-resource disk cache, plus a bounded cache of recently read small lumps.
#include "resourcefiles/resourcefile.h"
#include "md5.h"
#include "i_system.h"
#include "cmdlib.h"
#include "zlib.h"
#include <algorithm>
#include <cstring>

// Wads is destroyed after this translation unit's statics on some link orders.
// Keep the registry alive until process exit: lump destructors still consult it.
static TArray<FResourceLump *> &retained = *new TArray<FResourceLump *>;
static unsigned retainedBytes;
unsigned Z3DS_PurgeResourceCache()
{
    unsigned freed=retainedBytes;
    while (retained.Size())
    {
        FResourceLump *old=retained[retained.Size()-1]; retained.Delete(retained.Size()-1);
        delete[] old->Cache; old->Cache=NULL;
    }
    retainedBytes=0;
    return freed;
}
void Z3DS_ForgetResource(FResourceLump *lump)
{
    for (unsigned i=0; i<retained.Size(); ++i)
        if (retained[i] == lump) { retainedBytes -= lump->LumpSize; retained.Delete(i); break; }
}
bool Z3DS_RetainResource(FResourceLump *lump)
{
    if (lump->ResourceCacheOffset < 0 || lump->LumpSize <= 0 || lump->LumpSize > 65536) return false;
    while (retained.Size() && (retainedBytes + lump->LumpSize > 4*1024*1024 || retained.Size() >= 256))
    {
        FResourceLump *old = retained[0]; retainedBytes -= old->LumpSize; retained.Delete(0);
        delete[] old->Cache; old->Cache = NULL;
    }
    retained.Push(lump); retainedBytes += lump->LumpSize;
    return true;
}
bool Z3DS_ReadResourceCache(FResourceLump *lump)
{
    FileReader *reader = lump->Owner->ResourceCacheReader;
    if (!reader || lump->ResourceCacheOffset < 0) return false;
    BYTE *data = new BYTE[lump->LumpSize];
    bool valid = reader->Seek(lump->ResourceCacheOffset, SEEK_SET) == 0 &&
                 reader->Read(data, lump->LumpSize) == lump->LumpSize &&
                 (DWORD)crc32(0, data, lump->LumpSize) == lump->ResourceCacheCRC;
    if (!valid)
    {
        delete[] data; lump->ResourceCacheOffset = -1;
        Printf("3DS resource cache: damaged entry, using original %s\n", lump->Name);
        return false;
    }
    lump->Cache = (char *)data; lump->RefCount = 1;
    return true;
}
void Z3DS_LoadResourceCache(FResourceFile *owner, const BYTE *directory, unsigned size, unsigned kind)
{
    FString path(owner->Filename); path += ".3ds-resources";
    FileReader *reader = new FileReader;
    if (!reader->Open(path)) { delete reader; return; }
    BYTE header[52], digest[16]; DWORD sourceSize, storedKind, count;
    if (reader->Read(header, sizeof(header)) != sizeof(header) || memcmp(header, "Z3DRC001", 8)) goto invalid;
    memcpy(&sourceSize, header+24, 4); memcpy(&storedKind, header+28, 4); memcpy(&count, header+32, 4);
    sourceSize = LittleLong(sourceSize); storedKind = LittleLong(storedKind); count = LittleLong(count);
    if (sourceSize != (DWORD)owner->Reader->GetLength() || kind != storedKind || count > 262144) goto invalid;
    {
        MD5Context md5; md5.Update(directory, size); md5.Final(digest);
        if (memcmp(digest, header+8, 16)) goto invalid;
        unsigned tableSize = count * 16;
        if (reader->GetLength() < 52 + (long)tableSize) goto invalid;
        TArray<BYTE> table; table.Resize(tableSize);
        if (tableSize && reader->Read(&table[0], tableSize) != (long)tableSize) goto invalid;
        md5.Init(); if (tableSize) md5.Update(&table[0], tableSize); md5.Final(digest);
        if (memcmp(digest, header+36, 16)) goto invalid;
        struct Entry { DWORD key, size, offset, crc; };
        TArray<Entry> entries; entries.Resize(count);
        for (unsigned i=0; i<count; ++i)
        {
            DWORD v[4]; memcpy(v, &table[i*16], 16);
            for (int j=0; j<4; ++j) v[j] = LittleLong(v[j]);
            if (v[2] < 52 + tableSize || v[2] > (DWORD)reader->GetLength() ||
                v[1] > (DWORD)reader->GetLength()-v[2]) goto invalid;
            entries[i] = {v[0], v[1], v[2], v[3]};
        }
        if(count) std::sort(&entries[0], &entries[0]+count, [](const Entry&a,const Entry&b){return a.key<b.key;});
        for(unsigned i=1;i<count;++i) if(entries[i-1].key==entries[i].key) goto invalid;
        TArray<unsigned> bindings; bindings.Resize(owner->LumpCount());
        // Validate every binding before enabling any of them.
        for (unsigned i=0; i<owner->LumpCount(); ++i)
        {
            FResourceLump *lump = owner->GetLump(i);
            if(!count) goto invalid;
            Entry *entry=std::lower_bound(&entries[0], &entries[0]+count, lump->ResourceCacheKey,
                [](const Entry&e,DWORD key){return e.key<key;});
            if (entry == &entries[0]+count || entry->key != lump->ResourceCacheKey || entry->size != (DWORD)lump->LumpSize) goto invalid;
            bindings[i]=entry-&entries[0];
        }
        for (unsigned i=0; i<owner->LumpCount(); ++i)
        {
            FResourceLump *lump = owner->GetLump(i);
            Entry &entry = entries[bindings[i]];
            lump->ResourceCacheOffset = entry.offset; lump->ResourceCacheCRC = entry.crc;
        }
    }
    owner->ResourceCacheReader = reader;
    Printf("3DS resource cache: %u entries, 4 MiB bounded RAM cache (%s)\n", owner->LumpCount(), owner->Filename);
    return;
invalid:
    delete reader;
    Printf("3DS resource cache ignored (changed archive/invalid index): %s\n", owner->Filename);
}
