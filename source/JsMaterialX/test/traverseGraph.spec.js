import { expect } from 'chai';
import { traverse, initMaterialX } from './testHelpers';

it('Traverse Graph', async () => {
    let mx, doc, image2, constant, multiply, contrast, mix, output;
    function isCycle(cb) {
        try {
            return cb();
        } catch (exceptionPtr) {
            const message = mx.getExceptionMessage(exceptionPtr);
            if (message.indexOf('Encountered cycle') !== -1) {
                return true;
            }
            return false;
        }
    }

    mx = await initMaterialX();
    // Create a document.
    doc = mx.createDocument();
    // Create a node graph with the following structure:
    //
    // [image1] [constant]     [image2]
    //        \ /                 |
    //    [multiply]          [contrast]         [noise3d]
    //             \____________  |  ____________/
    //                          [mix]
    //                            |
    //                         [output]
    //
    const nodeGraph = doc.addNodeGraph();
    const image1 = nodeGraph.addNode('image');
    image2 = nodeGraph.addNode('image');
    constant = nodeGraph.addNode('constant');
    multiply = nodeGraph.addNode('multiply');
    contrast = nodeGraph.addNode('contrast');
    const noise3d = nodeGraph.addNode('noise3d');
    mix = nodeGraph.addNode('mix');
    output = nodeGraph.addOutput();
    multiply.setConnectedNode('in1', image1);
    multiply.setConnectedNode('in2', constant);
    contrast.setConnectedNode('in', image2);
    mix.setConnectedNode('fg', multiply);
    mix.setConnectedNode('bg', contrast);
    mix.setConnectedNode('mask', noise3d);
    output.setConnectedNode(mix);

    expect(doc.validate()).to.be.true;
    // TODO: select check message

    // Traverse the document tree (implicit iterator).
    const elements = doc.traverseTree();
    let nodeCount = 0;
    traverse(elements, (elem) => {
        // Display the filename of each image node.
        if (elem instanceof mx.Node) {
            nodeCount++;
        }
    });
    expect(nodeCount).to.equal(7);

    // Traverse the document tree (explicit iterator)
    let treeIter = doc.traverseTree();
    nodeCount = 0;
    let maxElementDepth = 0;
    traverse(treeIter, (elem) => {
        // Display the filename of each image node.
        if (elem instanceof mx.Node) {
            nodeCount++;
        }
        maxElementDepth = Math.max(maxElementDepth, treeIter.getElementDepth());
    });

    expect(nodeCount).to.equal(7);
    expect(maxElementDepth).to.equal(3);

    // Traverse the document tree (prune subtree).
    nodeCount = 0;
    treeIter = doc.traverseTree();
    traverse(treeIter, (elem) => {
        // Display the filename of each image node.
        if (elem instanceof mx.Node) {
            nodeCount++;
        }
        if (elem instanceof mx.NodeGraph) {
            treeIter.setPruneSubtree(true);
        }
    });

    expect(nodeCount).to.equal(0);
    
    // Traverse upstream from the graph output (implicit iterator)
    nodeCount = 0;
    traverse(output.traverseGraph(), (edge) => {
        const upstreamElem = edge.getUpstreamElement();
        const connectingElem = edge.getConnectingElement();
        const downstreamElem = edge.getDownstreamElement();
        if (upstreamElem instanceof mx.Node) {
            nodeCount++;
            if (downstreamElem instanceof mx.Node) {
                expect(connectingElem instanceof mx.Input).to.be.true;
            }
        }
    });
    expect(nodeCount).to.equal(7);

    // Traverse upstream from the graph output (explicit iterator)
    nodeCount = 0;
    maxElementDepth = 0;
    let maxNodeDepth = 0;
    let graphIter = output.traverseGraph();
    traverse(graphIter, (edge) => {
        const upstreamElem = edge.getUpstreamElement();
        if (upstreamElem instanceof mx.Node) {
            nodeCount++;
        }
        maxElementDepth = Math.max(maxElementDepth, graphIter.getElementDepth());
        maxNodeDepth = Math.max(maxNodeDepth, graphIter.getNodeDepth());
    });

    expect(nodeCount).to.equal(7);
    expect(maxElementDepth).to.equal(3);
    expect(maxNodeDepth).to.equal(3);

    // Traverse upstream from the graph output (prune subgraph)
    nodeCount = 0;
    graphIter = output.traverseGraph();
    traverse(graphIter, (edge) => {
        const upstreamElem = edge.getUpstreamElement();
        expect(upstreamElem.getSelf()).to.be.an.instanceof(mx.Element);
        if (upstreamElem instanceof mx.Node) {
            nodeCount++;
        }
        if (upstreamElem.getCategory() === 'multiply') {
            graphIter.setPruneSubgraph(true);
        }
    });
    expect(nodeCount).to.equal(5);

    // Create and detect a cycle
    multiply.setConnectedNode('in2', mix);

    expect(
        isCycle(() => {
            return output.hasUpstreamCycle();
        })
    ).to.be.true;
    expect(
        isCycle(() => {
            return doc.validate();
        })
    ).to.be.true;
    multiply.setConnectedNode('in2', constant);
    expect(
        isCycle(() => {
            return output.hasUpstreamCycle();
        })
    ).to.be.false;
    expect(
        isCycle(() => {
            return doc.validate();
        })
    ).to.be.true;

    // Create and detect a loop
    contrast.setConnectedNode('in', contrast);
    expect(
        isCycle(() => {
            return output.hasUpstreamCycle();
        })
    ).to.be.true;
    expect(
        isCycle(() => {
            return doc.validate();
        })
    ).to.be.true;
    contrast.setConnectedNode('in', image2);
    expect(
        isCycle(() => {
            return output.hasUpstreamCycle();
        })
    ).to.be.false;
    expect(
        isCycle(() => {
            return doc.validate();
        })
    ).to.be.true;
});
