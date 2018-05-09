// listen for the "keypress" event
process.stdin.on('data', data => {
	console.log(data);
	if (data[0] === 3) {
		process.exit(0);
	}
});

process.stdin.setRawMode(true);
