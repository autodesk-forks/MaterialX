//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTypeDef.h>

namespace MaterialX
{

const RtToken RtType::BOOLEAN("boolean");
const RtToken RtType::INTEGER("integer");
const RtToken RtType::FLOAT("float");
const RtToken RtType::VECTOR2("vector2");
const RtToken RtType::VECTOR3("vector3");
const RtToken RtType::VECTOR4("vector4");
const RtToken RtType::COLOR2("color2");
const RtToken RtType::COLOR3("color3");
const RtToken RtType::COLOR4("color4");
const RtToken RtType::MATRIX33("matrix33");
const RtToken RtType::MATRIX44("matrix44");
const RtToken RtType::TOKEN("token");
const RtToken RtType::STRING("string");
const RtToken RtType::FILENAME("filename");
const RtToken RtType::INTEGERARRAY("integerarray");
const RtToken RtType::FLOATARRAY("floatarray");
const RtToken RtType::BSDF("BSDF");
const RtToken RtType::EDF("EDF");
const RtToken RtType::VDF("VDF");
const RtToken RtType::SURFACESHADER("surfaceshader");
const RtToken RtType::VOLUMESHADER("volumeshader");
const RtToken RtType::DISPLACEMENTSHADER("displacementshader");
const RtToken RtType::LIGHTSHADER("lightshader");
const RtToken RtType::SURFACEMATERIAL("surfacematerial");
const RtToken RtType::VOLUMEMATERIAL("volumematerial");
const RtToken RtType::AUTO("auto");

namespace
{
    using ChannelMap = std::unordered_map<char, int>;

    struct TypeDef
    {
        TypeDef(const RtToken& name, const RtToken& basetype, const RtToken& sematic,
                 size_t size, const ChannelMap& channelMapping = ChannelMap()) :
            _name(name),
            _basetype(basetype),
            _semantic(sematic),
            _size(size),
            _channelMapping(channelMapping)
        {
            // TODO: Handle other types in connections
            _connectionTypes.insert(name);
        }

        const RtTokenSet& getValidConnectionTypes() const
        {
            return _connectionTypes;
        }

        const RtToken _name;
        const RtToken _basetype;
        const RtToken _semantic;
        const size_t _size;
        std::unordered_map<char, int> _channelMapping;
        RtTokenSet _connectionTypes;
    };

    class TypeDefRegistry
    {
    public:
        TypeDefRegistry()
        {
            // Register all default types.
            newType("boolean", RtTypeDef::BASETYPE_BOOLEAN);
            newType("integer", RtTypeDef::BASETYPE_INTEGER);
            newType("float", RtTypeDef::BASETYPE_FLOAT);
            newType("vector2", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_VECTOR, 2, { {'x', 0}, {'y', 1} });
            newType("vector3", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_VECTOR, 3, { {'x', 0}, {'y', 1}, {'z', 2} });
            newType("vector4", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_VECTOR, 4, { {'x', 0}, {'y', 1}, {'z', 2}, {'w', 3} });
            newType("color2", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_COLOR, 2, { {'r', 0}, {'a', 1} });
            newType("color3", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_COLOR, 3, { {'r', 0}, {'g', 1}, {'b', 2} });
            newType("color4", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_COLOR, 4, { {'r', 0}, {'g', 1}, {'b', 2}, {'a', 3} });
            newType("matrix33", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_MATRIX, 9);
            newType("matrix44", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_MATRIX, 16);
            newType("token", RtTypeDef::BASETYPE_STRING);
            newType("string", RtTypeDef::BASETYPE_STRING);
            newType("filename", RtTypeDef::BASETYPE_STRING, RtTypeDef::SEMANTIC_FILENAME);
            newType("integerarray", RtTypeDef::BASETYPE_INTEGER, RtTypeDef::SEMANTIC_NONE, 0);
            newType("floatarray", RtTypeDef::BASETYPE_FLOAT, RtTypeDef::SEMANTIC_NONE, 0);
            newType("BSDF", RtTypeDef::BASETYPE_NONE, RtTypeDef::SEMANTIC_CLOSURE);
            newType("EDF", RtTypeDef::BASETYPE_NONE, RtTypeDef::SEMANTIC_CLOSURE);
            newType("VDF", RtTypeDef::BASETYPE_NONE, RtTypeDef::SEMANTIC_CLOSURE);
            newType("surfaceshader", RtTypeDef::BASETYPE_NONE, RtTypeDef::SEMANTIC_SHADER);
            newType("volumeshader", RtTypeDef::BASETYPE_NONE, RtTypeDef::SEMANTIC_SHADER);
            newType("displacementshader", RtTypeDef::BASETYPE_NONE, RtTypeDef::SEMANTIC_SHADER);
            newType("lightshader", RtTypeDef::BASETYPE_NONE, RtTypeDef::SEMANTIC_SHADER);
            newType("auto", RtTypeDef::BASETYPE_NONE);
        }

