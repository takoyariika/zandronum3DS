"""Assemble GitHub repository sources and matching Release assets; never upload."""
import hashlib
import json
import shutil
import zipfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
VERSION='0.1.18'
OUT=ROOT/'build-3ds/github-release'
REPO=OUT/'repository'
ASSETS=OUT/'release-assets'
REPO.mkdir(parents=True,exist_ok=True); ASSETS.mkdir(parents=True,exist_ok=True)
# Refuse stale trees rather than silently retaining a previous release's files.
if any(REPO.iterdir()) or any(ASSETS.iterdir()):
    raise SystemExit('Output already populated. Rename build-3ds/github-release before regenerating.')
excluded_dirs={'.git','__pycache__','.pytest_cache','.vs','.idea'}
excluded_suffixes={'.exe','.dll','.o','.obj','.a','.lib','.elf','.cia','.3dsx','.pyc','.pyo','.log','.part'}
for folder in ('upstream','platform','tools','dependencies','reference-generated'):
    for file in sorted((ROOT/folder).rglob('*')):
        rel=file.relative_to(ROOT)
        if not file.is_file() or file.is_symlink() or excluded_dirs.intersection(rel.parts) or file.suffix.lower() in excluded_suffixes:
            continue
        target=REPO/rel; target.parent.mkdir(parents=True,exist_ok=True); shutil.copy2(file,target)
for name in ('README.md','BUILDING.md','NOTICE.md','LICENSE','RELEASE_NOTES.md','sources.lock.json','.gitignore','.gitattributes'):
    shutil.copy2(ROOT/name,REPO/name)
# Internal engineering journal contains local paths and personal test details.
(REPO/'PORT_STATUS.md').write_text((ROOT/'RELEASE_NOTES.md').read_text(encoding='utf-8'),encoding='utf-8')
shutil.copy2(ROOT/'build-3ds/zandronum-3ds.patch',REPO/'zandronum-3ds.patch')
runtime_in=ROOT/'build-3ds/dist'/f'zandronum-3ds-alpha-{VERSION}.zip'
runtime=ASSETS/runtime_in.name
source=ASSETS/f'zandronum-3ds-source-{VERSION}.zip'
source_note=f'Corresponding complete source: {source.name}, attached to the same Release.\nBuild: BUILDING.md inside the source archive. Keep source and binaries together.\n'
with zipfile.ZipFile(runtime_in) as old, zipfile.ZipFile(runtime,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as new:
    for item in old.infolist(): new.writestr(item,old.read(item.filename))
    new.writestr('SOURCE.txt',source_note)
    for name in ('NOTICE.md','LICENSE'):
        new.write(ROOT/name,name)
    for folder in ('dependencies','upstream'):
        for file in sorted((ROOT/folder).rglob('*')):
            if not file.is_file() or '.git' in file.parts: continue
            name=file.name.lower()
            if name.startswith(('license','copying','copyright','notice')):
                new.write(file,'licenses/third-party/'+file.relative_to(ROOT).as_posix())
with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    for file in sorted(REPO.rglob('*')):
        if file.is_file(): z.write(file,file.relative_to(REPO).as_posix())
shutil.copy2(ROOT/'RELEASE_NOTES.md',ASSETS/'RELEASE_NOTES.md')
instructions='''GitHubへの公開手順

1. repository/（まとめZIPではrelease-assets/のソースZIPを展開）の中身を
   GitHubリポジトリへ追加します（ZIP自体ではなく展開したファイル）。
   .gitignoreも含めます。build-3dsや元ゲーム、MOD、設定・ログは追加しません。
2. タグ v0.1.18 のReleaseを作成し、release-assets/RELEASE_NOTES.mdを本文に使用します。
3. release-assets/ の配布ZIP・対応ソースZIP・SHA256SUMS.txtを同じReleaseへ添付します。
   配布ZIPにはCIAと3DSX、エンジンデータとキャッシュ、ツールとライセンスが入っています。
4. ソースは修正済みエンジン全体を含みます。GitHubの自動Source code ZIPを使う場合も、
   アップロード済みのリポジトリが対応する全ソースであることを確認してください。

UPLOAD-0.1.18.zipはこの一式を持ち運ぶためのまとめです。
GitHubへ自動アップロード・リポジトリ作成・タグ作成は行っていません。
'''
(OUT/'UPLOAD.md').write_text(instructions,encoding='utf-8')
checksums=[]
for file in (runtime,source,ASSETS/'RELEASE_NOTES.md'):
    checksums.append(hashlib.sha256(file.read_bytes()).hexdigest()+'  '+file.name)
(ASSETS/'SHA256SUMS.txt').write_text('\n'.join(checksums)+'\n',encoding='ascii')
bundle=OUT/f'UPLOAD-{VERSION}.zip'
with zipfile.ZipFile(bundle,'w',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
    z.write(OUT/'UPLOAD.md','UPLOAD.md')
    for file in sorted(ASSETS.iterdir()): z.write(file,'release-assets/'+file.name)
print(OUT)
print(bundle)
