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
            throw std::runtime_error("XML parse error at offset " + std::to_string(m_position) + ": " + message);
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
                if (!element.attributes.emplace(attribute_name, std::string{}).second)
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

    const std::string& RequireAttribute(const XmlElement& element, const std::string& name)
    {
        const auto iterator = element.attributes.find(name);
        if (iterator == element.attributes.end())
        {
            throw std::runtime_error("Element '" + element.name + "' is missing attribute '" + name + "'");
        }
        return iterator->second;
    }

    float ParseFloatAttribute(const XmlElement& element, const std::string& name)
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
            throw std::runtime_error("Element '" + element.name + "' has an invalid float attribute '" + name + "'");
        }

        if (parsed_length != value.size() || !std::isfinite(result))
        {
            throw std::runtime_error("Element '" + element.name + "' has an invalid float attribute '" + name + "'");
        }
        return result;
    }

    glm::vec3 ParseVectorAttribute(const XmlElement& element, const std::string& name)
    {
        const std::string& value = RequireAttribute(element, name);
        std::istringstream stream(value);
        glm::vec3 result;
        if (!(stream >> result.x >> result.y >> result.z))
        {
            throw std::runtime_error("Element '" + element.name + "' has an invalid vec3 attribute '" + name + "'");
        }

        std::string extra_value;
        if (stream >> extra_value || !std::isfinite(result.x) || !std::isfinite(result.y) || !std::isfinite(result.z))
        {
            throw std::runtime_error("Element '" + element.name + "' has an invalid vec3 attribute '" + name + "'");
        }
        return result;
    }

    Color ParseColorAttribute(const XmlElement& element, const std::string& name)
    {
        const glm::vec3 value = ParseVectorAttribute(element, name);
        return Color{value.x, value.y, value.z};
    }

    bool HasAttribute(const XmlElement& element, const std::string& name)
    {
        return element.attributes.find(name) != element.attributes.end();
    }

    float ParseOptionalFloatAttribute(const XmlElement& element, const std::string& name, float default_value)
    {
        return HasAttribute(element, name) ? ParseFloatAttribute(element, name) : default_value;
    }

    glm::vec3 ParseOptionalVectorAttribute(const XmlElement& element, const std::string& name, const glm::vec3& default_value)
    {
        return HasAttribute(element, name) ? ParseVectorAttribute(element, name) : default_value;
    }

    glm::vec3 ParseAttenuations(const XmlElement& element)
    {
        if (HasAttribute(element, "attenuations"))
            return ParseVectorAttribute(element, "attenuations");
        return glm::vec3(
            ParseFloatAttribute(element, "quadratic"),
            ParseFloatAttribute(element, "linear"),
            ParseFloatAttribute(element, "constant"));
    }

    void LoadLight(Scene& scene, const XmlElement& element)
    {
        if (!element.children.empty())
            throw std::runtime_error("Light element must not contain children");

        if (element.name == "directional" || element.name == "directional_light")
        {
            scene.CreateLight<DirectionalLight>(
                ParseVectorAttribute(element, "direction"),
                ParseColorAttribute(element, "radiance"));
        }
        else if (element.name == "point" || element.name == "point_light")
        {
            scene.CreateLight<PointLight>(
                ParseVectorAttribute(element, "position"),
                ParseColorAttribute(element, "intensity"),
                ParseAttenuations(element));
        }
        else if (element.name == "spot" || element.name == "spot_light")
        {
            scene.CreateLight<SpotLight>(
                ParseVectorAttribute(element, "direction"),
                ParseVectorAttribute(element, "position"),
                ParseColorAttribute(element, "intensity"),
                ParseFloatAttribute(element, "inner_angle"),
                ParseFloatAttribute(element, "outer_angle"),
                ParseAttenuations(element));
        }
        else
        {
            throw std::runtime_error("Unknown light element: '" + element.name + "'");
        }
    }

    void LoadObject(Scene& scene, const XmlElement& element)
    {
        if (element.name != "object")
            throw std::runtime_error("Unknown object element: '" + element.name + "'");

        SceneObject* object = scene.CreateSceneObject(
            ParseOptionalVectorAttribute(element, "position", glm::vec3(0.0f)),
            ParseOptionalVectorAttribute(element, "euler", glm::vec3(0.0f)),
            ParseOptionalFloatAttribute(element, "scale", 1.0f));
        if (element.children.empty())
            throw std::runtime_error("Scene object must contain at least one primitive");

        for (const XmlElement& primitive_element : element.children)
        {
            if (primitive_element.name == "sphere")
                object->CreatePrimitive<Sphere>(ParseFloatAttribute(primitive_element, "radius"));
            else if (primitive_element.name == "disk")
                object->CreatePrimitive<Disk>(ParseFloatAttribute(primitive_element, "radius"));
            else if (primitive_element.name == "triangle")
                object->CreatePrimitive<Triangle>(
                    ParseVectorAttribute(primitive_element, "v0"),
                    ParseVectorAttribute(primitive_element, "v1"),
                    ParseVectorAttribute(primitive_element, "v2"));
            else
                throw std::runtime_error("Unknown primitive element: '" + primitive_element.name + "'");
        }
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
    if (HasAttribute(root, "version") && RequireAttribute(root, "version") != "2")
        throw std::runtime_error("Unsupported scene XML version");

    Scene loaded_scene;
    const bool has_legacy_camera = HasAttribute(root, "camera_position") ||
                                   HasAttribute(root, "camera_forward") ||
                                   HasAttribute(root, "camera_up") ||
                                   HasAttribute(root, "camera_fov");
    if (has_legacy_camera)
    {
        loaded_scene.m_cameraSettings.position = ParseVectorAttribute(root, "camera_position");
        loaded_scene.m_cameraSettings.forward = ParseVectorAttribute(root, "camera_forward");
        loaded_scene.m_cameraSettings.up = ParseVectorAttribute(root, "camera_up");
        loaded_scene.m_cameraSettings.verticalFov = ParseFloatAttribute(root, "camera_fov");
    }

    bool has_camera_element = false;
    bool has_lights_element = false;
    bool has_objects_element = false;
    for (const XmlElement& element : root.children)
    {
        if (element.name == "camera")
        {
            if (has_legacy_camera || has_camera_element || !element.children.empty())
                throw std::runtime_error("Scene must contain one empty camera element");
            has_camera_element = true;
            loaded_scene.m_cameraSettings.position = ParseOptionalVectorAttribute(element, "position", loaded_scene.m_cameraSettings.position);
            loaded_scene.m_cameraSettings.forward = ParseOptionalVectorAttribute(element, "forward", loaded_scene.m_cameraSettings.forward);
            loaded_scene.m_cameraSettings.up = ParseOptionalVectorAttribute(element, "up", loaded_scene.m_cameraSettings.up);
            loaded_scene.m_cameraSettings.verticalFov = ParseOptionalFloatAttribute(element, "fov", loaded_scene.m_cameraSettings.verticalFov);
        }
        else if (element.name == "lights")
        {
            if (has_lights_element)
                throw std::runtime_error("Scene must contain at most one lights element");
            has_lights_element = true;
            for (const XmlElement& light_element : element.children)
                LoadLight(loaded_scene, light_element);
        }
        else if (element.name == "objects")
        {
            if (has_objects_element)
                throw std::runtime_error("Scene must contain at most one objects element");
            has_objects_element = true;
            for (const XmlElement& object_element : element.children)
                LoadObject(loaded_scene, object_element);
        }
        else if (element.name == "directional_light" || element.name == "point_light" || element.name == "spot_light")
        {
            LoadLight(loaded_scene, element);
        }
        else if (element.name == "object")
            LoadObject(loaded_scene, element);
        else
            throw std::runtime_error("Unknown scene element: '" + element.name + "'");
    }

    m_sceneObjects.swap(loaded_scene.m_sceneObjects);
    m_lights.swap(loaded_scene.m_lights);
    m_cameraSettings = loaded_scene.m_cameraSettings;
}

const SceneCameraSettings& Scene::GetCameraSettings() const
{
    return m_cameraSettings;
}

const std::vector<std::unique_ptr<Light>>& Scene::GetLights() const
{
    return m_lights;
}

SceneObject* Scene::CreateSceneObject(const glm::vec3& position, const glm::vec3& euler, float scale)
{
    auto object = std::make_unique<SceneObject>(position, euler, scale);
    SceneObject* object_pointer = object.get();
    m_sceneObjects.emplace_back(std::move(object));
    return object_pointer;
}

bool Scene::Intersect(const Ray& ray, Intersection& intersection) const
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