        RtTypeDef* newType(const RtToken& name, const RtToken& basetype, const RtToken& sematic = RtTypeDef::SEMANTIC_NONE,
                            size_t size = 1, const ChannelMap& channelMapping = ChannelMap())
        {
            _map[name] = std::unique_ptr<RtTypeDef>(new RtTypeDef(name, basetype, sematic, size));

            RtTypeDef* ptr = _map[name].get();
            for (auto it : channelMapping)
            {
                ptr->setChannelIndex(it.first, it.second);
            }

            return ptr;
        }

        const RtTypeDef* findType(const RtToken& name)
        {
            auto it = _map.find(name);
            return it != _map.end() ? it->second.get() : nullptr;
        }

        static TypeDefRegistry& get()
        {
            static TypeDefRegistry _registry;
            return _registry;
        }

    private:
        using RtTypeDefPtr = std::unique_ptr<RtTypeDef>;
        RtTokenMap<RtTypeDefPtr> _map;
    };
}

const RtToken RtTypeDef::BASETYPE_NONE = "none";
const RtToken RtTypeDef::BASETYPE_BOOLEAN = "boolean";
const RtToken RtTypeDef::BASETYPE_FLOAT = "float";
const RtToken RtTypeDef::BASETYPE_INTEGER = "integer";
const RtToken RtTypeDef::BASETYPE_STRING = "string";
const RtToken RtTypeDef::BASETYPE_STRUCT = "struct";

const RtToken RtTypeDef::SEMANTIC_NONE = "none";
const RtToken RtTypeDef::SEMANTIC_COLOR = "color";
const RtToken RtTypeDef::SEMANTIC_VECTOR = "vector";
const RtToken RtTypeDef::SEMANTIC_MATRIX = "matrix";
const RtToken RtTypeDef::SEMANTIC_FILENAME = "filename";
const RtToken RtTypeDef::SEMANTIC_CLOSURE = "closure";
const RtToken RtTypeDef::SEMANTIC_SHADER = "shader";

RtTypeDef::RtTypeDef(const RtToken& name, const RtToken& basetype, const RtToken& semantic, size_t size) :
    _ptr(new TypeDef(name, basetype, semantic, size))
{
}

RtTypeDef::~RtTypeDef()
{
    delete static_cast<TypeDef*>(_ptr);
}

const RtToken& RtTypeDef::getName() const
{
    return static_cast<TypeDef*>(_ptr)->_name;
}

const RtToken& RtTypeDef::getBaseType() const
{
    return static_cast<TypeDef*>(_ptr)->_basetype;
}

const RtToken& RtTypeDef::getSemantic() const
{
    return static_cast<TypeDef*>(_ptr)->_semantic;
}

size_t RtTypeDef::getSize() const
{
    return static_cast<TypeDef*>(_ptr)->_size;
}

int RtTypeDef::getChannelIndex(char channel) const
{
    TypeDef* ptr = static_cast<TypeDef*>(_ptr);
    auto it = ptr->_channelMapping.find(channel);
    return it != ptr->_channelMapping.end() ? it->second : -1;
}

void RtTypeDef::setChannelIndex(char channel, int index)
{
    static_cast<TypeDef*>(_ptr)->_channelMapping[channel] = index;
}

const RtTokenSet& RtTypeDef::getValidConnectionTypes() const
{
    return static_cast<TypeDef*>(_ptr)->getValidConnectionTypes();
}

RtTypeDef* RtTypeDef::registerType(const RtToken& name, const RtToken& basetype,
                                     const RtToken& semantic, size_t size)
{
    if (TypeDefRegistry::get().findType(name))
    {
        throw ExceptionRuntimeError("A type named '" + name.str() + "' is already registered");
    }
    return TypeDefRegistry::get().newType(name, basetype, semantic, size);
}

const RtTypeDef* RtTypeDef::findType(const RtToken& name)
{
    return TypeDefRegistry::get().findType(name);
}

}
