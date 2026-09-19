#include "Scene.h"
#include "Disk.h"
#include "Sphere.h"
#include "Triangle.h"

#include <cctype>
#include <cmath>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{
    struct XmlElement
    {
        std::string name;
        std::map<std::string, std::string> attributes;
        std::vector<XmlElement> children;
    };

    class XmlParser
    {
    public:
        explicit XmlParser(const std::string& text)
            : m_text(text)
        {
        }

        XmlElement Parse()
        {
            SkipIgnored();
            XmlElement root = ParseElement();
            SkipIgnored();
            if (!AtEnd())
                Fail("unexpected content after the root element");
            return root;
        }

    private:
        bool AtEnd() const
        {
            return m_position >= m_text.size();
        }

        bool StartsWith(const char* value) const
        {
            const std::string_view expected(value);
            return m_text.compare(m_position, expected.size(), expected) == 0;
        }

        void Fail(const std::string& message) const
        {
            throw std::runtime_error(
                "XML parse error at offset " +
                std::to_string(m_position) + ": " + message);
        }

        void Expect(char expected)
        {
            if (AtEnd() || m_text[m_position] != expected)
                Fail(std::string("expected '") + expected + "'");
            ++m_position;
        }

        void SkipWhitespace()
        {
            while (!AtEnd() && std::isspace(static_cast<unsigned char>(m_text[m_position])))
                ++m_position;
        }

        void SkipUntil(const char* terminator, const char* description)
        {
            const std::size_t terminator_position = m_text.find(terminator, m_position);
            if (terminator_position == std::string::npos)
                Fail(std::string("unterminated ") + description);
            m_position = terminator_position + std::char_traits<char>::length(terminator);
        }

        void SkipIgnored()
        {
            bool skipped = true;
            while (skipped)
            {
                skipped = false;
                SkipWhitespace();
                if (StartsWith("<!--"))
                {
                    m_position += 4;
                    SkipUntil("-->", "comment");
                    skipped = true;
                }
                else if (StartsWith("<?"))
                {
                    m_position += 2;
                    SkipUntil("?>", "processing instruction");
                    skipped = true;
                }
            }
        }

        std::string ParseName()
        {
            if (AtEnd() || !(std::isalpha(static_cast<unsigned char>(m_text[m_position])) ||
                              m_text[m_position] == '_'))
            {
                Fail("expected a name");
            }

            const std::size_t start = m_position++;
            while (!AtEnd())
            {
                const unsigned char character = static_cast<unsigned char>(m_text[m_position]);
                if (!(std::isalnum(character) || character == '_' || character == '-' || character == '.'))
                    break;
                ++m_position;
            }
            return m_text.substr(start, m_position - start);
        }

        std::string ParseAttributeValue()
        {
            if (AtEnd() || (m_text[m_position] != '\'' && m_text[m_position] != '"'))
                Fail("expected a quoted attribute value");

            const char quote = m_text[m_position++];
            const std::size_t start = m_position;
            while (!AtEnd() && m_text[m_position] != quote)
                ++m_position;
            if (AtEnd())
                Fail("unterminated attribute value");

            const std::string value = m_text.substr(start, m_position - start);
            ++m_position;
            return value;
        }

        XmlElement ParseElement()
        {
            Expect('<');
            if (StartsWith("/") || StartsWith("!") || StartsWith("?"))
                Fail("unexpected markup");

            XmlElement element;
            element.name = ParseName();

            while (true)
            {
                SkipWhitespace();
                if (StartsWith("/>") )
                {
                    m_position += 2;
                    return element;
                }
                if (!AtEnd() && m_text[m_position] == '>')
                {
                    ++m_position;
                    break;
                }

                const std::string attribute_name = ParseName();
                if (!element.attributes.emplace(
                        attribute_name,
                        std::string{}).second)
                {
                    Fail("duplicate attribute '" + attribute_name + "'");
                }
                SkipWhitespace();
                Expect('=');
                SkipWhitespace();
                element.attributes[attribute_name] = ParseAttributeValue();
            }

            while (true)
            {
                SkipIgnored();
                if (StartsWith("</"))
                {
                    m_position += 2;
                    const std::string closing_name = ParseName();
                    SkipWhitespace();
                    Expect('>');
                    if (closing_name != element.name)
                        Fail("closing tag does not match '" + element.name + "'");
                    return element;
                }

                if (AtEnd())
                    Fail("unterminated element '" + element.name + "'");

                if (m_text[m_position] == '<')
                {
                    element.children.emplace_back(ParseElement());
                    continue;
                }

                const std::size_t text_start = m_position;
                while (!AtEnd() && m_text[m_position] != '<')
                    ++m_position;
                for (std::size_t i = text_start; i < m_position; ++i)
                {
                    if (!std::isspace(static_cast<unsigned char>(m_text[i])))
                        Fail("non-whitespace text is not supported");
                }
            }
        }

        const std::string& m_text;
        std::size_t m_position = 0;
    };

    const std::string& RequireAttribute(
        const XmlElement& element,
        const std::string& name)
    {
        const auto iterator = element.attributes.find(name);
        if (iterator == element.attributes.end())
        {
            throw std::runtime_error(
                "Element '" + element.name + "' is missing attribute '" + name + "'");
        }
        return iterator->second;
    }

    float ParseFloatAttribute(
        const XmlElement& element,
        const std::string& name)
    {
        const std::string& value = RequireAttribute(element, name);
        std::size_t parsed_length = 0;
        float result = 0.0f;
        try
        {
            result = std::stof(value, &parsed_length);
        }
        catch (const std::exception&)
        {
            throw std::runtime_error(
                "Element '" + element.name + "' has an invalid float attribute '" + name + "'");
        }

        if (parsed_length != value.size() || !std::isfinite(result))
        {
            throw std::runtime_error(
                "Element '" + element.name + "' has an invalid float attribute '" + name + "'");
        }
        return result;
    }

    glm::vec3 ParseVectorAttribute(
        const XmlElement& element,
        const std::string& name)
    {
        const std::string& value = RequireAttribute(element, name);
        std::istringstream stream(value);
        glm::vec3 result;
        if (!(stream >> result.x >> result.y >> result.z))
        {
            throw std::runtime_error(
                "Element '" + element.name + "' has an invalid vec3 attribute '" + name + "'");
        }

        std::string extra_value;
        if (stream >> extra_value ||
            !std::isfinite(result.x) ||
            !std::isfinite(result.y) ||
            !std::isfinite(result.z))
        {
            throw std::runtime_error(
                "Element '" + element.name + "' has an invalid vec3 attribute '" + name + "'");
        }
        return result;
    }
}

