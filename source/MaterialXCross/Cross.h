//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#ifndef MATERIALX_CROSS_H
#define MATERIALX_CROSS_H

#include <string>

namespace MaterialX
{
namespace Cross
{
    void initialize();
    void finalize();

    std::string glslToHlsl(
        const std::string& glslUniformDefinitions,
        const std::string& glslCode,
        const std::string& fragmentName
    );
}
}

#endif
