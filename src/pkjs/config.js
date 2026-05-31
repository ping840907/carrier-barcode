/*
 * Clay 設定頁面設定檔。
 *
 * Clay 是 Pebble 通用的設定頁面框架，watchface 與 watchapp 皆可使用
 * （並不限定於錶盤）。此檔案描述設定畫面上的欄位，Clay 會在使用者
 * 按下「儲存」後，自動將各欄位的值依 messageKey 透過 AppMessage
 * 傳送到手錶端。
 */

module.exports = [
  {
    "type": "heading",
    "defaultValue": "Loader Barcode"
  },
  {
    "type": "text",
    "defaultValue":
      "設定要顯示的一維條碼內容與編碼格式。" +
      "設定完成後條碼會儲存在手錶記憶體中，下次開啟 App 會直接顯示，無須重新設定。" +
      "<br/><br/>顯示條碼時，按「上 / 下」鍵可將條碼旋轉 90°（共 4 個方向），按「選擇 (SELECT)」鍵可切換是否顯示文字。"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "條碼設定"
      },
      {
        "type": "input",
        "messageKey": "BARCODE_DATA",
        "label": "條碼內容",
        "defaultValue": "",
        "attributes": {
          "placeholder": "例如 HELLO123",
          "limit": 40,
          "autocapitalize": "characters"
        }
      },
      {
        "type": "select",
        "messageKey": "BARCODE_FORMAT",
        "label": "編碼格式",
        "defaultValue": "0",
        "options": [
          { "label": "Code 39 Std (預設)", "value": "0" },
          { "label": "Code 128", "value": "1" },
          { "label": "QR Code", "value": "2" }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "BARCODE_TEXT_VISIBLE",
        "label": "顯示文字",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "text",
    "defaultValue":
      "提示：Code 39 Std 支援 0-9、A-Z、空白與 - . $ / + %。" +
      "Code 128 支援可見 ASCII 字元（偶數長度純數字會自動使用較精簡的 Code C）。" +
      "QR Code 可存放任意文字（數字 / 英數 / 一般文字會自動選擇最佳模式）。"
  },
  {
    "type": "submit",
    "defaultValue": "儲存並傳送到手錶"
  }
];
