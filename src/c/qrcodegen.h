/*
 * QR Code generator library (C) — header.
 *
 * Faithful port of Project Nayuki's public-domain / MIT-licensed
 * "QR Code generator library". See https://www.nayuki.io/page/qr-code-generator-library
 *
 * Self-contained: no dynamic memory, caller supplies buffers.
 */

#ifndef QRCODEGEN_H
#define QRCODEGEN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error correction level in a QR Code symbol. */
enum qrcodegen_Ecc {
	qrcodegen_Ecc_LOW = 0,     /* ~7% */
	qrcodegen_Ecc_MEDIUM,      /* ~15% */
	qrcodegen_Ecc_QUARTILE,    /* ~25% */
	qrcodegen_Ecc_HIGH,        /* ~30% */
};

/* The mask pattern used in a QR Code symbol. */
enum qrcodegen_Mask {
	qrcodegen_Mask_AUTO = -1,
	qrcodegen_Mask_0 = 0,
	qrcodegen_Mask_1,
	qrcodegen_Mask_2,
	qrcodegen_Mask_3,
	qrcodegen_Mask_4,
	qrcodegen_Mask_5,
	qrcodegen_Mask_6,
	qrcodegen_Mask_7,
};

/* Describes how a segment's data bits are interpreted. */
enum qrcodegen_Mode {
	qrcodegen_Mode_NUMERIC      = 0x1,
	qrcodegen_Mode_ALPHANUMERIC = 0x2,
	qrcodegen_Mode_BYTE         = 0x4,
	qrcodegen_Mode_KANJI        = 0x8,
	qrcodegen_Mode_ECI          = 0x7,
};

/* A segment of character/binary/control data in a QR Code symbol. */
struct qrcodegen_Segment {
	enum qrcodegen_Mode mode;
	int numChars;
	uint8_t *data;
	int bitLength;
};

#define qrcodegen_VERSION_MIN   1
#define qrcodegen_VERSION_MAX  40

/* Buffer length (bytes) required to hold a QR Code of the given version. */
#define qrcodegen_BUFFER_LEN_FOR_VERSION(n)  ((((n) * 4 + 17) * ((n) * 4 + 17) + 7) / 8 + 1)

/* Maximum buffer length needed for any QR Code (version 40). */
#define qrcodegen_BUFFER_LEN_MAX  qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_VERSION_MAX)

/*
 * Encodes the given text into a QR Code, returning true on success.
 * tempBuffer and qrcode must each be qrcodegen_BUFFER_LEN_MAX bytes (or sized for maxVersion).
 */
bool qrcodegen_encodeText(const char *text, uint8_t tempBuffer[], uint8_t qrcode[],
	enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl);

/* Encodes the given binary data into a QR Code, returning true on success. */
bool qrcodegen_encodeBinary(uint8_t dataAndTemp[], size_t dataLen, uint8_t qrcode[],
	enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl);

/* Encodes the given segments into a QR Code, returning true on success. */
bool qrcodegen_encodeSegments(const struct qrcodegen_Segment segs[], size_t len,
	enum qrcodegen_Ecc ecl, uint8_t tempBuffer[], uint8_t qrcode[]);

/* Advanced: encode segments choosing version/mask/ecc with the given constraints. */
bool qrcodegen_encodeSegmentsAdvanced(const struct qrcodegen_Segment segs[], size_t len,
	enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask,
	bool boostEcl, uint8_t tempBuffer[], uint8_t qrcode[]);

/* Tests whether the string can be encoded as a numeric segment. */
bool qrcodegen_isNumeric(const char *text);

/* Tests whether the string can be encoded as an alphanumeric segment. */
bool qrcodegen_isAlphanumeric(const char *text);

/* Number of bytes needed for a buffer used by qrcodegen_makeBytes(). */
size_t qrcodegen_calcSegmentBufferSize(enum qrcodegen_Mode mode, size_t numChars);

struct qrcodegen_Segment qrcodegen_makeBytes(const uint8_t data[], size_t len, uint8_t buf[]);
struct qrcodegen_Segment qrcodegen_makeNumeric(const char *digits, uint8_t buf[]);
struct qrcodegen_Segment qrcodegen_makeAlphanumeric(const char *text, uint8_t buf[]);
struct qrcodegen_Segment qrcodegen_makeEci(long assignVal, uint8_t buf[]);

/* Returns the side length (in modules) of the given QR Code (21..177). */
int qrcodegen_getSize(const uint8_t qrcode[]);

/* Returns the colour of the module (true = black) at (x, y); out-of-bounds returns false. */
bool qrcodegen_getModule(const uint8_t qrcode[], int x, int y);

#ifdef __cplusplus
}
#endif

#endif
