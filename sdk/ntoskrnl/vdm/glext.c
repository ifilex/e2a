#include "winebox.h"

#if defined(BOXEDWINE_OPENGL_SDL) || defined(BOXEDWINE_OPENGL_ES)
#include GLH
#include "glcommon.h"

#include "pbl.h"

PblMap* glFunctionMap;

char* glIsLoaded[GL_FUNC_COUNT];

void glExtensionsLoaded() {
    void* pfn = NULL;
    U32 i;

    glFunctionMap = pblMapNewHashMap();

#undef GL_FUNCTION
#define GL_FUNCTION(func, RET, PARAMS, ARGS, PRE, POST, LOG) glIsLoaded[func]="gl"#func;

#undef GL_FUNCTION_CUSTOM
#define GL_FUNCTION_CUSTOM(func, RET, PARAMS) glIsLoaded[func]="gl"#func;

#undef GL_EXT_FUNCTION
#define GL_EXT_FUNCTION(func, RET, PARAMS) if (ext_gl##func) glIsLoaded[func]="gl"#func;

#include "glfunctions.h"
    for (i=0;i<GL_FUNC_COUNT;i++) {
        if (glIsLoaded[i])
            pblMapAdd(glFunctionMap, glIsLoaded[i], strlen(glIsLoaded[i])+1, &pfn, sizeof(void*));
    }

}
#endif