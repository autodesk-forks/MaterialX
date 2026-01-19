//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXGenShader/ShaderGraphOptimizationPass.h>

#include <MaterialXGenShader/ShaderGraph.h>
#include <MaterialXGenShader/ShaderGraphDebug.h>
#include <MaterialXGenShader/GenContext.h>
#include <MaterialXGenShader/GenOptions.h>
#include <MaterialXTrace/Tracing.h>

#include <iostream>

MATERIALX_NAMESPACE_BEGIN

void ShaderGraphPassManager::addPass(ShaderGraphOptimizationPassPtr pass)
{
    _passes.push_back(pass);
}

size_t ShaderGraphPassManager::runToFixedPoint(ShaderGraph& graph, GenContext& context, size_t maxIterations)
{
    const bool dumpDot = context.getOptions().optDumpShaderGraphDot;
    size_t totalChanges = 0;
    
    if (dumpDot)
    {
        writeShaderGraphDot(graph, FilePath("opt_" + graph.getName() + "_initial.dot"));
    }

    for (size_t iteration = 0; iteration < maxIterations; ++iteration)
    {
        bool anyChanges = false;
        
        if (dumpDot)
        {
            std::cout << "=== Optimization iteration " << iteration << " ===" << std::endl;
        }

        for (const auto& pass : _passes)
        {
            if (dumpDot)
            {
                std::cout << "  Running pass: " << pass->getName() << std::endl;
            }

            bool passModified = false;
            {
                // Trace the optimization pass (parent slice has material name)
                MX_TRACE_SCOPE(Tracing::Category::Optimize, pass->getName().c_str());
                passModified = pass->run(graph, context);
            }
            
            if (passModified)
            {
                anyChanges = true;
                ++totalChanges;
                
                if (dumpDot)
                {
                    const string filename = "opt_" + graph.getName() + 
                                          "_iter" + std::to_string(iteration) + 
                                          "_" + pass->getName() + ".dot";
                    writeShaderGraphDot(graph, FilePath(filename));
                    std::cout << "    -> Graph modified, wrote " << filename << std::endl;
                }
            }
            else if (dumpDot)
            {
                std::cout << "    -> No changes" << std::endl;
            }
        }

        // Fixed point reached - no pass made any changes
        if (!anyChanges)
        {
            if (dumpDot)
            {
                std::cout << "=== Converged after " << iteration << " iterations ===" << std::endl;
            }
            break;
        }
    }

    if (dumpDot)
    {
        writeShaderGraphDot(graph, FilePath("opt_" + graph.getName() + "_final.dot"));
    }

    return totalChanges;
}

MATERIALX_NAMESPACE_END

