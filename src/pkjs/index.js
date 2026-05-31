/*
 * PebbleKit JS 進入點。
 *
 * 透過 pebble-clay 自動處理設定頁面：
 *   - 顯示設定畫面（showConfiguration）
 *   - 在使用者關閉設定頁時（webviewclosed），自動把設定值依各欄位的
 *     messageKey 透過 AppMessage 傳到手錶端。
 *
 * Clay 也會把設定值儲存在手機端的 localStorage，因此再次開啟設定頁
 * 時會記住上次的內容。
 */

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);
