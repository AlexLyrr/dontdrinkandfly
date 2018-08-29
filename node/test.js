// var SerialPort = require('serialport');
// var port = new SerialPort('/dev/tty.usbserial-DN00P2UG', {
// 	// var port = new SerialPort("COM3", {
// 	baudRate: 115200
// });

// port.on('data', function(data) {
// 	console.log('Data:', data.toString('ascii'));
// });

const SerialPort = require('serialport');
const Readline = require('parser-readline');
const port = new SerialPort('/dev/tty.usbserial-DN00P2UG', {
	baudRate: 115200
});
const parser = port.pipe(new Readline({ delimiter: '\r\n' }));
parser.on('data', console.log);

/**
 * Ouput observations:
 * 	phi:
		Clockwise x: 0-ish until 32000 linear 180 degrees
   		Anti-clockwise x: 0-ish unti -32000 linear 180 degrees

	theta:
		Clockwise y: 0-ish until -16000 linear 90 degrees, decreases from there again
		anti-clockwise y: 0-ish until 16000 linear 90 degrees, decreases from there again

   	psi:
   		Clockwise y: 0-ish until 32000 non linear
   		Anti-clockwise y: 0-ish until -32000 non linear

   	sp: 
		clockwise x: 0 until 10k-ish
		ant-clockwise x: 0 until -10k-ish

   	sq: 
		clockwise y: 0 until 10k-ish
		ant-clockwise y: 0 until -10k-ish

   	sr:
		clockwise z: 0 until -10k-ish
		anti-clockwise z: 0 until 10k-ish

	bat_volt
		560 ish	
	
	temperature
		2831 ish

	pressure
		99900 ish (2 stories high)
 *
 * 
 */
