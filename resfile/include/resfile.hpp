#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <optional>

namespace rsf {

class Resource;
template<typename T>
class ResourceImpl;

/**
 * Error while loading resource
 */
class LoadError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

/**
 * A resource file which contains resources
 */
class ResFile {


public:
    using ValType = std::variant<std::string, float, int>;
    using Params = std::unordered_map<std::string, ValType>;
    using ResourceLoader = std::function<std::unique_ptr<Resource>(const Params &)>;
    using LoadersStore = std::unordered_map<std::string, ResourceLoader>;

private:

    std::unordered_map<std::string, Resource*> resources;
    std::vector<std::unique_ptr<Resource>> managed;
    static LoadersStore &GetLoaders();

public:

    template<typename T>
    static void RegisterType(const char *name) {
        GetLoaders()[name] = [](const Params& params) {
            return std::make_unique<T>(params);
        };
    }

    void LoadFile(std::string_view src);
    void LoadBuffer(std::string_view buf);
    void Serialize(std::ostream &os) const;
    void WriteFile(std::string_view dest) const;

    void Add(std::string_view name, Resource *res) {
        resources[std::string(name)] = res;
    }

    void AddManaged(std::string_view name, std::unique_ptr<Resource> res) {
        Add(name, res.get());
        managed.push_back(std::move(res));
    }

    void Unmanage(Resource *res) {
        for (size_t i = 0; i < managed.size(); ++i) {
            if (managed[i].get() == res) {
                (void) managed[i].release();
                managed.erase(managed.begin() + i);
                return;
            }
        }
    }

    Resource *GetUntyped(const std::string &name) {
        if (resources.count(name))
            return resources[name];
        else return nullptr;
    }

    template<typename T>
    T *Get(const std::string &name) {
        Resource *res = GetUntyped(name);
        return dynamic_cast<T*>(res);
    }

    std::vector<std::string> GetKeys() const {
        std::vector<std::string> keys;
        keys.reserve(resources.size());
        for (const auto &it : resources)
            keys.push_back(it.first);
        return keys;
    }
};

class Resource {
public:
    virtual const char *GetType() const = 0;
    virtual void Store(ResFile::Params &params) const = 0;
};

/**
 * One of resources from resource file.
 */
template<typename T>
class ResourceImpl : public virtual Resource {
    // constructor(const ResFile::Params &params);
    // static const char *Type() = 0;

public:
    const char *GetType() const override { return T::Type(); }

    T *As() {
        return dynamic_cast<T>(this);
    }
};

std::optional<int> GetInt(const rsf::ResFile::Params &data, const std::string &key);
std::optional<float> GetFloat(const rsf::ResFile::Params &data, const std::string &key);
std::optional<std::string_view> GetText(const rsf::ResFile::Params &data, const std::string &key);

/**
 * A dictionary with keys and values
 */
class Dict : public ResourceImpl<Dict> {
    ResFile::Params data;
public:
    static const char *Type() { return "dict"; } 
    Dict() = default;
    Dict(const ResFile::Params &params);
    virtual void Store(ResFile::Params &params) const override;

    std::optional<int> GetInt(const std::string &key) const;
    std::optional<float> GetFloat(const std::string &key) const;
    std::optional<std::string_view> GetText(const std::string &key) const;
};

template<typename Type>
class ResourceTypeRegistration {
public: 
    ResourceTypeRegistration()  {
        ResFile::RegisterType<Type>(Type::Type());
    }
};

#define REGISTER_RESOURCE(type) static rsf::ResourceTypeRegistration<type> _res_##type;

};
