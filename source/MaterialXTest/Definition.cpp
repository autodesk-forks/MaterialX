//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXTest/Catch/catch.hpp>

#include <MaterialXCore/Definition.h>
#include <MaterialXCore/Document.h>
#include <MaterialXFormat/XmlIo.h>

namespace mx = MaterialX;

TEST_CASE("Implementation Check", "[definition]")
{
    // Create document and load in standard definitions and reference implementation
    //
    mx::DocumentPtr doc = mx::createDocument();

    std::vector<std::string> filenames =
    {
        "documents/Libraries/mx_stdlib_defs.mtlx",
        "documents/Libraries/mx_stdlib_impl_osl.mtlx",
    };
    for (const std::string& filename : filenames)
    {
        mx::readFromXmlFile(doc, filename);
    }

    // Check that there is one implementation per nodedef.
    const std::string target;
    const std::string language("osl");
    unsigned int found = 0;
    std::vector<mx::NodeDefPtr> definitions = doc->getNodeDefs();
    for (mx::NodeDefPtr definition : definitions)
    {
        // Ignore untyped nodedefs as they require no definition
        const std::string typeAttribute = definition->getAttribute(mx::Element::TYPE_ATTRIBUTE);
        if (typeAttribute != mx::NONE_TYPE_STRING)
        {
            if (definition->getImplementation(target, language))
            {
                found++;
            }
        }
        else
        {
            found++;
        }
    }
    REQUIRE(found == definitions.size());
}