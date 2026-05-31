#include <pebble.h>
#include <stdlib.h>   /* atoi */
#include <string.h>   /* strncpy */
#include "qrcodegen.h"

/* ------------------------------------------------------------------------- *
 *  Loader Barcode
 *
 *  顯示條碼的 Pebble App，支援：
 *    - 一維條碼：Code 39 Std（預設）、Code 128
 *    - 二維條碼：QR Code
 *
 *  使用 Clay 設定頁面設定條碼內容與格式。內容/格式/旋轉方向皆存於
 *  persistent storage，設定後常駐記憶體，無須反覆設定。
 *  顯示時按「上 / 下」鍵可旋轉條碼方向（每次 90°，共 4 個方向）。
 *
 *  一維編碼引擎（Code 39 / Code 128）為自行撰寫；QR Code 採用
 *  Project Nayuki 的公有領域 qrcodegen 函式庫（src/c/qrcodegen.*）。
 * ------------------------------------------------------------------------- */

/* ---- 編碼格式 ---- */
#define FORMAT_CODE39   0   /* Code 39 Std（標準 43 字元集 + * 起訖碼）*/
#define FORMAT_CODE128  1
#define FORMAT_QR       2

/* ---- persistent storage keys ---- */
#define PERSIST_DATA          1
#define PERSIST_FORMAT        2
#define PERSIST_ROTATION      3
#define PERSIST_TEXT_VISIBLE  4

/* ---- 參數 ---- */
#define MAX_DATA_LEN    41     /* 含結尾 '\0'，可放 40 個字元 */
#define MAX_MODULES     1400   /* 一維模組（最小單位寬度）緩衝區上限 */
#define CODE39_WIDE     2      /* Code 39 寬:窄 比例 */
#define TEXT_AREA_H     22     /* 畫面底部保留給人眼可讀文字的高度 */
#define QR_MAX_VERSION  10     /* 最大 QR 版本（57x57）；保持可掃描且省記憶體 */

/* ------------------------------------------------------------------------- *
 *  全域狀態
 * ------------------------------------------------------------------------- */
static Window  *s_window;
static Layer   *s_canvas_layer;

static char     s_data[MAX_DATA_LEN] = "";
static int      s_format   = FORMAT_CODE39;
static int      s_rotation = 0;            /* 0 = 正面, 1 = 順時鐘 90 度 */
static bool     s_text_visible = true;     /* 是否顯示底部人眼可讀文字 */

/* 一維條碼模組緩衝 */
static uint8_t  s_modules[MAX_MODULES];    /* 每個元素為 1（黑條）或 0（白）*/
static int      s_module_count = 0;

/* 二維 QR 緩衝 */
static uint8_t  s_qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_MAX_VERSION)];
static uint8_t  s_qrtemp[qrcodegen_BUFFER_LEN_FOR_VERSION(QR_MAX_VERSION)];
static bool     s_qr_ok = false;

/* ------------------------------------------------------------------------- *
 *  一維編碼引擎 ── 共用工具
 * ------------------------------------------------------------------------- */

static char to_upper(char c) {
  if (c >= 'a' && c <= 'z') return (char)(c - 'a' + 'A');
  return c;
}

static bool is_digit(char c) { return c >= '0' && c <= '9'; }

/* 依「寬度序列」往模組緩衝區寫入，色彩由 start_color 起，每段交替。*/
static void append_widths(const uint8_t *widths, int n, int start_color) {
  int color = start_color;
  for (int i = 0; i < n; i++) {
    for (int w = 0; w < widths[i]; w++) {
      if (s_module_count < MAX_MODULES) {
        s_modules[s_module_count++] = (uint8_t)color;
      }
    }
    color = !color;
  }
}

/* 寫入單一白色模組（字元間隔）*/
static void append_gap(void) {
  if (s_module_count < MAX_MODULES) s_modules[s_module_count++] = 0;
}