Scene::~Scene() = default;

void Scene::LoadSceneFromXML(const std::string& file_path)
{
    std::ifstream file(file_path, std::ios::binary);
    if (!file)
        throw std::runtime_error("Unable to open scene file: " + file_path);

    std::ostringstream contents;
    contents << file.rdbuf();
    const XmlElement root = XmlParser(contents.str()).Parse();
    if (root.name != "scene")
        throw std::runtime_error("Scene XML root element must be 'scene'");

    Scene loaded_scene;
    for (const XmlElement& object_element : root.children)
    {
        if (object_element.name != "object")
            throw std::runtime_error(
                "Scene element must contain only 'object' children");

        SceneObject* object = loaded_scene.CreateSceneObject(
            ParseVectorAttribute(object_element, "position"),
            ParseVectorAttribute(object_element, "euler"),
            ParseFloatAttribute(object_element, "scale"));

        if (object_element.children.empty())
            throw std::runtime_error("Scene object must contain at least one primitive");

        for (const XmlElement& primitive_element : object_element.children)
        {
            if (primitive_element.name == "sphere")
            {
                object->CreatePrimitive<Sphere>(
                    ParseFloatAttribute(primitive_element, "radius"));
            }
            else if (primitive_element.name == "disk")
            {
                object->CreatePrimitive<Disk>(
                    ParseFloatAttribute(primitive_element, "radius"));
            }
            else if (primitive_element.name == "triangle")
            {
                object->CreatePrimitive<Triangle>(
                    ParseVectorAttribute(primitive_element, "v0"),
                    ParseVectorAttribute(primitive_element, "v1"),
                    ParseVectorAttribute(primitive_element, "v2"));
            }
            else
            {
                throw std::runtime_error(
                    "Unknown primitive element: '" + primitive_element.name + "'");
            }
        }
    }

    m_sceneObjects.swap(loaded_scene.m_sceneObjects);
}

SceneObject* Scene::CreateSceneObject(
    const glm::vec3& position,
    const glm::vec3& euler,
    float scale)
{
    auto object = std::make_unique<SceneObject>(position, euler, scale);
    SceneObject* object_pointer = object.get();
    m_sceneObjects.emplace_back(std::move(object));
    return object_pointer;
}

bool Scene::Intersect(
    const Ray& ray,
    Intersection& intersection) const
{
    float closest_t = ray.maxT;
    bool has_hit = false;

    for (const std::unique_ptr<SceneObject>& object : m_sceneObjects)
    {
        Intersection candidate{};
        if (!object->Intersect(ray, candidate) || candidate.t >= closest_t)
            continue;

        closest_t = candidate.t;
        intersection = candidate;
        has_hit = true;
    }

    return has_hit;
}
