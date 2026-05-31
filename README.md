# loader-barcode

Pebble 條碼 App，支援一維與二維條碼。

使用 [Clay](https://github.com/pebble/clay) 設定頁面設定要顯示的條碼。內建自行撰寫的一維編碼引擎，預設為 **Code 39 Std**，另支援 **Code 128**；二維條碼支援 **QR Code**。條碼設定完成後會儲存在手錶記憶體中常駐，無須反覆設定；顯示時可用按鍵旋轉方向。

## 功能

- **Clay 設定頁面**：在手機 Pebble App 的 App 設定畫面輸入條碼內容、選擇編碼格式。
  - Clay 為 Pebble 通用的設定頁面框架，watchface 與 watchapp 皆可使用（不限定於錶盤）。本 App 即為一般 watchapp。
- **編碼引擎**：
  - **Code 39 Std**（預設，`src/c/main.c`）：標準 Code 39，支援 `0-9`、`A-Z`、空白與 `- . $ / + %`，自動加上 `*` start/stop 字元。
  - **Code 128**（`src/c/main.c`）：支援可見 ASCII（Code B）；偶數長度的純數字會自動使用較精簡的 Code C，並計算 mod-103 校驗碼。
  - **QR Code**（`src/c/qrcodegen.*`）：採用 Project Nayuki 公有領域 qrcodegen 函式庫，依內容自動選擇數字 / 英數 / byte 模式與最佳遮罩，含 Reed-Solomon 錯誤更正。
- **記憶體常駐**：條碼內容、格式、旋轉方向皆存於 persistent storage，開啟 App 即直接顯示，無須重新設定。
- **旋轉**：顯示條碼時按 **上 / 下** 鍵可將條碼旋轉 90°（共 4 個方向），一維與二維皆支援。

## 操作

| 按鍵 | 功能 |
| ---- | ---- |
| 上   | 逆向旋轉 90° |
| 下   | 順向旋轉 90° |
| 返回 | 離開 App |

設定方式：手機 Pebble App → 找到 **Loader Barcode** → 點齒輪/設定 → 輸入內容並儲存。

## 建置

需要 [Pebble SDK](https://github.com/pebble-dev/wiki/wiki) / Rebble 工具鏈：

```sh
pebble build
pebble install --emulator basalt   # 或安裝到實機
```

支援平台：aplite、basalt、chalk、diorite、emery。

## 專案結構

```
package.json        # 專案設定、messageKeys、Clay 相依
src/c/main.c        # 手錶端 App 與一維編碼引擎、繪圖、旋轉、持久化
src/c/qrcodegen.c   # QR Code 編碼引擎（Project Nayuki，公有領域）
src/c/qrcodegen.h
src/pkjs/index.js   # PebbleKit JS 進入點，初始化 Clay
src/pkjs/config.js  # Clay 設定頁面欄位定義
```