/* ------------------------------------------------------------------------- *
 *  Code 39 Std 編碼
 *
 *  每個字元由 9 個元素（5 bar + 4 space，交替，首尾為 bar）組成，其中
 *  恰有 3 個為「寬」元素。字元之間以一個窄空白分隔。整體格式為
 *  *  資料...  *（* 為 start/stop 字元）。標準（Std）Code 39 字元集為
 *  0-9、A-Z、空白與 - . $ / + %，不含 Full ASCII 擴充。
 * ------------------------------------------------------------------------- */

static const char *code39_pattern(char c) {
  switch (c) {
    case '0': return "nnnwwnwnn";
    case '1': return "wnnwnnnnw";
    case '2': return "nnwwnnnnw";
    case '3': return "wnwwnnnnn";
    case '4': return "nnnwwnnnw";
    case '5': return "wnnwwnnnn";
    case '6': return "nnwwwnnnn";
    case '7': return "nnnwnnwnw";
    case '8': return "wnnwnnwnn";
    case '9': return "nnwwnnwnn";
    case 'A': return "wnnnnwnnw";
    case 'B': return "nnwnnwnnw";
    case 'C': return "wnwnnwnnn";
    case 'D': return "nnnnwwnnw";
    case 'E': return "wnnnwwnnn";
    case 'F': return "nnwnwwnnn";
    case 'G': return "nnnnnwwnw";
    case 'H': return "wnnnnwwnn";
    case 'I': return "nnwnnwwnn";
    case 'J': return "nnnnwwwnn";
    case 'K': return "wnnnnnnww";
    case 'L': return "nnwnnnnww";
    case 'M': return "wnwnnnnwn";
    case 'N': return "nnnnwnnww";
    case 'O': return "wnnnwnnwn";
    case 'P': return "nnwnwnnwn";
    case 'Q': return "nnnnnnwww";
    case 'R': return "wnnnnnwwn";
    case 'S': return "nnwnnnwwn";
    case 'T': return "nnnnwnwwn";
    case 'U': return "wwnnnnnnw";
    case 'V': return "nwwnnnnnw";
    case 'W': return "wwwnnnnnn";
    case 'X': return "nwnnwnnnw";
    case 'Y': return "wwnnwnnnn";
    case 'Z': return "nwwnwnnnn";
    case '-': return "nwnnnnwnw";
    case '.': return "wwnnnnwnn";
    case ' ': return "nwwnnnwnn";
    case '$': return "nwnwnwnnn";
    case '/': return "nwnwnnnwn";
    case '+': return "nwnnnwnwn";
    case '%': return "nnnwnwnwn";
    case '*': return "nwnnwnwnn";   /* start / stop */
    default:  return NULL;
  }
}

static void code39_append_char(char c) {
  const char *p = code39_pattern(c);
  if (!p) return;
  uint8_t widths[9];
  for (int i = 0; i < 9; i++) {
    widths[i] = (p[i] == 'w') ? CODE39_WIDE : 1;
  }
  append_widths(widths, 9, 1 /* 首元素為 bar */);
}

static void encode_code39(const char *text) {
  code39_append_char('*');                 /* start */
  for (const char *c = text; *c; c++) {
    char u = to_upper(*c);
    if (!code39_pattern(u)) continue;       /* 略過不支援的字元 */
    append_gap();
    code39_append_char(u);
  }
  append_gap();
  code39_append_char('*');                  /* stop */
}

/* ------------------------------------------------------------------------- *
 *  Code 128 編碼（Code B / Code C 自動選擇，mod-103 校驗）
 * ------------------------------------------------------------------------- */

static const char *CODE128[] = {
  "212222","222122","222221","121223","121322","131222","122213","122312",
  "132212","221213","221312","231212","112232","122132","122231","113222",
  "123122","123221","223211","221132","221231","213212","223112","312131",
  "311222","321122","321221","312212","322112","322211","212123","212321",
  "232121","111323","131123","131321","112313","132113","132311","211313",
  "231113","231311","112133","112331","132131","113123","113321","133121",
  "313121","211331","231131","213113","213311","213131","311123","311321",
  "331121","312113","312311","332111","314111","221411","431111","111224",
  "111422","121124","121421","141122","141221","112214","112412","122114",
  "122411","142112","142211","241211","221114","413111","241112","134111",
  "111242","121142","121241","114212","124112","124211","411212","421112",
  "421211","212141","214121","412121","111143","111341","131141","114113",
  "114311","411113","411311","113141","114131","311141","411131","211412",
  "211214","211232","2331112"   /* 106 = STOP */
};

