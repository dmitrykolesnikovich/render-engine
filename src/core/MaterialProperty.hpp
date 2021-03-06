//
// Created by Krisu on 2020/3/6.
//

#ifndef RENDER_ENGINE_MATERIALPROPERTY_HPP
#define RENDER_ENGINE_MATERIALPROPERTY_HPP

#include <variant>

#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Texture.hpp"

class Shader;

namespace {
    /* Number represent GL_TEXTUREx used for glActiveTexture */
    enum class MaterialPropertyType {
        Albedo    = 0,
        Normal    = 1,
        Specular  = 2,
        Metallic  = 3,
        Roughness = 4,
        Emissive  = 5,
        Ao        = 6,
        Height    = 7,
        /* used to access number of element in enum */
        count
    };

    constexpr const unsigned MaterialPropertyTypeCount
            = static_cast<unsigned>(MaterialPropertyType::count);

    /* PBR_Material uniform name */
    constexpr const char* mp_uniform_str[]
            {
                    "m_albedo",
                    "m_normal",
                    "m_specular",
                    "m_metallic",
                    "m_roughness",
                    "m_emissive",
                    "m_ao",
                    "m_height"
            };

    /* This assert will fail if number of mp_uniform_str's element not equal to
     * MaterialPropertyType's element */
    static_assert(sizeof(mp_uniform_str)/sizeof(const char*) ==
                  static_cast<int>(MaterialPropertyType::count));
};


class MaterialProperty {
public:
    /* Setting property from application side (PBR_Material) */
    void SetProperty(float x, float y, float z) {
        SetProperty(glm::vec3(x, y, z));
    }

    template <typename T>
    void SetProperty(T value) {
        static_assert(std::is_convertible_v<T, float> ||
                      std::is_convertible_v<T, Texture*> ||
                      std::is_convertible_v<T, glm::vec3>,
                      "Failed to set variant, not convertible type to float/vec3/Texture*");
        property = value;
    }

    /* Setting shader uniform */
    void UpdateShader(Shader & shader, MaterialPropertyType type) {
        auto texture_unit = static_cast<unsigned>(type);
        std::string const& mp_name = mp_uniform_str[texture_unit];
        auto index = property.index();
        shader.UseShaderProgram();
        if (index == 0) {
            shader.SetWithoutCheck(mp_name + ".value",
                                   std::get<0>(property));
            shader.SetWithoutCheck(mp_name + ".use_map", false);
        } else if (index == 1) {
            shader.SetWithoutCheck(mp_name + ".value",
                                   std::get<1>(property));
            shader.SetWithoutCheck(mp_name + ".use_map", false);
        } else {
            Texture * texture = std::get<2>(property);
            glActiveTexture(GL_TEXTURE0 + texture_unit);
            glBindTexture(static_cast<unsigned>(texture->Type()), texture->ID());
            shader.SetWithoutCheck(mp_name + ".map", texture_unit);
            shader.SetWithoutCheck(mp_name + ".use_map", true);
            // glBindTexture(static_cast<unsigned>(texture->Type()), 0);
        }
    }

private:
    std::variant<float, glm::vec3, Texture*> property;
};


#endif //RENDER_ENGINE_MATERIALPROPERTY_HPP
