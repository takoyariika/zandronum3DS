Zandronum 3DS cache tools (Windows PowerShell 5.1 or later)

Drag one or more WAD/PK3/ZIP files onto Build-ResourceCache.cmd.
Keep the four CMD/PS1 files together when extracting this tools package.

WAD (IWAD/PWAD): creates filename.wad.3ds-resources for all lumps.
PK3/ZIP: creates .3ds-resources and .3ds-cache for PNG registration.
Build-TextureCache.cmd creates only PNG headers for PK3/ZIP; WAD input
automatically uses the all-resource WAD builder instead.

Copy the original and generated sidecar files to the same SD directory.
Example:
  SD:/3ds/zandronum/DOOM2.WAD
  SD:/3ds/zandronum/DOOM2.WAD.3ds-resources
Keep downloads and their caches together in their downloads directory too.
Select only the original WAD/PK3 in the launcher. Do not select cache files
as MODs or add them to args.txt. Regenerate after modifying an original.

Original files are preserved. WADs are already uncompressed, so resource
caches take additional SD space comparable to their lump data. Speed gains
depend on the content; these tools do not reduce gameplay memory usage.
ZIP64, encrypted ZIPs and console-compressed WADs are unsupported.
This tools update works with the existing 0.1.16 CIA; no reinstall needed.
