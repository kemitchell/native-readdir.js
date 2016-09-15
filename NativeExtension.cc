#include "functions.h"

using v8::FunctionTemplate;

NAN_MODULE_INIT(InitAll) {
    Directory::Init(target);
}

NODE_MODULE(NativeExtension, InitAll)
