# GivetakeJam BASIC
![Platform](https://img.shields.io/badge/platform-Raspberry%20Pi%20Pico-blue)
![Language](https://img.shields.io/badge/language-BASIC-orange)
![License](https://img.shields.io/badge/license-IchigoJam-green)
![Version](https://img.shields.io/badge/version-v1.6.1-brightgreen)
![Status](https://img.shields.io/badge/status-stable-success)

##  プログラム容量の拡張、配列変数の拡張、外部EEPROM対応の改善、NEC方式の赤外線受信コマンド、環境測定コマンドの追加を行い、従来のIchigoJam P BASICの互換性を維持しながら機能強化しています。

## 機能 / 変更点
- プログラム容量を 1024 → 4096バイト に拡張

- 内部保存プログラム本数を 100 → 25 に変更

- 配列変数を拡張：
  - オリジナル：[0] ～ [101]
  - 追加した変数：[102] ～ [357]
    - #C00 に VAR2 追加

- 配列関連の不具合修正：
  - FOR/NEXT 動作を修正
  - CLV / CLEAR で拡張配列領域も初期化されるよう修正
  - 間接参照 [[x]] に対応

- FILESコマンド改善：
  - FILES → 0～24
  - FILES0 → 外部EEPROMがあれば 100～131
  - FILES n → 0～n（25～99はスキップ）

- 外部EEPROM 対応の改善
  - 外部EEPROM検出を I2Cアドレス 0x50 のACKで実装
  - 24LC64 / 24LC256 / 24FC1025 に単一バイナリで対応
  - EEPROM書き込みを32バイト分割にして安全性向上

- LIST エリアを #E00 に移動

- メモリマップ更新：
  - #000 CHAR
  - #700 PCG
  - #800 VAR
  - #900 VRAM
  - #C00 VAR2
  - #E00 LIST（4096バイト）

- HX1838系赤外線リモコン受信モジュール向け NEC方式赤外線受信コマンド IR.IN を追加
  - 書式
    ```sh
      IR.IN port,[n]
  - 説明
    - 指定した入力ポートから NEC形式の赤外線信号を受信し、デコード結果を [n] から始まる配列変数に格納します。
  - 結果の格納先
    - [n+0] = 生データ 1 バイト目
    - [n+1] = 生データ 2 バイト目
    - [n+2] = 生データ 3 バイト目
    - [n+3] = 生データ 4 バイト目
    - [n+4] = repeat フラグ
    - [n+5] = エラー コード
    - [n+6] = モード
      - 0 = NEC 標準
      - 1 = NEC 拡張
  - 使用例 プログラム IR_IN_TEST.BAS
  - 使用例 表示結果 807F01FE
  - 注意
    - HX1838の出力は待機時 HIGH を前提としています
    - 38kHz搬送波は受光モジュール側で復調済みです
    - 安定動作のため、エラー除外と repeat除外を推奨します
    - リーダーコード検出後の NECデコード区間では、安定化のため割り込みの影響を抑えています

- AHT20とBMP280モジュールを使って温湿度と気圧を測定するコマンド ENV.IN を追加
  - 書式
    ```sh
      ENV.IN [n]
  - 説明
    - I2CラインにつないだAHT20とBMP280から 測定データを受信し、デコード結果を [n] から始まる配列変数に格納します。
  - 結果の格納先
    - [n+0] = 温度 (AHT20, 0.1℃単位)
    - [n+1] = 湿度 (AHT20, 0.1%RH単位)
    - [n+2] = 気圧 (BMP280, 0.1hPa単位)
    - [n+3] = 温度 (BMP280, 0.1℃単位)
    - [n+4] = エラー コード
      - 0 = 正常
      - 1 = AHT20 が見つからない
      - 2 = BMP280 が見つからない（※致命エラーにはしない）
      - 3 = AHT20 測定失敗
      - 4 = BMP280 読み出し失敗
      - 5 = BMP280 チップID不正
    - [n+5] = 状態フラグ
      - bit0 = AHT20 正常
      - bit1 = BMP280 正常
  - 使用例 プログラム ENV_IN_TEST.BAS

- 拡張版識別のため VER() を 16114 に変更
  - 使用例 BASIC
    ```sh 
     ? VER()
     16114
    ```

---

#  IchigoJam P BASICのソースコードを変更して４K版にしたものです。よって、まずはIchigoJam P BASICがコンパイルできる環境を作成します。

## 環境構築
- "CMake"と"GCC"をインストールする  

## IchigoJam_P のコンパイル環境 
　　-  IchigoJam_P のディレクトリーで作成します。

- 以下のライブラリを`IchigoJam_P`ディレクトリ直下に置く  
    - "IchigoJam_BASIC": 1つ上の階層にある`IchigoJam_BASIC`ディレクトリをコピーする  
    - "pico-sdk": https://github.com/NaturalStyle/pico-sdk.git `196662b`のコミットを使用(別のコミットを使用すると正しく動作しない)  
    - "pico-extras": https://github.com/raspberrypi/pico-extras.git 開発時は`09c64d5`のコミットを使用  
    - "PicoDVI": https://github.com/mlorenzati/PicoDVI.git `579eecc`のコミットを使用(別のコミットを使用すると正しく動作しない)  

- 以下のパスを通す  
    ```sh
    PICO_SDK_PATH=foo/IchigoJam_P/pico-sdk  
    PICO_EXTRAS_PATH=foo/IchigoJam_P/pico-extras  
    ```

詳細は以下を参照  
[Raspberry Pi Picoをセットアップしよう](https://datasheets.raspberrypi.com/pico/getting-started-with-pico-JP.pdf)  

## ビルド
```sh
cd IchigoJam_P  
mkdir build  
cd build  
cmake ..  
make  
```

`IchigoJam_P.uf2`ファイルが作成できたらpicoに書き込む  

## 動作がおかしくなったら
`build`ディレクトリを消して作り直してみる  
特に映像の出力が正常に行われなくなることが多い  

## TODO
高解像度に対応する(TVでも使えるようにするため)  


## ４K版にするために GivetakeJam_P のディレクトリーにある以下のファイルを IchigoJam_P の各のディレクトリーに上書きコピーします。
    - "IchigoJam_BASIC": basic.h , ram.h , tokens_v1.5.h
    - "src": config.h , i2ceeprom.h , storage.h , io.h
    - IchigoJam P の時の様にビルドして、`IchigoJam_P.uf2`ファイルが作成できたらpicoに書き込みます。
    - IchigoJam_P.uf2: 4K版のファームウェアファイルです。
    - チェックサム
      SHA-256: 61860e5e6818e80bd31dc70909b0e5b500e3cb495ddb19fb6f2698dc40066e32
      MD5: 2ffb09348083811a0f3c3518ba52da45
      SHA-1: 2d34932ca879a46af408d8daa54a3b9b6a9114cf
    
    - ARRAY_VAR_TOTAL_TEST.BAS: 配列変数のテストプログラムです。このテストが ALL OK で通ることを確認しています。

## Screenshot

### 動作環境
![screenshot](./docs/IMG_2095up.png)

### 表示画面 ( 名称, HELP表示：メモリマップ, FILES表示 )
![screenshot](./docs/IMG_2159up.png)

### IchigoJam_P 互換基板 ( MECHTARIAT PJ 凸 )
![screenshot](./docs/IMG_2168up.png)

## ライセンス / license
    - Copyright 2014-2025 the IchigoJam authors. All rights reserved. MIT license.  https://ichigojam.net/license

## 商標 / Trademark
    - IchigoJamは、株式会社jig.jpの登録商標です。(IchigoJam is a registered trademark of jig.jp co., ltd.) 
    - Raspberry Pi は、Raspberry Pi財団の登録商標です。(Raspberry Pi is a registered trademark of the Raspberry Pi Foundation)  

## 利用規約に同意の上、ダウンロードしてください。第三者への配布にはライセンスが必要になります。詳しくはこちら。https://pcn.club/sp/ijp/
    - 利用規約（IchigoJamロイヤリティフリープログラム利用規約） https://ichigojam.net/ichigojam-license.pdf

## 既知の制限 / 注意事項
- プログラム最大 4096 バイト
- 内部保存プログラム本数は 25本
- 外部EEPROM表示は 100～131固定
  - 24LC64 追加本数 2本
  - 24LC256 追加本数 8本
  - 24FC1025 追加本数 32本 
- 実際の使用可能プログラム本数はEEPROM容量依存
- EEPROM検出はI2C アドレス 0x50 の応答で判定
- IR受信を安定させるため、BASIC側で repeat 除外を推奨します

ギブテクウインウイン
2026/5/29 作成  
![Release](https://img.shields.io/github/v/release/IchigoJam/ichigojam-firm)
![Downloads](https://img.shields.io/github/downloads/IchigoJam/ichigojam-firm/total)
