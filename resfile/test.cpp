#include "include/resfile.hpp"
#include "resfile.cpp"
#include <iostream>

int main(int argc, const char *argv[]) {
    const char *file = "example.res";
    if (argc >= 2)
        file = argv[1];
    rsf::ResFile rsf;
    rsf.LoadFile(file);
    rsf::Dict *dict = rsf.Get<rsf::Dict>("sub.other");
    std::cout << dict->GetText("foo").value_or("<no key>") << '\n';
    std::cout << dict->GetText("key").value_or("<no key>") << '\n';
    rsf.WriteFile("copy.res");
    return 0;
}
