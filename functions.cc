#include "functions.h"
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>

using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::GetFunction;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using Nan::Set;
using v8::Function;
using v8::FunctionTemplate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

Nan::Persistent<v8::Function> Directory::constructor;

NAN_MODULE_INIT(Directory::Init) {
    v8::Local<v8::FunctionTemplate> functionTemplate =
        Nan::New<v8::FunctionTemplate>(New);
    functionTemplate->SetClassName(Nan::New("Directory").ToLocalChecked());
    functionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(functionTemplate, "open", Directory::Open);
    Nan::SetPrototypeMethod(functionTemplate, "read", Directory::Read);
    Nan::SetPrototypeMethod(functionTemplate, "close", Directory::Close);

    constructor.Reset(Nan::GetFunction(functionTemplate).ToLocalChecked());
    Nan::Set(
        target,
        Nan::New("Directory").ToLocalChecked(),
        Nan::GetFunction(functionTemplate).ToLocalChecked()
    );
}

Directory::Directory(Nan::Utf8String* path) : path(path) {}

Directory::~Directory() {
    delete path;
}

// Constructor

NAN_METHOD(Directory::New) {
    if (info.IsConstructCall()) {
        if (info[0]->IsUndefined()) {
            Nan::ThrowError("Missing path argument");
        } else {
            // See https://github.com/nodejs/nan/issues/464 on conversion
            // to C-string.
            Directory* object = new Directory(new Nan::Utf8String(info[0]));
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

// Open

OpenWorker::OpenWorker(Directory* directory, Callback* callback)
    : AsyncWorker(callback), directory(directory) {
    result = NULL;
    saved_errno = 0;
}

OpenWorker::~OpenWorker () { }

void OpenWorker::Execute () {
    errno = 0;
    result = opendir(**directory->path);
    saved_errno = errno;
}

void OpenWorker::HandleOKCallback () {
    HandleScope scope;
    Local<Value> argv[1];
    if (result == NULL) {
        argv[0] = Nan::ErrnoException(
                      saved_errno,
                      "opendir",
                      NULL,
                      **directory->path
                  );
    } else {
        directory->stream = result;
        argv[0] = Null();
    }
    callback->Call(1, argv);
};

NAN_METHOD(Directory::Open) {
    Directory* object = Nan::ObjectWrap::Unwrap<Directory>(info.This());
    Callback* callback = new Callback(info[0].As<Function>());
    AsyncQueueWorker(new OpenWorker(object, callback));
}

// Read

ReadWorker::ReadWorker(Directory* directory, Callback* callback)
    : AsyncWorker(callback), directory(directory) {
    result = NULL;
    saved_errno = 0;
}

ReadWorker::~ReadWorker () { }

void ReadWorker::Execute() {
    errno = 0;
    result = readdir(directory->stream);
    saved_errno = errno;
}

void ReadWorker::HandleOKCallback () {
    HandleScope scope;
    if (result == NULL) {
        if (saved_errno == 0) {
            Local<Value> argv[] = {
                Null()
            };
            callback->Call(1, argv);
        } else {
            Local<Value> argv[] = {
                Nan::ErrnoException(
                    saved_errno,
                    "readdir",
                    NULL,
                    **directory->path
                )
            };
            callback->Call(1, argv);
        }
    } else {
        Local<Value> argv[] = {
            Null(),
            Nan::New(result->d_name).ToLocalChecked()
        };
        callback->Call(2, argv);
    }
};

NAN_METHOD(Directory::Read) {
    Directory* object = Nan::ObjectWrap::Unwrap<Directory>(info.This());
    Callback* callback = new Callback(info[0].As<Function>());
    AsyncQueueWorker(new ReadWorker(object, callback));
}

// Close

CloseWorker::CloseWorker(Directory* directory, Callback* callback)
    : AsyncWorker(callback), directory(directory) {
    result = 0;
    saved_errno = 0;
}

CloseWorker::~CloseWorker () { }

void CloseWorker::Execute () {
    errno = 0;
    result = closedir(directory->stream);
    saved_errno = errno;
};

void CloseWorker::HandleOKCallback () {
    HandleScope scope;
    if (result == 0) {
        Local<Value> argv[1] = {
            Null()
        };
        directory->stream = NULL;
        callback->Call(1, argv);
    } else {
        Local<Value> argv[1] = {
            Nan::ErrnoException(
                saved_errno,
                "closedir",
                NULL,
                **directory->path
            )
        };
        callback->Call(1, argv);
    }
};

NAN_METHOD(Directory::Close) {
    Directory* object = Nan::ObjectWrap::Unwrap<Directory>(info.This());
    Callback* callback = new Callback(info[0].As<Function>());
    AsyncQueueWorker(new CloseWorker(object, callback));
}