static void code128_append_value(int v) {
  const char *p = CODE128[v];
  int n = 0;
  uint8_t widths[8];
  for (const char *q = p; *q && n < 8; q++) {
    widths[n++] = (uint8_t)(*q - '0');
  }
  append_widths(widths, n, 1 /* 首元素為 bar */);
}

static void encode_code128(const char *text) {
  int len = 0;
  bool all_digits = true;
  for (const char *c = text; *c; c++) {
    len++;
    if (!is_digit(*c)) all_digits = false;
  }
  if (len == 0) return;

  long sum;
  int  pos = 1;

  if (all_digits && (len % 2 == 0)) {
    code128_append_value(105);            /* Start C */
    sum = 105;
    for (int i = 0; i < len; i += 2) {
      int v = (text[i] - '0') * 10 + (text[i + 1] - '0');
      code128_append_value(v);
      sum += (long)v * pos++;
    }
  } else {
    code128_append_value(104);            /* Start B */
    sum = 104;
    for (const char *c = text; *c; c++) {
      int ch = (unsigned char)*c;
      int v = (ch >= 32 && ch <= 127) ? (ch - 32) : 0;
      code128_append_value(v);
      sum += (long)v * pos++;
    }
  }

  int check = (int)(sum % 103);
  code128_append_value(check);            /* 校驗碼 */
  code128_append_value(106);              /* STOP */
}

/* ------------------------------------------------------------------------- *
 *  依目前 s_data / s_format 重建條碼緩衝區
 * ------------------------------------------------------------------------- */
static void rebuild_barcode(void) {
  s_module_count = 0;
  s_qr_ok = false;
  if (s_data[0] == '\0') return;

  if (s_format == FORMAT_QR) {
    s_qr_ok = qrcodegen_encodeText(s_data, s_qrtemp, s_qrcode,
        qrcodegen_Ecc_MEDIUM, qrcodegen_VERSION_MIN, QR_MAX_VERSION,
        qrcodegen_Mask_AUTO, true);
  } else if (s_format == FORMAT_CODE128) {
    encode_code128(s_data);
  } else {
    encode_code39(s_data);
  }
}

/* ------------------------------------------------------------------------- *
 *  繪圖
 * ------------------------------------------------------------------------- */

/* 繪製一維條碼。
 *
 * 自動依畫面邊界延展：
 *   - s_rotation == 0（正面）：模組沿「寬度」排列、長條沿「高度」延展。
 *   - s_rotation == 1（順時鐘 90 度）：模組沿「高度」排列、長條沿「寬度」延展。
 *
 * 模組寬度採「智慧填滿」：
 *   - 先用最大的「整數」模組寬度。若該寬度 >= 2px，各模組等寬（narrow:wide
 *     比例完全精確），白邊縮到最小且置中。
 *   - 若整數寬已是 1px（長條碼在寬螢幕上仍填不滿），則把剩餘像素以平均
 *     分散（Bresenham）方式分給部分模組，使其 +1px 補滿可用寬度。±1px 的
 *     抖動平均分布、平均比例仍 ~2:1，避免大片左右白邊（例如 emery 200px）。
 * 長條方向（厚度）一律填滿整個交叉邊。
 */
