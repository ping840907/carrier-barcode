# loader-barcode

Pebble 條碼 App，支援一維與二維條碼。

使用 [Clay](https://www.npmjs.com/package/@rebble/clay)（`@rebble/clay`）設定頁面設定要顯示的條碼。內建自行撰寫的一維編碼引擎，預設為 **Code 39 Std**，另支援 **Code 128**；二維條碼支援 **QR Code**。條碼設定完成後會儲存在手錶記憶體中常駐，無須反覆設定；顯示時可用按鍵旋轉方向。

## 功能

- **Clay 設定頁面**：在手機 Pebble App 的 App 設定畫面輸入條碼內容、選擇編碼格式。
  - Clay 為 Pebble 通用的設定頁面框架，watchface 與 watchapp 皆可使用（不限定於錶盤）。本 App 即為一般 watchapp。
- **編碼引擎**：
  - **Code 39 Std**（預設，`src/c/main.c`）：標準 Code 39，支援 `0-9`、`A-Z`、空白與 `- . $ / + %`，自動加上 `*` start/stop 字元。
  - **Code 128**（`src/c/main.c`）：支援可見 ASCII（Code B）；偶數長度的純數字會自動使用較精簡的 Code C，並計算 mod-103 校驗碼。
  - **QR Code**（`src/c/qrcodegen.*`）：採用 Project Nayuki 公有領域 qrcodegen 函式庫，依內容自動選擇數字 / 英數 / byte 模式與最佳遮罩，含 Reed-Solomon 錯誤更正。
- **自動延展**：條碼會自動依當前畫面邊界延展鋪滿可用空間；切換方向時也會自動適應該方向的邊界長度（不需手動調整）。
- **長度偵測與降級顯示**：當條碼長度超出畫面邊界時，會依長度自動逐步降級，確保畫面上的條碼始終可掃描：
  1. **直向放得下** → 正常顯示，可自由切換方向。
  2. **直向放不下** → 主動禁止直向，強制改用較長的橫向（順時鐘 90°）；此時上 / 下鍵不會切回直向。
  3. **橫向含文字仍放不下** → 自動隱藏底部人眼可讀文字，讓條碼用滿全螢幕。
  4. **全螢幕橫向仍放不下** → 顯示指示畫面（"Too Long"），提醒使用者縮短內容或改用其他編碼方式。
  - QR Code 為正方形、方向不影響容量，故僅在「含文字 → 隱藏文字 → 顯示指示畫面」之間降級。
- **記憶體常駐**：條碼內容、格式、方向皆存於 persistent storage，開啟 App 即直接顯示，無須重新設定。
- **方向切換**：顯示條碼時按 **上 / 下** 鍵可在「正面」與「順時鐘 90°」之間切換，一維與二維皆支援。

## 操作

| 按鍵 | 功能 |
| ---- | ---- |
| 上 / 下 | 在正面與順時鐘 90° 之間切換方向 |
| 選擇 (SELECT) | 切換是否顯示底部文字 |
| 返回 | 離開 App |

文字顯示與否也可在 Clay 設定頁的「顯示文字」開關設定;兩處皆會持久化。

設定方式：手機 Pebble App → 找到 **Loader Barcode** → 點齒輪/設定 → 輸入內容並儲存。

## 建置

需要 [Pebble SDK](https://github.com/pebble-dev/wiki/wiki) / Rebble 工具鏈：

```sh
pebble build
pebble install --emulator basalt   # 或安裝到實機
```

支援平台：aplite、basalt、chalk、diorite、emery、flint（Pebble 2 Duo）、gabbro（Pebble Round 2）。

## 專案結構

```
package.json        # 專案設定、messageKeys、Clay 相依
src/c/main.c        # 手錶端 App 與一維編碼引擎、繪圖、旋轉、持久化
src/c/qrcodegen.c   # QR Code 編碼引擎（Project Nayuki，MIT License）
src/c/qrcodegen.h
src/pkjs/index.js   # PebbleKit JS 進入點，初始化 Clay
src/pkjs/config.js  # Clay 設定頁面欄位定義
```

## 致謝 / Credits

本專案的 **QR Code 編碼引擎**（`src/c/qrcodegen.c`、`src/c/qrcodegen.h`）改用自
[Project Nayuki 的 QR Code generator library](https://www.nayuki.io/page/qr-code-generator-library)
（[GitHub: nayuki/QR-Code-generator](https://github.com/nayuki/QR-Code-generator)），以 **MIT License** 釋出。
一維條碼引擎（Code 39 / Code 128）與其餘程式為本專案自行撰寫。

設定頁面使用 [`@rebble/clay`](https://www.npmjs.com/package/@rebble/clay)（Rebble 社群維護）。

### QR Code generator library — MIT License

```
QR Code generator library (C)

Copyright (c) Project Nayuki. (MIT License)
https://www.nayuki.io/page/qr-code-generator-library

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
- The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
- The Software is provided "as is", without warranty of any kind, express or
  implied, including but not limited to the warranties of merchantability,
  fitness for a particular purpose and noninfringement. In no event shall the
  authors or copyright holders be liable for any claim, damages or other
  liability, whether in an action of contract, tort or otherwise, arising from,
  out of or in connection with the Software or the use or other dealings in the
  Software.
```
