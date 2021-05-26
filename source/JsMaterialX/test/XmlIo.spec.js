import * as path from 'path';
import { initMaterialX } from './testHelpers';
import { expect } from 'chai';
import * as fs from 'fs';

const resourcesBasePath = path.resolve('../../../resources/Materials/Examples/Syntax');

describe('XmlIo', () => {
  let mx;
  before(async () => {
      mx = await initMaterialX();
  });

  const loadFileSysmtem = (filePath) => {
    const file = fs.readFileSync(filePath);
    const dir = path.dirname(filePath);
    const dirStats = mx.FS.analyzePath(dir);
    if (!dirStats.exists) {
      mx.FS.createPath('/', dir, true, true);
    }
    mx.FS.createDataFile(dir, path.basename(filePath), new Uint8Array(file), true, true, true);
  }

  it('should read simple file', async () => {
    const doc = mx.createDocument();
    const filePath = path.resolve(resourcesBasePath, 'SimpleSrf.mtlx');
    await mx.readFromXmlFile(doc, filePath, "");
    expect(doc.getNodeDefs().length).to.equal(1);
  });

  it('should convert FilePath to string', () => {
    const exportOptions = new mx.XmlExportOptions();
    expect(typeof exportOptions.userTexturePath).to.equal("string");
  });

  it('should prepend include tag', () => {
    const doc = mx.createDocument();
    const includePath = "SomePath";
    const writeOptions = new mx.XmlWriteOptions();
    mx.prependXInclude(doc, includePath);
    const xmlString = mx.writeToXmlString(doc, writeOptions);
    expect(xmlString).to.include(xmlString);
  });

  it('load library', () => {
    const doc = mx.createDocument();
    const filepath = path.resolve(resourcesBasePath, 'SimpleSrf.mtlx');
    loadFileSysmtem(filepath);
    mx.loadLibrary(filepath, doc);
    const xmlString = mx.writeToXmlString(doc, new mx.XmlWriteOptions());
    expect(xmlString).to.include(filepath);
  });
});

