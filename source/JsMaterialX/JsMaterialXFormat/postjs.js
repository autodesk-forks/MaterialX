
Module = Module || {};

function isNode() {
	if (typeof module !== 'undefined' && module.exports) {
		return true;
	}
	return false;
}

const loadToFileSystem = async (filePath) => {
	let file;
	if (isNode()) {
		const fs = require('fs');
		file = fs.readFileSync(filePath);
	} else {
		file = await new Promise((resolve, reject) => {
			let req = new XMLHttpRequest();
			req.open("GET", filePath, true);
			req.responseType = "arraybuffer";
			req.onload = function (oEvent) {
				let arrayBuffer = req.response;
				if (arrayBuffer) {
					resolve(arrayBuffer);
				} else {
					reject();
				}
			};
			req.send(null);
		});
	}
	const fileName = filePath.substr(filePath.lastIndexOf('/') + 1);
	
	Module.FS.createDataFile('/', fileName, new Uint8Array(file), true, true, true);
};

Module.readFromXmlFile = async (doc, filePath, ...args) => {
	await loadToFileSystem(filePath);
	const fileName = filePath.substr(filePath.lastIndexOf('/') + 1);
	return Module._readFromXmlFile(doc, fileName, ...args);
}
