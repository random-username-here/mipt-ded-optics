#include "cum/manager.hpp"
#include "cum/plugin.hpp"
#include <iostream>
#include <cstring>

int main(int argc, const char *argv[]) {

    if (argc != 2) {
        std::cout << "Expected a plugin file\n";
        return -1;
    }
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        std::cout << "Plugin inspector\n";
        std::cout << "Usage: " << argv[0] << " plugin-file.so\n";
        std::cout << "Will try to load that plugin, and get info about it\n";
        return 0;
    }

    cum::Manager manager;

    cum::Plugin *plugin = nullptr;
    try {
        plugin = manager.LoadFromFile(argv[1]);
    } catch (const cum::Manager::LoadError &ex) {
        std::cout << "Failed to load plugin: " << ex.what() << "\n";
        return -1;
    }

    std::cout << "Name           : " << plugin->GetName() << '\n';
    std::cout << "Id             : " << plugin->GetIdentifier() << '\n';
    std::cout << "Description    : " << plugin->GetDescription() << '\n';
    std::cout << "Dependencies   : ";
    for (auto i : plugin->GetDependencies())
        std::cout << i << " ";
    std::cout << "\n";
    std::cout << "Conflicts with : ";
    for (auto i : plugin->GetConflicts())
        std::cout << i << " ";
    std::cout << "\n";

    return 0;
}
