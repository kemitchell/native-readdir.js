#ifndef NATIVE_EXTENSION_GRAB_H
#define NATIVE_EXTENSION_GRAB_H

#include <nan.h>
#include <dirent.h>
#include <sys/types.h>

// Wrapped constructible object that encapsulates a `DIR` pointer
// used to call `readdir(3)`.
class Directory : public Nan::ObjectWrap {
public:
    static NAN_MODULE_INIT(Init);

private:
    explicit Directory(char* path);
    ~Directory();

    static NAN_METHOD(New);
    static Nan::Persistent<v8::Function> constructor;

    // Wraps `opendir(3)`
    static NAN_METHOD(Open);

    // Wraps `readdir(3)`
    static NAN_METHOD(Read);

    // Wraps `closedir(3)`
    static NAN_METHOD(Close);

    // C-string for the directory path. This is the argument to
    // `opendir(3)`. The object keeps it around in case it needs to
    // throw an error using NAN's `ErrnoException` helper.
    char* path;

    // The return value of `opendir(3)`, which is passed repeatedly to
    // `readdir(3)` and eventually to `closedir(3)`.
    DIR* directory;
};

#endif
