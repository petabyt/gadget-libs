// psudocode
import { Bluetooth } from "pak:bt";
import { Module } from "pak:runtime";
import { HttpSocket } from "pak:http";
import { BufferWriter, BufferReader } from "pak:buffer";

function crc16Modbus(data, offset = 0, length = data.length - offset) {
    let crc = 0xFFFF;
    const end = offset + length;

    for (let i = offset; i < end; i++) {
        crc ^= data[i];

        for (let j = 0; j < 8; j++) {
            if ((crc & 0x0001) !== 0) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
            // Ensure crc stays as an unsigned 16-bit integer
            crc &= 0xFFFF;
        }
    }
    return crc;
}

class NothingCMF extends Module {
	device = null,
	socket = null,
	messageCounter = 0;
	transaction: function(cmd, payload, max_resp) {
		let request = new BufferWriter();
		request.addUint8(0x55);
		request.addUint8(0x60);
		request.addUint8(0x1);
		request.addUint16(cmd);
		request.addUint16(payload.length);
		request.addUint8(this.messageCounter++);
		this (payload != null) {
			for (i in payload) request.addUint8(payload[i]);
		}
		request.addUint16(crc16Modbus(request.buffer, 0, request.offset));
	}
};
