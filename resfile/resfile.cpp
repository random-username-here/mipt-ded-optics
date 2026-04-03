#include "resfile.hpp"
#include <cstdint>
#include <fstream>
#include <optional>
#include <sstream>
#include <string_view>
#include <variant>
#include <iostream>

namespace rsf {

ResFile::LoadersStore &ResFile::GetLoaders() {
    static LoadersStore store;
    return store;
}

void ResFile::LoadFile(std::string_view src) {
    std::string src_str(src);
    std::ifstream ifs(src_str, std::ios::binary | std::ios::ate);
    auto size = ifs.tellg();
    std::vector<char> data(size);
    ifs.seekg(0);
    ifs.read(data.data(), size);
    LoadBuffer(std::string_view(data.data(), size));
}

template<typename T>
T bget(std::istringstream &str) {
    union un { T val; char data[sizeof(T)]; };
    un val;
    str.read(val.data, sizeof(T));
    return val.val;
}

std::string bgetstr(std::istringstream &str) {
    uint32_t len = bget<uint32_t>(str);
    std::string res;
    res.resize(len, '-');
    str.read(res.data(), len);
    return res;
}

void ResFile::LoadBuffer(std::string_view buf) {
    std::string data(buf);
    std::istringstream stream(data);
    
    uint32_t numRes = bget<uint32_t>(stream);
    Params params;
    printf("loading resfile with %zu items\n", (size_t) numRes);
    for (size_t i = 0; i < numRes; ++i) {
        std::string name, type;
        name = bgetstr(stream);
        type = bgetstr(stream);
        uint32_t nfields = bget<uint32_t>(stream);
        printf("\tobject `%s` of type `%s` with %zu fields\n", name.c_str(), type.c_str(), (size_t) nfields);
        for (size_t j = 0; j < nfields; ++j) {
            std::string fname = bgetstr(stream);
            char ftype = bget<char>(stream);
            printf("\t\t%s = (type %c)\n", fname.c_str(), ftype);
            ValType val;
            switch (ftype) {
                case 's': val = bgetstr(stream); break;
                case 'i': val = (int) bget<int32_t>(stream); break;
                case 'f': val = bget<float>(stream); break;
                default: {
                    std::string err = "Bad data type for ";
                    err += fname;
                    err += "@";
                    err += name;
                    err += " : `";
                    err += ftype;
                    err += "`";
                    throw LoadError(err);
                }
            }
            params[fname] = val;
        }
        if (!GetLoaders().count(type)) {
            std::string err = "No loader for type `";
            err += type;
            err += "`";
            throw LoadError(err);
        }
        AddManaged(name, GetLoaders().at(type)(params));
        params.clear();
    }
}

template<typename T>
void bwrite(std::ostream &str, T v) {
    union un { T val; char data[sizeof(T)]; };
    un val;
    val.val = v;
    str.write(val.data, sizeof(T));
}

void bwritestr(std::ostream &str, const std::string &v) {
    bwrite<uint32_t>(str, v.size());
    str.write(v.data(), v.size());
}

void ResFile::Serialize(std::ostream &str) const {
    Params params;
    bwrite<uint32_t>(str, resources.size());
    for (const auto &it : resources) {
        const auto &res = it.second;
        bwritestr(str, it.first);
        bwritestr(str, res->GetType());
        res->Store(params);
        bwrite<uint32_t>(str, params.size());
        for (auto &pi : params) {
            bwritestr(str, pi.first);
            if (std::holds_alternative<int>(pi.second)) {
                bwrite<char>(str, 'i');
                bwrite<int32_t>(str, std::get<int>(pi.second));
            } else if (std::holds_alternative<float>(pi.second)) {
                bwrite<char>(str, 'f');
                bwrite<float>(str, std::get<float>(pi.second));
            } else if (std::holds_alternative<std::string>(pi.second)) {
                bwrite<char>(str, 's');
                bwritestr(str, std::get<std::string>(pi.second));
            }
        }
        params.clear();
    }
}

void ResFile::WriteFile(std::string_view dest) const {
    std::string dest_str(dest);
    std::ofstream ofs(dest_str, std::ios::binary);
    Serialize(ofs);
}

std::optional<int> GetInt(const rsf::ResFile::Params &data, const std::string &key) {
    if (!data.count(key)) return std::nullopt;
    if (std::holds_alternative<int>(data.at(key)))
        return std::get<int>(data.at(key));
    return std::nullopt;

}
std::optional<float> GetFloat(const rsf::ResFile::Params &data, const std::string &key) {
    if (!data.count(key)) return std::nullopt;
    if (std::holds_alternative<int>(data.at(key)))
        return std::get<int>(data.at(key));
    else if (std::holds_alternative<float>(data.at(key)))
        return std::get<float>(data.at(key));
    return std::nullopt;
}

std::optional<std::string_view> GetText(const rsf::ResFile::Params &data, const std::string &key) {
    if (!data.count(key)) return std::nullopt;
    if (std::holds_alternative<std::string>(data.at(key)))
        return std::get<std::string>(data.at(key));
    return std::nullopt;
}


// ------- dicts

Dict::Dict(const ResFile::Params &params) {
    data = params;
}

void Dict::Store(ResFile::Params &params) const {
    for (const auto &item : data)
        params[item.first] = item.second;
}

std::optional<int> Dict::GetInt(const std::string &key) const {
    return rsf::GetInt(data, key);
}

std::optional<float> Dict::GetFloat(const std::string &key) const {
    return rsf::GetFloat(data, key);
}

std::optional<std::string_view> Dict::GetText(const std::string &key) const {
    return rsf::GetText(data, key);
}
REGISTER_RESOURCE(Dict);

};
