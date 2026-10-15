# Windowsでのビルド

このソース一式には、修正済みZandronum 3.2.1、3DS実装、依存ライブラリーのソース、生成済みXLATパーサーとCIA設定が含まれます。参考リポジトリを別途取得したり、WAD/MODを用意したりする必要はありません。各ライブラリーのライセンスは元の場所に保持しています。

必要なもの:

- Python 3.10以降。ビルド自体は標準ライブラリーのみ使用します。
- Windows用devkitPro/devkitARM、libctru、3dsxtool、smdhtool。検証済みコンパイラーはdevkitARM GCC 16.1.0です。
- ホスト用re2c。スキャナーを生成します。
- CIAを作る場合はmakerom 0.18.3のWindows版。入手先とアーカイブのSHA256は `sources.lock.json` にあります。実行ファイルは公開ソースに同梱しません。
- キャッシュ作成にWindows PowerShell 5.1以降。

PowerShellでソースのルートから実行します。パスは自分の環境に合わせて変更してください。

```powershell
python tools/prepare_build.py --re2c re2c --makerom C:/tools/makerom.exe
python tools/arm_compile.py --devkitpro C:/devkitPro --jobs 6
python tools/build_game.py --jobs 6
python tools/build_cia.py
python tools/package_sd.py
powershell -NoProfile -ExecutionPolicy Bypass -File tools/Build-ResourceCache.ps1 -Archive build-3ds/sdmc/3ds/zandronum/zandronum.pk3
python tools/create_release.py
```

Gitの管理情報がないソースZIPでも、エンジンのリビジョンは `sources.lock.json` から取得します。Git作業ツリーとして `upstream/.git` を保持している場合のみGitが必要です。`GIT` 環境変数（配布生成）または `arm_compile.py --git` で使用するGitを指定できます。

結果は `build-3ds/dist/` に出ます。ヘッダーを変更した場合は `arm_compile.py` を全体実行してください。`--filter` は調査専用で、配布用ビルドには使用しません。SDKやコンパイラーの版が異なる場合、同じバイト列になることは保証しません。

`package_sd.py --iwad 自分のWAD --byoc 自分のMOD` は手元の動作確認用です。このオプションで作るSDツリー全体は配布しないでください。`create_release.py` は公開するエンジンデータを明示的に選び、個人WAD/MODを含めません。

テスト用スクリプトには、Citra・デスクトップ版Zandronum・各自のゲームデータが別途必要です。PillowはLCDキャプチャーの変換／検証にのみ使用します。生成済みファイルを上書きする初期インポート用 `prepare_audio_backend.py` などは、通常のビルドでは実行しません。
