#include "functions.h"
#include <dirent.h>
#include <sys/types.h>

Nan::Persistent<v8::Function> Directory::constructor;

NAN_MODULE_INIT(Directory::Init) {
    v8::Local<v8::FunctionTemplate> functionTemplate =
        Nan::New<v8::FunctionTemplate>(New);
    functionTemplate->SetClassName(Nan::New("Directory").ToLocalChecked());
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(functionTemplate, "open", Open);
    Nan::SetPrototypeMethod(functionTemplate, "read", Read);
    Nan::SetPrototypeMethod(functionTemplate, "close", Close);

    constructor.Reset(Nan::GetFunction(functionTemplate).ToLocalChecked());
    Nan::Set(
        target,
        Nan::New("Directory").ToLocalChecked(),
        Nan::GetFunction(functionTemplate).ToLocalChecked()
    );
}

Directory::Directory(char* path) : path(path) {}

Directory::~Directory() {}

NAN_METHOD(Directory::New) {
    if (info.IsConstructCall()) {
        if (info[0]->IsUndefined()) {
            Nan::ThrowError("Missing path argument");
        } else {
            // See https://github.com/nodejs/nan/issues/464 on conversion
            // to C-string.
            Nan::Utf8String pathString(info[0]);
            Directory* object = new Directory(*pathString);
            object->Wrap(info.This());
            info.GetReturnValue().Set(info.This());
        }
    } else {
        const int argc = 1;
        v8::Local<v8::Value> argv[argc] = {info[0]};
        v8::Local<v8::Function> cons = Nan::New(constructor);
        info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
}

NAN_METHOD(Directory::Open) {
    Directory* object = Nan::ObjectWrap::Unwrap<Directory>(info.This());
    char* path = object->path;
    errno = 0;
    DIR* directory = opendir(path);
    if (directory == NULL) {
        Nan::ThrowError(
            Nan::ErrnoException(
                errno,
                "readdir",
                NULL,
                NULL // TODO directory
            )
        );
    } else {
        object->directory = directory;
        info.GetReturnValue().Set(Nan::True());
    }
}

NAN_METHOD(Directory::Read) {
    Directory* object = Nan::ObjectWrap::Unwrap<Directory>(info.This());
    DIR* directory = object->directory;
    if (directory == NULL) {
        Nan::ThrowError("no open DIR");
        return;
    }
    errno = 0;
    struct dirent* entry = readdir(directory);
    if (entry == NULL) {
        if (errno == 0) {
            info.GetReturnValue().Set(Nan::Null());
        } else {
            Nan::ThrowError(
                Nan::ErrnoException(
                    errno,
                    "readdir",
                    NULL,
                    NULL // TODO directory
                )
            );
        }
    } else {
        info.GetReturnValue().Set(
            Nan::New(entry->d_name).ToLocalChecked()
        );
    }
}

NAN_METHOD(Directory::Close) {
    Directory* object = Nan::ObjectWrap::Unwrap<Directory>(info.This());
    DIR* directory = object->directory;
    if (directory == NULL) {
        Nan::ThrowError("no open DIR");
        return;
    }
    errno = 0;
    // Return `true` on successful close.
    if (closedir(directory) == 0) {
        info.GetReturnValue().Set(Nan::True());
    } else {
        Nan::ThrowError(
            Nan::ErrnoException(
                errno,
                "readdir",
                NULL,
                NULL // TODO directory
            )
        );
    }
}
