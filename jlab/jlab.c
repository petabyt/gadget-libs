#include <stdint.h>
#include <byteswap.h>
#include <string.h>

#define MAGIC {0xFE, 0xDC, 0xBA}
#define END_BYTE 0xEF

enum CommandCodes {
	SET_SETTING_TYPE0 = 0x08,
	SET_SETTING_TYPE1 = 0xff,
	SET_SETTING_TYPE2 = 0xc0,
	GET_PROPERTIES1 = 0x03,
	GET_PROPERTIES2 = 0xc1,
	GET_PROPERTIES3 = 0x07,
};

enum PacketType {
	PACKET_COMMAND = 0xC0,
	PACKET_EVENT = 0x80,
	PACKET_RESP = 0x00,
};

// SET_SETTING_TYPE2 / 0xc0
enum PropertiesType2 {
	PROP_TOUCH_CONTROLS = 2,
	PROP_MOVIE_MUSIC_MODE = 5,
	// Unknown 7 set during init
};
// SET_SETTING_TYPE1 / 0xff
enum PropertiesType1 {
	PROP_SAFE_HEARING = 3,
	PROP_ENABLE_TOUCH_CONTROLS = 5,
};
// SET_SETTING_TYPE0 / 0x08
enum PropertiesType0 {
	PROP_EQUALIZER = 4,
	PROP_NOISE_CANCELLING = 0xd,
};

enum AncModes {
	ANC_BE_AWARE = 2,
	ANC_ON = 1,
	ANC_OFF = 0,
};

enum EqualizerPresetIndexes {
	EQ1 = 0,
	EQ2 = 1,
	EQ3 = 2,
	EQ_CUSTOM = 6,
};

enum PropertyC1Codes {
	PROP_BATTERY_STATUS = 0, // 0x64 0x64 0x64 == 100% 100% 100%
	PROP_DEVICE_NAME = 1, // "JLab GO Pods ANC"
	PROP_FW_UNKNOWN_KEY = 0xd, // "hE9yfseX6UdK7rFh"
	PROP_FW_VERSION_CODE = 0xc, // "jl_sdk_ac697_publish"
};

struct __attribute__((packed)) PacketHeader {
	uint8_t magic[3];
	uint8_t packet_type;
	uint8_t command;
	uint16_t length;
	uint8_t payload[]; // of size length
	// end byte (0xEF)
};

struct __attribute__((packed)) SetPropertyContainer {
	uint8_t transaction_counter;
	uint8_t unknown_0xff;
	uint8_t value_length;
	uint8_t property_id;
	uint8_t selected_index;
};

struct __attribute__((packed)) NoiseCancellingPayload {
	struct SetPropertyContainer hdr;

	uint8_t anc_mode; // AncModes

	// Following 2 seem to always be 0x4000
	uint16_t unknown_4000_1;
	uint16_t unknown_4000_2;
	uint16_t value_1;
	uint16_t value_2;
}noise_cancelling;

struct __attribute__((packed)) SetPropertyByte {
	uint8_t transaction_counter;
	uint8_t length;
	uint8_t property_id;
	uint8_t value;
}mode;

struct __attribute__((packed)) EqualizerValue {
	uint8_t val_31;
	uint8_t val_62;
	uint8_t val_125;
	uint8_t val_250;
	uint8_t val_500;
	uint8_t val_1k;
	uint8_t val_2k;
	uint8_t val_4k;
	uint8_t val_8k;
	uint8_t val_16k;
};
struct __attribute__((packed)) SetEqualizerPayload {
	struct SetPropertyContainer hdr;
	struct EqualizerValue value;
};

int example_turn_off_anc(void) {
	int message_counter = 0x40;
	char buffer[0xff];
	int of = 0;

	((struct PacketHeader *)&buffer[of])[0] = (struct PacketHeader){
		.magic = MAGIC,
		.packet_type = PACKET_COMMAND,
		.command = SET_SETTING_TYPE0,
		.length = bswap_16(0xd),
	};
	of += sizeof(struct PacketHeader);
	((struct NoiseCancellingPayload *)&buffer[of])[0] = (struct NoiseCancellingPayload){
		.hdr = {message_counter++, 0xff, 0xa, PROP_NOISE_CANCELLING},
		.anc_mode = ANC_OFF,
		.unknown_4000_1 = bswap_16(0x4000),
		.unknown_4000_2 = bswap_16(0x4000),
		.value_1 = bswap_16(0x0),
		.value_2 = bswap_16(0x0),
	};
	of += sizeof(struct NoiseCancellingPayload);
	buffer[of] = END_BYTE;

	return 0;
}
