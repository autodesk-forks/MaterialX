import Module from './_build/JsMaterialXCore.js';
import { expect } from 'chai';
import { getMtlxStrings } from './testHelpers';

describe('XmlExport', () => {
    // These should be relative to cwd
    const includeTestPath = 'data/includes';
    const examplesPath = '../../../resources/Materials/Examples/Syntax';
    const exampleFilenames = [
        'Looks.mtlx',
        'PaintMaterials.mtlx',
        'PostShaderComposite.mtlx',
        'CustomNode.mtlx',
        'GeomInfos.mtlx',
        'MaterialBasic.mtlx',
        'MultiOutput.mtlx',
        'NodeGraphs.mtlx',
        'PreFlattenedGraph.mtlx',
        'PreShaderComposite.mtlx',
        'SimpleSrf.mtlx',
        'SubGraphs.mtlx',
    ];

    let mx;
    before(async () => {
        mx = await Module();
    });

    it('Convert FilePath to string', () => {
        const exportOptions = new mx.XmlExportOptions();
        expect(typeof exportOptions.resolvedTexturePath).to.equal('string');
    });

    it('Export file to string', async () => {
        // Read all example documents and write them to an XML string
        const searchPath = examplesPath;
        for (let filename of exampleFilenames) {
            const doc = mx.createDocument();
            await mx.readFromXmlFile(doc, filename, searchPath);

            // Serialize to XML.
            const exportOptions = new mx.XmlExportOptions();
            exportOptions.writeXIncludeEnable = false;
            const xmlString = mx.exportToXmlString(doc, exportOptions);

            // Verify that the serialized document is identical.
            const writtenDoc = mx.createDocument();
            await mx.readFromXmlString(writtenDoc, xmlString);
            expect(writtenDoc).to.eql(doc);
        };
    });

    // Node only, because we cannot read from a downloaded file in the browser
    it('Export XML to file', async () => {
        const filename = '_build/testFile.mtlx';
        const includeRegex = /<xi:include href="(.*)"\s*\/>/g;
        const doc = mx.createDocument();
        await mx.readFromXmlFile(doc, 'root.mtlx', includeTestPath);

        // Export using includes
        mx.exportToXmlFile(doc, filename);
        // Read written document and compare with the original
        const doc2 = mx.createDocument();
        await mx.readFromXmlFile(doc2, filename, includeTestPath);
        expect(doc2.equals(doc));
        // Read written file content and verify that includes are preserved
        let fileString = getMtlxStrings([filename], '')[0];
        let matches = Array.from(fileString.matchAll(includeRegex));
        expect(matches.length).to.be.greaterThan(0);

        // Export inlining included content
        const exportOptions = new mx.XmlExportOptions();
        exportOptions.writeXIncludeEnable = false;
        mx.writeToXmlFile(doc, filename, exportOptions);
        // Read written document and compare with the original
        const doc3 = mx.createDocument();
        await mx.readFromXmlFile(doc3, filename);
        expect(doc3.equals(doc));
        expect(doc.getChild('paint_semigloss')).to.exist;
        // Read written file content and verify that includes are inlined
        fileString = getMtlxStrings([filename], '')[0];
        matches = Array.from(fileString.matchAll(includeRegex));
        expect(matches.length).to.equal(0);
    });
});

