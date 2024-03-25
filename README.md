**Raspberry Pi Picoを使用したIchigoJamの移植**

# 環境構築
https://datasheets.raspberrypi.com/pico/getting-started-with-pico-JP.pdf  
LINUXでの操作説明？なので、一部参考にならないコマンドがある  

以下プロジェクトのディレクトリ名をraspberry_piとして説明を進める  
このプロジェクトをクローンすれば、pico-sdk等のライブラリのクローンは不要  
第2章2.2の通り、CmakeとGCCは、なければインストールする  
IchigoJamの移植プログラムをコンパイルするには、raspberry_pi直下にbuildディレクトリを作成し、第3章と同様に進める  

以下のパスを通す必要があるので、~/.zshrcに絶対パスで保存推奨  
export PICO_SDK_PATH=hoge/raspberry_pi/pico-sdk  
export PICO_EXTRAS_PATH=hoge/raspberry_pi/pico-extras  

uf2ファイルが作成できたらピコに書き込む  
BOOTSELボタンを押しながらパソコンとピコをUSBで繋いだら、uf2ファイルを書き込める状態になる  
コマンドを使わなくても、ファインダーでRPI-RP2に放り込めば書き込める  

# ブレッドボード上でのIchigoJamの回路  
//TODO 後で書く  

# ディレクトリの説明
IchigoJam.selected  IchigoJam.R.srcから必要なファイルだけ抜粋(原則元ファイルから変更しない)  
IchigoJamR.src  実際のIchigoJamで使われているソース  
pico-example  pico-sdkの使用例  
pico-extras  pico-sdkに含まれない開発中のライブラリ(ディープスリープ実装のために使用)  
pico-playground  pico-extrasの使用例  
pico-sdk  ピコ用のライブラリ  
PicoDVI  DVI出力のためのライブラリ、オリジナルにDVIをstopできる機能を追加したものを使用、Mar 12,2023 579eeccのコミットを使用(これより先のコミットだとうまく動かない)  

# 参考プログラム
pico-examples/uart/uart_advanced/uart_advanced.c  
pico-examples/usb/host/host_cdc_msc_hid/main.c  

pico-playground/sleep/hello_dormant/hello_dormant.c  
pico-playground/sleep/hello_sleep/hello_sleep.c  

PicoDVI/software/apps/mandelbrot/main.c  

IchigoJamR.src/main.c　　

# 参考URL
## PICO-SDKリファレンス
https://www.raspberrypi.com/documentation/pico-sdk/hardware.html  
https://www.raspberrypi.com/documentation/pico-sdk/high_level.html  

## DVI関連
https://qiita.com/riris4488/items/252949e16af73e9fbcdb  
https://learn.adafruit.com/adafruit-dvi-breakout-board/raspberry-pi-pico-demo-code  

## フラッシュメモリ関連
https://www.prototype00.com/2022/07/raspberry-pi-picoflash-cc.html  
https://decafish.blog.ss-blog.jp/2021-05-30  
