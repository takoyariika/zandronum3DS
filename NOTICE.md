# 著作権とソースの案内

このプロジェクトはZandronum 3.2.1の3DS向け移植です。Zandronum公式・Nintendo公式の製品ではありません。

`LICENSE` は元のZandronumのライセンスです。すべてのファイルを一つのライセンスへ変更するものではありません。ZDoom、Doom由来のコード、各ライブラリー、3DS参考実装にはそれぞれの通知・条件があります。各ファイルのヘッダー、`upstream/docs/`、`dependencies/` のライセンスも参照してください。

- エンジン: `upstream/LICENSE.txt`、`upstream/docs/`
- 音声実装・Tremor・libogg・dr_libs等: `dependencies/AUDIO_LICENSES.txt` と各ソース
- TLS: `dependencies/mbedtls-3.6.7/LICENSE`
- SQLite・zstd等: `dependencies/` と `upstream/` 内の各ライセンス
- DSP抽出実装: `dependencies/dsp1-LICENSE.txt`、`dependencies/ctrtool-LICENSE.txt`
- 出典・固定リビジョン: `sources.lock.json`

バイナリーを公開するときは、同じReleaseへ対応する完全なソースZIPも添付してください。配布ZIPの `licenses/` を削除しないでください。SDK／ツールチェーンは別途devkitProから入手します。

元ゲームのIWAD、有料ゲームデータ、第三者MOD、Nintendoファームウェア、個人の設定・セーブ・ログはこの公開パッケージに含みません。実行には利用者が自分のIWADを用意します。
