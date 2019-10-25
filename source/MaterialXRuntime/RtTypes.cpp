//
// TM & (c) 2017 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/RtTypes.h>
#include <memory>

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

    struct TypeDesc
    {
        TypeDesc(string name, unsigned char basetype, unsigned char sematic,
                 size_t size, const ChannelMap& channelMapping = ChannelMap()) :
            _name(name),
            _basetype(basetype),
            _semantic(sematic),
            _size(size),
            _channelMapping(channelMapping)
        {
        }

        const string _name;
        const unsigned char _basetype;
        const unsigned char _semantic;
        const size_t _size;
        std::unordered_map<char, int> _channelMapping;
    };

    class TypeDescRegistry
    {
    public:
        TypeDescRegistry()
        {
            // Register all default types.
            newType("boolean", RtTypeDesc::BASETYPE_BOOLEAN);
            newType("integer", RtTypeDesc::BASETYPE_INTEGER);
            newType("float", RtTypeDesc::BASETYPE_FLOAT);
            newType("vector2", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_VECTOR, 2, { {'x', 0}, {'y', 1} });
            newType("vector3", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_VECTOR, 3, { {'x', 0}, {'y', 1}, {'z', 2} });
            newType("vector4", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_VECTOR, 4, { {'x', 0}, {'y', 1}, {'z', 2}, {'w', 3} });
            newType("color2", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_COLOR, 2, { {'r', 0}, {'a', 1} });
            newType("color3", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_COLOR, 3, { {'r', 0}, {'g', 1}, {'b', 2} });
            newType("color4", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_COLOR, 4, { {'r', 0}, {'g', 1}, {'b', 2}, {'a', 3} });
            newType("matrix33", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_MATRIX, 9);
            newType("matrix44", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_MATRIX, 16);
            newType("token", RtTypeDesc::BASETYPE_STRING);
            newType("string", RtTypeDesc::BASETYPE_STRING);
            newType("filename", RtTypeDesc::BASETYPE_STRING, RtTypeDesc::SEMANTIC_FILENAME);
            newType("integerarray", RtTypeDesc::BASETYPE_INTEGER, RtTypeDesc::SEMANTIC_NONE, 0);
            newType("floatarray", RtTypeDesc::BASETYPE_FLOAT, RtTypeDesc::SEMANTIC_NONE, 0);
            newType("BSDF", RtTypeDesc::BASETYPE_NONE, RtTypeDesc::SEMANTIC_CLOSURE);
            newType("EDF", RtTypeDesc::BASETYPE_NONE, RtTypeDesc::SEMANTIC_CLOSURE);
            newType("VDF", RtTypeDesc::BASETYPE_NONE, RtTypeDesc::SEMANTIC_CLOSURE);
            newType("surfaceshader", RtTypeDesc::BASETYPE_NONE, RtTypeDesc::SEMANTIC_SHADER);
            newType("volumeshader", RtTypeDesc::BASETYPE_NONE, RtTypeDesc::SEMANTIC_SHADER);
            newType("displacementshader", RtTypeDesc::BASETYPE_NONE, RtTypeDesc::SEMANTIC_SHADER);
            newType("lightshader", RtTypeDesc::BASETYPE_NONE, RtTypeDesc::SEMANTIC_SHADER);
            newType("auto", RtTypeDesc::BASETYPE_NONE);
        }

        RtTypeDesc* newType(const RtToken& name, unsigned char basetype, unsigned char sematic = RtTypeDesc::SEMANTIC_NONE,
                            size_t size = 1, const ChannelMap& channelMapping = ChannelMap())
        {
            _map[name] = std::make_unique<RtTypeDesc>(name, basetype, sematic, size);

            RtTypeDesc* ptr = _map[name].get();
            for (auto it : channelMapping)
            {
                ptr->setChannelIndex(it.first, it.second);
            }

            return ptr;
        }

        const RtTypeDesc* findType(const RtToken& name)
        {
            auto it = _map.find(name);
            return it != _map.end() ? it->second.get() : nullptr;
        }

        static TypeDescRegistry& get()
        {
            static TypeDescRegistry _registry;
            return _registry;
        }

    private:
        using RtTypeDescPtr = std::unique_ptr<RtTypeDesc>;
        RtTokenMap<RtTypeDescPtr> _map;
    };
}

RtTypeDesc::RtTypeDesc(const string& name, unsigned char basetype, unsigned char semantic, size_t size):
    _ptr(new TypeDesc(name, basetype, semantic, size))
{
}

RtTypeDesc::~RtTypeDesc()
{
    delete static_cast<TypeDesc*>(_ptr);
}

const string& RtTypeDesc::getName() const
{
    return static_cast<TypeDesc*>(_ptr)->_name;
}

unsigned char RtTypeDesc::getBaseType() const
{
    return static_cast<TypeDesc*>(_ptr)->_basetype;
}

unsigned char RtTypeDesc::getSemantic() const
{
    return static_cast<TypeDesc*>(_ptr)->_semantic;
}

size_t RtTypeDesc::getSize() const
{
    return static_cast<TypeDesc*>(_ptr)->_size;
}

int RtTypeDesc::getChannelIndex(char channel) const
{
    TypeDesc* ptr = static_cast<TypeDesc*>(_ptr);
    auto it = ptr->_channelMapping.find(channel);
    return it != ptr->_channelMapping.end() ? it->second : -1;
}

void RtTypeDesc::setChannelIndex(char channel, int index)
{
    static_cast<TypeDesc*>(_ptr)->_channelMapping[channel] = index;
}

RtTypeDesc* RtTypeDesc::registerType(const RtToken& name, unsigned char basetype,
                                     unsigned char semantic, size_t size)
{
    if (TypeDescRegistry::get().findType(name))
    {
        throw ExceptionRuntimeError("A type named '" + name.str() + "' is already registered");
    }
    return TypeDescRegistry::get().newType(name, basetype, semantic, size);
}

const RtTypeDesc* RtTypeDesc::findType(const RtToken& name)
{
    return TypeDescRegistry::get().findType(name);
}

}
