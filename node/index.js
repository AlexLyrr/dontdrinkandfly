var SerialPort = require("serialport");
var port = new SerialPort("/dev/tty.usbserial-DN00P2UG", {
	baudRate: 115200,
	autoOpen: false
});

port.open(err => {
	if (err) {
		console.log("Open error:", err);
	}
	setTimeout(() => {
		console.log("Starting...");
		port.write(
			Buffer.from([
				0x01,
				0x00,
				0x00,
				0x00,
				0x00,

				0x00,
				0x00,
				0x00,
				0x00,
				0x00
			])
		);
	}, 1e3);
	setTimeout(() => {
		console.log("Switching manual...");
		port.write(
			Buffer.from([
				0x05,
				0x02,
				0x00,
				0x00,
				0x00,

				0x00,
				0x00,
				0x00,
				0x00,
				0x00
			])
		);
		setTimeout(() => {
			port.write(
				Buffer.from([
					0x03,
					0x00,
					100,
					90,
					90,

					1000 >> 8,
					1000 & 0xff,
					0x00,
					0x00,
					0x00
				])
			);
		}, 1e3);
	}, 5e3);
	setTimeout(() => {
		console.log("Abort abort...");
		port.write(
			Buffer.from([
				0x03,
				0x80,
				0x00,
				0x00,
				0x00,

				0x00,
				0x00,
				0x00,
				0x00,
				0x00
			])
		);
	}, 30e3);
});

// Switches the port into "flowing mode"
port.on("data", function(data) {
	if (data.length === 10) {
		switch (data[0]) {
			case 2:
				console.log(
					`${new Date().toString()} DroneStatus(mode=${data[1] &
						0x0f}, bat=${data[2] << 8}, phi=${data.readInt8(3) *
						(360 / 128)}, theta=${data.readInt8(4) *
						(360 / 128)}, appClock=${data.readUInt16BE(7)})`
				);
				break;
			case 7:
				console.log("Received error packet with code:", data[1]);
				break;
			case 10:
				console.log(
					`${new Date().toString()} MotorStatus(1=${data.readUInt16BE(
						1
					)}, 1=${data.readUInt16BE(3)}, 1=${data.readUInt16BE(
						5
					)}, 1=${data.readUInt16BE(7)})`
				);
				break;
			default:
				console.log("unknown packet type:", data[0]);
		}
	} else {
		console.log(data.toString("utf8"));
	}
	// console.log(data);
});
