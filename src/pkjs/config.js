/*
 * Clay configuration page.
 *
 * Clay is Pebble's general settings-page framework (works for both
 * watchfaces and watchapps). On save, Clay sends each field's value to the
 * watch via AppMessage using its messageKey.
 */

module.exports = [
  {
    "type": "heading",
    "defaultValue": "Loader Barcode"
  },
  {
    "type": "text",
    "defaultValue":
      "Set the barcode content and format. It is saved on the watch — no need to re-enter."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Barcode"
      },
      {
        "type": "input",
        "messageKey": "BARCODE_DATA",
        "label": "Content",
        "defaultValue": "",
        "attributes": {
          "placeholder": "e.g. HELLO123",
          "limit": 40,
          "autocapitalize": "characters"
        }
      },
      {
        "type": "select",
        "messageKey": "BARCODE_FORMAT",
        "label": "Format",
        "defaultValue": "0",
        "options": [
          { "label": "Code 39 Std", "value": "0" },
          { "label": "Code 128", "value": "1" },
          { "label": "QR Code", "value": "2" }
        ]
      },
      {
        "type": "toggle",
        "messageKey": "BARCODE_TEXT_VISIBLE",
        "label": "Show text",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "text",
    "defaultValue":
      "On the watch: UP / DOWN rotates 90°, SELECT toggles the text."
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
];
