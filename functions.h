#ifndef NATIVE_EXTENSION_GRAB_H
#define NATIVE_EXTENSION_GRAB_H

#include <nan.h>
#include <sys/types.h> // for DIR

class Directory : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);

private:
    explicit Directory(char* path);
    ~Directory();

    static NAN_METHOD(New);
    static NAN_METHOD(Open);
    static NAN_METHOD(Read);
    static NAN_METHOD(Close);
    static Nan::Persistent<v8::Function> constructor;

    char* path;
    DIR* directory;
};

#endif
