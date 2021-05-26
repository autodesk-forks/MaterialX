
Module = Module || {};

function isNode() {
	if (typeof module !== 'undefined' && module.exports) {
		return true;
	}
	return false;
}

const loadToFileSystem = async (filePath) => {
	let file;

	const fileName = filePath.substr(filePath.lastIndexOf('/') + 1);
	let dir = '/';
	if (isNode()) {
		const fs = require('fs');
		const path = require('path');
		dir = path.dirname(filePath);
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
	const fileStats = Module.FS.analyzePath(`${dir}/${fileName}`);
	if (fileStats.exists) {
		return `${dir}/${fileName}`;
	}

	const dirStats = Module.FS.analyzePath(dir);
	if (!dirStats.exists) {
		Module.FS.createPath('/', dir, true, true);
	}
	
	Module.FS.createDataFile(dir, fileName, new Uint8Array(file), true, true, true);
	return `${dir}/${fileName}`;
};

Module.readFromXmlFile = async (doc, filePath, ...args) => {
	const fileName = await loadToFileSystem(filePath);
	return Module._readFromXmlFile(doc, fileName, ...args);
}

Module.loadLibrary = async (filePath, doc, ...args) => {
	const fileName = await loadToFileSystem(filePath);
	return Module._loadLibrary(fileName, doc, ...args);
}
