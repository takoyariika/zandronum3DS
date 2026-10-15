param([Parameter(Position=0)][string]$Archive, [string]$Output)
$ErrorActionPreference='Stop'
if (-not $Archive) { $Archive=Read-Host 'WAD/PK3/ZIP path' }
$Archive=[IO.Path]::GetFullPath($Archive.Trim().Trim('"'))
if (-not $Output) { $Output=$Archive+'.3ds-resources' }
$Output=[IO.Path]::GetFullPath($Output)
if ($Output -eq $Archive) { throw 'Output must differ from the original.' }
if (-not $Output.EndsWith('.3ds-resources',[StringComparison]::OrdinalIgnoreCase)) { throw 'Output must end in .3ds-resources; original archives are never overwritten.' }
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$definitions=@'
using System;
using System.IO;
using System.IO.Compression;
using System.Collections.Generic;
using System.Security.Cryptography;
public static class ResourceCacheBuilder {
    class Record { public uint Key, Size, Offset, CRC, Source; public ZipArchiveEntry Zip; }
    static uint U32(byte[] b,int p) { return BitConverter.ToUInt32(b,p); }
    static uint[] crcTable = MakeTable();
    static uint[] MakeTable() { var t=new uint[256]; for(uint n=0;n<256;n++){uint c=n;for(int k=0;k<8;k++)c=(c&1)!=0?0xedb88320^(c>>1):c>>1;t[n]=c;}return t; }
    static uint Copy(Stream input, Stream output, uint size) {
        byte[] buffer=new byte[65536]; uint left=size, crc=0xffffffff;
        while(left>0) { int read=input.Read(buffer,0,(int)Math.Min(left,(uint)buffer.Length)); if(read<=0)throw new InvalidDataException("Truncated resource");
            output.Write(buffer,0,read); for(int i=0;i<read;i++)crc=crcTable[(crc^buffer[i])&255]^(crc>>8); left-=(uint)read; }
        return crc^0xffffffff;
    }
    public static uint Build(string archive,string output) {
        string temporary=output+".building";
        using(var source=File.OpenRead(archive)) using(var r=new BinaryReader(source)) using(var md5=MD5.Create()) {
            if(source.Length>Int32.MaxValue)throw new InvalidDataException("Archives over 2 GiB are unsupported");
            byte[] first=r.ReadBytes(12), directory; uint kind; var entries=new List<Record>(); ZipArchive zip=null;
            try {
                if(first.Length==12 && (System.Text.Encoding.ASCII.GetString(first,0,4)=="IWAD" || System.Text.Encoding.ASCII.GetString(first,0,4)=="PWAD")) {
                    kind=2; uint count=U32(first,4), offset=U32(first,8);
                    if(count>262144 || (ulong)offset+count*16UL>(ulong)source.Length)throw new InvalidDataException("Invalid/unsupported WAD directory");
                    source.Position=offset; directory=r.ReadBytes(checked((int)count*16));
                    for(uint i=0;i<count;i++) { int p=(int)i*16; uint pos=U32(directory,p), size=U32(directory,p+4);
                        if((directory[p+8]&128)!=0)throw new InvalidDataException("Console-compressed WADs are unsupported by this preparation tool");
                        if((ulong)pos+size>(ulong)source.Length)throw new InvalidDataException("Invalid WAD lump");
                        entries.Add(new Record {Key=i,Size=size,Source=pos}); }
                } else {
                    kind=1; int tailSize=(int)Math.Min(65557,source.Length); source.Position=source.Length-tailSize;
                    byte[] tail=r.ReadBytes(tailSize); int e=-1;
                    for(int p=tail.Length-22;p>=0;p--)if(U32(tail,p)==0x06054b50 && p+22+BitConverter.ToUInt16(tail,p+20)==tail.Length){e=p;break;}
                    if(e<0 || BitConverter.ToUInt16(tail,e+4)!=0 || BitConverter.ToUInt16(tail,e+6)!=0)throw new InvalidDataException("Unsupported ZIP");
                    uint count=BitConverter.ToUInt16(tail,e+10), size=U32(tail,e+12), offset=U32(tail,e+16);
                    if(count==65535 || (ulong)offset+size>(ulong)source.Length)throw new InvalidDataException("ZIP64 is unsupported");
                    source.Position=offset; directory=r.ReadBytes((int)size);
                    source.Position=0; zip=new ZipArchive(source,ZipArchiveMode.Read,true);
                    if(zip.Entries.Count!=count)throw new InvalidDataException("ZIP directory mismatch");
                    int cursor=0;
                    for(int i=0;i<count;i++) {
                        if(cursor+46>directory.Length || U32(directory,cursor)!=0x02014b50)throw new InvalidDataException("Bad ZIP directory");
                        if((BitConverter.ToUInt16(directory,cursor+8)&1)!=0)throw new InvalidDataException("Encrypted entries are unsupported");
                        uint method=BitConverter.ToUInt16(directory,cursor+10);
                        if(method!=0 && method!=8)throw new InvalidDataException("Prepare stored/deflated PK3/ZIP archives only");
                        entries.Add(new Record {Key=U32(directory,cursor+42),Size=U32(directory,cursor+24),CRC=U32(directory,cursor+16),Zip=zip.Entries[i]});
                        cursor+=46+BitConverter.ToUInt16(directory,cursor+28)+BitConverter.ToUInt16(directory,cursor+30)+BitConverter.ToUInt16(directory,cursor+32);
                    }
                }
                using(var cache=new FileStream(temporary,FileMode.Create,FileAccess.ReadWrite)) using(var w=new BinaryWriter(cache)) {
                    uint dataStart=checked(52+(uint)entries.Count*16); cache.Position=dataStart;
                    int done=0;
                    foreach(var entry in entries) {
                        if(cache.Position+entry.Size>Int32.MaxValue)throw new InvalidDataException("Expanded cache exceeds 2 GiB");
                        entry.Offset=(uint)cache.Position;
                        if(kind==2) { source.Position=entry.Source; entry.CRC=Copy(source,cache,entry.Size); }
                        else using(var input=entry.Zip.Open()) { if(Copy(input,cache,entry.Size)!=entry.CRC)throw new InvalidDataException("ZIP resource CRC mismatch"); }
                        if(++done%2000==0)Console.WriteLine("Prepared {0}/{1} resources",done,entries.Count);
                    }
                    byte[] table;
                    using(var m=new MemoryStream())using(var tw=new BinaryWriter(m)) {
                        foreach(var entry in entries){tw.Write(entry.Key);tw.Write(entry.Size);tw.Write(entry.Offset);tw.Write(entry.CRC);}
                        tw.Flush();table=m.ToArray();
                    }
                    cache.Position=0;w.Write(System.Text.Encoding.ASCII.GetBytes("Z3DRC001"));w.Write(md5.ComputeHash(directory));
                    w.Write((uint)source.Length);w.Write(kind);w.Write((uint)entries.Count);w.Write(md5.ComputeHash(table));w.Write(table);w.Flush();
                }
                if(File.Exists(output))File.Delete(output);
                File.Move(temporary,output);
                Console.WriteLine("Prepared all {0} resources: {1}",entries.Count,output);
                return kind;
            } finally { if(zip!=null)zip.Dispose(); if(File.Exists(temporary))File.Delete(temporary); }
        }
    }
}
'@
if ($PSVersionTable.PSVersion.Major -ge 6) { Add-Type -TypeDefinition $definitions }
else { Add-Type -ReferencedAssemblies System,System.Core,System.IO.Compression,System.IO.Compression.FileSystem -TypeDefinition $definitions }
$kind=[ResourceCacheBuilder]::Build($Archive,$Output)
if ($kind -eq 1) {
    $textureOutput = if ($Output.EndsWith('.3ds-resources')) { $Output.Substring(0,$Output.Length-14)+'.3ds-cache' } else { $Output+'.3ds-cache' }
    & (Join-Path $PSScriptRoot 'Build-TextureCache.ps1') -Archive $Archive -Output $textureOutput
}
Write-Host 'Copy the original archive and its .3ds-resources (and .3ds-cache if created) together onto SD.'
