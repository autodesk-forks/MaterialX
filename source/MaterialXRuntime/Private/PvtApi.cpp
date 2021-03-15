//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtApi.h>

#include <MaterialXRuntime/RtApi.h>
#include <MaterialXRuntime/RtSchema.h>
#include <MaterialXRuntime/RtNodeDef.h>
#include <MaterialXRuntime/RtNodeGraph.h>
#include <MaterialXRuntime/RtNodeImpl.h>
#include <MaterialXRuntime/RtTargetDef.h>
#include <MaterialXRuntime/RtFileIo.h>

namespace MaterialX
{

namespace
{
    const RtToken DEFAULT_LIBRARY_NAME("default");
    const string NUMBERS("0123456789");
}

void PvtApi::reset()
{
    _createFunctions.clear();
    _libraries.clear();
    _stages.clear();

    _unitDefinitions = UnitConverterRegistry::create();
}

void PvtApi::loadLibrary(const RtToken& name, const FilePath& path, const RtReadOptions* options, bool forceReload)
{
    auto it = _libraries.find(name);
    if (it != _libraries.end())
    {
        if (forceReload)
        {
            _libraries.erase(it);
        }
        else
        {
            throw ExceptionRuntimeError("A library named '" + name.str() + "' has already been loaded");
        }
    }

    RtStagePtr stage = PvtStage::createNew(name);
    _libraries[name] = stage;

    // Load in the files(s).
    RtFileIo file(stage);
    file.readLibrary(path, _searchPaths, options);

    // Register any definitions and implementations.
    for (RtPrim prim : stage->traverse())
    {
        if (prim.hasApi<RtNodeDef>())
        {
            registerNodeDef(prim);
        }
        else if (prim.hasApi<RtNodeGraph>())
        {
            RtNodeGraph nodegraph(prim);
            if (nodegraph.getDefinition() != EMPTY_TOKEN)
            {
                registerNodeGraph(prim);
            }
        }
        else if (prim.hasApi<RtNodeImpl>())
        {
            registerNodeImpl(prim);
        }
        else if (prim.hasApi<RtTargetDef>())
        {
            registerTargetDef(prim);
        }
    }
}

void PvtApi::unloadLibrary(const RtToken& name)
{
    auto it = _libraries.find(name);
    if (it != _libraries.end())
    {
        RtStagePtr stage = it->second;
        for (RtPrim prim : stage->traverse())
        {
            if (prim.hasApi<RtNodeDef>())
            {
                unregisterNodeDef(prim.getName());
            }
            else if (prim.hasApi<RtNodeGraph>())
            {
                RtNodeGraph nodegraph(prim);
                if (nodegraph.getDefinition() != EMPTY_TOKEN)
                {
                    unregisterNodeGraph(prim.getName());
                }
            }
            else if (prim.hasApi<RtNodeImpl>())
            {
                unregisterNodeImpl(prim.getName());
            }
            else if (prim.hasApi<RtTargetDef>())
            {
                unregisterTargetDef(prim.getName());
            }
        }

        _libraries.erase(it);
    }
}

RtToken PvtApi::makeUniqueStageName(const RtToken& name) const
{
    RtToken newName = name;

    // Check if there is another stage with this name.
    RtStagePtr otherStage = getStage(name);
    if (otherStage)
    {
        // Find a number to append to the name, incrementing
        // the counter until a unique name is found.
        string baseName = name.str();
        int i = 1;
        const size_t n = name.str().find_last_not_of(NUMBERS) + 1;
        if (n < name.str().size())
        {
            const string number = name.str().substr(n);
            i = std::stoi(number) + 1;
            baseName = baseName.substr(0, n);
        }
        // Iterate until there is no other stage with the resulting name.
        do {
            newName = RtToken(baseName + std::to_string(i++));
            otherStage = getStage(newName);
        } while (otherStage);
    }

    return newName;
}

}