static void draw_linear(GContext *ctx, GRect content) {
  if (s_module_count <= 0) return;

  bool horiz = (s_rotation == 0);

  /* axis = 模組排列方向可用長度；cross = 長條延展方向可用長度 */
  int axis  = horiz ? content.size.w : content.size.h;
  int cross = horiz ? content.size.h : content.size.w;

  const int quiet = 6;   /* 排列方向兩端留白（quiet zone）*/
  const int cmarg = 6;   /* 延展方向兩側留白 */

  int draw_len = axis - 2 * quiet;
  if (draw_len < s_module_count) draw_len = axis;   /* 太擠就連 quiet zone 一起用 */

  int unit = draw_len / s_module_count;             /* 最大整數模組寬 */
  if (unit < 1) unit = 1;

  /* unit >= 2：各模組等寬，比例精確；unit == 1：以剩餘像素平均加寬補滿 */
  int total = (unit >= 2) ? unit * s_module_count : draw_len;
  int extra = (unit >= 2) ? 0 : (draw_len - s_module_count);
  if (extra < 0) extra = 0;

  int off = (axis - total) / 2;                     /* 置中 */
  if (off < 0) off = 0;

  int thick = cross - 2 * cmarg;                    /* 長條延展滿整個交叉邊 */
  if (thick < 1) thick = 1;

  graphics_context_set_fill_color(ctx, GColorBlack);
  int acc = 0;
  int pos = off;
  for (int i = 0; i < s_module_count; i++) {
    int w = unit;
    if (unit < 2) {                  /* 平均分散剩餘像素 */
      w = 1;
      acc += extra;
      if (acc >= s_module_count) { w += 1; acc -= s_module_count; }
    }
    if (s_modules[i]) {
      GRect r = horiz ? GRect(content.origin.x + pos,   content.origin.y + cmarg, w,     thick)
                      : GRect(content.origin.x + cmarg, content.origin.y + pos,   thick, w);
      graphics_fill_rect(ctx, r, 0, GCornerNone);
    }
    pos += w;
  }
}

/* 繪製二維 QR。
 *
 * 依可用空間（取寬高較小者）自動把每個模組放到最大整數像素，保留 quiet
 * zone 並置中。s_rotation == 1 時整個矩陣順時鐘旋轉 90 度。
 */
static void draw_qr(GContext *ctx, GRect content) {
  int n = qrcodegen_getSize(s_qrcode);
  int avail = (content.size.w < content.size.h) ? content.size.w : content.size.h;

  int unit = (avail - 12) / n;     /* 兩側各保留約 6px quiet zone */
  if (unit < 1) unit = 1;

  int total = unit * n;
  int ox = content.origin.x + (content.size.w - total) / 2;
  int oy = content.origin.y + (content.size.h - total) / 2;

  graphics_context_set_fill_color(ctx, GColorBlack);
  for (int my = 0; my < n; my++) {
    for (int mx = 0; mx < n; mx++) {
      if (!qrcodegen_getModule(s_qrcode, mx, my)) continue;
      int gx, gy;
      if (s_rotation == 1) {       /* 順時鐘 90 度 */
        gx = n - 1 - my; gy = mx;
      } else {                     /* 正面 */
        gx = mx;         gy = my;
      }
      GRect r = GRect(ox + gx * unit, oy + gy * unit, unit, unit);
      graphics_fill_rect(ctx, r, 0, GCornerNone);
    }
  }
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);

  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, b, 0, GCornerNone);

  bool have = (s_format == FORMAT_QR) ? s_qr_ok : (s_module_count > 0);
  if (!have) {
    /* 注意：手錶內建字型對中文支援有限，提示文字使用英文以確保可正常顯示 */
    graphics_context_set_text_color(ctx, GColorBlack);
    GRect tr = grect_inset(b, GEdgeInsets(10));
    graphics_draw_text(ctx,
        "No barcode set\n\nOpen the Pebble\nphone app settings\nto configure",
        fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
        tr, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    return;
  }

  /* 隱藏文字時，條碼可用滿整個畫面 */
  int reserved = s_text_visible ? TEXT_AREA_H : 0;
  GRect content = GRect(0, 0, b.size.w, b.size.h - reserved);

  if (s_format == FORMAT_QR) {
    draw_qr(ctx, content);
  } else {
    draw_linear(ctx, content);
  }

  /* 底部人眼可讀文字（可由設定或 SELECT 鍵開關）*/
  if (s_text_visible) {
    GRect text_r = GRect(2, b.size.h - TEXT_AREA_H, b.size.w - 4, TEXT_AREA_H);
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, s_data,
        fonts_get_system_font(FONT_KEY_GOTHIC_18),
        text_r, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  }
}

