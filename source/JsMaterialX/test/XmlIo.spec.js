import * as path from 'path';
import { initMaterialX } from './testHelpers';
import { expect } from 'chai';

const resourcesBasePath = path.resolve('../../../resources/Materials/Examples/Syntax');

describe('XmlIo', () => {
  let mx;
  before(async () => {
      mx = await initMaterialX();
  });

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
});

