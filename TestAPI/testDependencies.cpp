#include "FreeImage.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
int main() {
    const unsigned count=FreeImage_GetDependenciesCount(); if(count<4) return 1;
    FI_DEPENDENCY_INFO info={}; bool png=false;
    for(unsigned i=0;i<count;++i) {
        if(!FreeImage_GetDependencyInfo(i,&info) || !info.name || !*info.name) return 2;
        std::printf("%s: compiled=%s runtime=%s source=%s\n",info.name,info.compiled_version ? info.compiled_version : "unknown",info.runtime_version ? info.runtime_version : "unknown",info.system_library ? "system" : "bundled");
        if(!std::strcmp(info.name,"libpng")) { png=true; if(!info.compiled_version || !info.runtime_version) return 3; }
        FI_DEPENDENCY_INFO second={}; if(!FreeImage_GetDependencyInfo(i,&second) || info.name!=second.name) return 4;
    }
    if(!png || FreeImage_GetDependencyInfo(count,&info) || FreeImage_GetDependencyInfo(0,NULL)) return 5;
}