/* ------------------------------------------------------------------------- *
 *  持久化
 * ------------------------------------------------------------------------- */
static void load_state(void) {
  if (persist_exists(PERSIST_DATA)) {
    persist_read_string(PERSIST_DATA, s_data, sizeof(s_data));
  }
  s_format   = persist_exists(PERSIST_FORMAT)   ? persist_read_int(PERSIST_FORMAT)   : FORMAT_CODE39;
  s_rotation = persist_exists(PERSIST_ROTATION) ? persist_read_int(PERSIST_ROTATION) : 0;
  s_text_visible = persist_exists(PERSIST_TEXT_VISIBLE) ? persist_read_bool(PERSIST_TEXT_VISIBLE) : true;
  if (s_format < FORMAT_CODE39 || s_format > FORMAT_QR) s_format = FORMAT_CODE39;
  if (s_rotation != 0 && s_rotation != 1) s_rotation = 0;
}

static void save_state(void) {
  persist_write_string(PERSIST_DATA, s_data);
  persist_write_int(PERSIST_FORMAT, s_format);
  persist_write_int(PERSIST_ROTATION, s_rotation);
  persist_write_bool(PERSIST_TEXT_VISIBLE, s_text_visible);
}

/* ------------------------------------------------------------------------- *
 *  AppMessage（接收 Clay 設定）
 * ------------------------------------------------------------------------- */
static void inbox_received(DictionaryIterator *iter, void *context) {
  bool changed = false;

  Tuple *t_data = dict_find(iter, MESSAGE_KEY_BARCODE_DATA);
  if (t_data && t_data->type == TUPLE_CSTRING) {
    strncpy(s_data, t_data->value->cstring, sizeof(s_data) - 1);
    s_data[sizeof(s_data) - 1] = '\0';
    changed = true;
  }

  Tuple *t_fmt = dict_find(iter, MESSAGE_KEY_BARCODE_FORMAT);
  if (t_fmt) {
    int fmt = (t_fmt->type == TUPLE_CSTRING)
                ? atoi(t_fmt->value->cstring)
                : (int)t_fmt->value->int32;
    if (fmt < FORMAT_CODE39 || fmt > FORMAT_QR) fmt = FORMAT_CODE39;
    s_format = fmt;
    changed = true;
  }

  Tuple *t_txt = dict_find(iter, MESSAGE_KEY_BARCODE_TEXT_VISIBLE);
  if (t_txt) {
    s_text_visible = (t_txt->type == TUPLE_CSTRING)
                       ? (atoi(t_txt->value->cstring) != 0)
                       : (t_txt->value->int32 != 0);
    changed = true;
  }

  if (changed) {
    rebuild_barcode();
    save_state();
    layer_mark_dirty(s_canvas_layer);
  }
}

static void inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "AppMessage dropped: %d", (int)reason);
}

/* ------------------------------------------------------------------------- *
 *  按鍵：上 / 下 切換方向（正面 <-> 順時鐘 90 度）
 * ------------------------------------------------------------------------- */
static void toggle_rotation(void) {
  s_rotation = s_rotation ? 0 : 1;
  persist_write_int(PERSIST_ROTATION, s_rotation);
  layer_mark_dirty(s_canvas_layer);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  toggle_rotation();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  toggle_rotation();
}

/* SELECT：切換是否顯示底部人眼可讀文字 */
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_text_visible = !s_text_visible;
  persist_write_bool(PERSIST_TEXT_VISIBLE, s_text_visible);
  layer_mark_dirty(s_canvas_layer);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

/* ------------------------------------------------------------------------- *
 *  視窗 / App 生命週期
 * ------------------------------------------------------------------------- */
static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);

  window_set_background_color(window, GColorWhite);

  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  layer_add_child(root, s_canvas_layer);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

static void init(void) {
  load_state();
  rebuild_barcode();

  s_window = window_create();
  window_set_click_config_provider(s_window, click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  app_message_register_inbox_received(inbox_received);
  app_message_register_inbox_dropped(inbox_dropped);
  app_message_open(256, 64);
}

static void deinit(void) {
  save_state();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
