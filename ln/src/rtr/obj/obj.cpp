#include "rtr/obj/obj.h"
#include "resfile.hpp"
#include "rtr/renderer.h"

namespace rtr {

Object::~Object() {}

void Object::setSelected(bool selected) {
    if (selected == m_isSelected) return;
    m_isSelected = selected;
    m_scene->onSelectionChange.trigger();
}

vec3f maskvec(vec3f a, vec3f b, int mask) {
    return vec3f(
        (mask & 1) ? b.x() : a.x(),
        (mask & 2) ? b.y() : a.y(),
        (mask & 4) ? b.z() : a.z()
    );
}

void Object::debugDraw(SceneAnnotater &annot) const {
    if (!isSelected())
        return;
    AABB box = bbox();
#define LINE_COLOR rgbf(1, 1, 1)
#define MLINE(a, b)\
        annot.line(\
            maskvec(box.min, box.max, a),\
            maskvec(box.min, box.max, b),\
            LINE_COLOR\
        )
    MLINE(0b000, 0b001);
    MLINE(0b000, 0b010);
    MLINE(0b000, 0b100);

    MLINE(0b100, 0b110);
    MLINE(0b100, 0b101);
    MLINE(0b010, 0b110);
    MLINE(0b010, 0b011);
    MLINE(0b001, 0b101);
    MLINE(0b001, 0b011);

    MLINE(0b111, 0b110);
    MLINE(0b111, 0b101);
    MLINE(0b111, 0b011);
#undef MLINE
}

size_t Object::mg_idIdx = 0;

#define FFIELDS\
    X("obj.mat.ambient", ambient)\
    X("obj.mat.diffuse", diffuse)\
    X("obj.mat.specular", specular)\
    X("obj.mat.emission", emission)\
    X("obj.mat.shinyness", shinyness)\
    X("obj.mat.reflection", reflection)\
    X("obj.mat.refraction", refraction)\
    X("obj.mat.refractiveIndex", refractiveIndex)\

Object::Object(const rsf::ResFile::Params &params) {
    m_material.baseColor = rgbf(
        rsf::GetFloat(params, "obj.mat.baseColor.r").value_or(1),
        rsf::GetFloat(params, "obj.mat.baseColor.g").value_or(1),
        rsf::GetFloat(params, "obj.mat.baseColor.b").value_or(1)
    );
    m_material.lightScalePerM = rgbf(
        rsf::GetFloat(params, "obj.mat.scale.r").value_or(1),
        rsf::GetFloat(params, "obj.mat.scale.g").value_or(1),
        rsf::GetFloat(params, "obj.mat.scale.b").value_or(1)
    );
    m_name = rsf::GetText(params, "obj.name").value_or("<no name>");
#define X(name, param)\
        m_material.param = rsf::GetFloat(params, name).value_or(0);
    FFIELDS
#undef X
}

void Object::Store(rsf::ResFile::Params &params) const {
    params["obj.name"] = m_name;
    params["obj.mat.baseColor.r"] = m_material.baseColor.r();
    params["obj.mat.baseColor.g"] = m_material.baseColor.g();
    params["obj.mat.baseColor.b"] = m_material.baseColor.b();
    params["obj.mat.scale.r"] = m_material.lightScalePerM.r();
    params["obj.mat.scale.g"] = m_material.lightScalePerM.g();
    params["obj.mat.scale.b"] = m_material.lightScalePerM.b();
#define X(name, param)\
        params[name] = m_material.param;
    FFIELDS
#undef X

}

};
