#include <stdlib.h>
#include <napi.h>
#include "FontDescriptor.h"

#include <iostream>

// using namespace v8;

// these functions are implemented by the platform
ResultSet *getAvailableFonts();
ResultSet *findFonts(FontDescriptor *);
FontDescriptor *findFont(FontDescriptor *);
FontDescriptor *substituteFont(char *, char *);

// converts a ResultSet to a JavaScript array
Napi::Array collectResults(napi_env env, ResultSet *results) {
  // Nan::EscapableHandleScope scope;
  // Local<Array> res = Nan::New<Array>(results->size());

  Napi::Array res = Napi::Array::New(env, results->size());
  int i = 0;
  for (ResultSet::iterator it = results->begin(); it != results->end(); it++) {
    res.Set(i++, (*it)->toJSObject(env));
  }

  delete results;
  return res;
  // return scope.Escape(res);
}

// converts a FontDescriptor to a JavaScript object
Napi::Object wrapResult(napi_env env, FontDescriptor *result) {
  // Nan::EscapableHandleScope scope;
  if (result == NULL)
    return Napi::Object::New(env);
    // return scope.Escape(Nan::Null());

  // Local<Object> res = result->toJSObject();
  Napi::Object res = result->toJSObject(env);
  delete result;
  return res;
  // return scope.Escape(res);
}

// holds data about an operation that will be
// performed on a background thread
// struct AsyncRequest {
//   uv_work_t work;
//   FontDescriptor *desc;     // used by findFont and findFonts
//   char *postscriptName;     // used by substituteFont
//   char *substitutionString; // ditto
//   FontDescriptor *result;   // for functions with a single result
//   ResultSet *results;       // for functions with multiple results
//   Nan::Callback *callback;  // the actual JS callback to call when we are done

//   AsyncRequest(Local<Value> v) {
//     work.data = (void *)this;
//     callback = new Nan::Callback(v.As<Function>());
//     desc = NULL;
//     postscriptName = NULL;
//     substitutionString = NULL;
//     result = NULL;
//     results = NULL;
//   }

//   ~AsyncRequest() {
//     delete callback;

//     if (desc)
//       delete desc;

//     if (postscriptName)
//       delete postscriptName;

//     if (substitutionString)
//       delete substitutionString;

//     // result/results deleted by wrapResult/collectResults respectively
//   }
// };

// calls the JavaScript callback for a request
// void asyncCallback(uv_work_t *work) {
//   Nan::HandleScope scope;
//   AsyncRequest *req = (AsyncRequest *) work->data;
//   Nan::AsyncResource async("asyncCallback");
//   Local<Value> info[1];

//   if (req->results) {
//     info[0] = collectResults(req->results);
//   } else if (req->result) {
//     info[0] = wrapResult(req->result);
//   } else {
//     info[0] = Nan::Null();
//   }

//   req->callback->Call(1, info, &async);
//   delete req;
// }

// void getAvailableFontsAsync(uv_work_t *work) {
//   AsyncRequest *req = (AsyncRequest *) work->data;
//   req->results = getAvailableFonts();
// }

Napi::ThreadSafeFunction napi_thread;
std::thread *worker_thread = nullptr;

void worker() {
  auto callback = []( Napi::Env env, 
    Napi::Function jsCallback) {
    jsCallback.Call({ collectResults(env, getAvailableFonts()) });
  };

  napi_thread.NonBlockingCall( callback );
}

void FinalizerCallback(Napi::Env env) {
  if (worker_thread && worker_thread->joinable())
    worker_thread->join();
}

template<bool async>
Napi::Value getAvailableFonts(const Napi::CallbackInfo& info) {
  if (async) {
    napi_thread = Napi::ThreadSafeFunction::New(info.Env(),
                              info[0].As<Napi::Function>(),
                              "getAvailableFonts",
                              0,
                              1,
                              FinalizerCallback);
    worker_thread = new std::thread(&worker);
    return info.Env().Undefined();
  } else {
    return collectResults(info.Env(), getAvailableFonts());
  }
}

// void findFontsAsync(uv_work_t *work) {
//   AsyncRequest *req = (AsyncRequest *) work->data;
//   req->results = findFonts(req->desc);
// }

template<bool async>
// NAN_METHOD(findFonts) {
Napi::Value findFonts(const Napi::CallbackInfo& info) {
  // if (info.Length() < 1 || !info[0]->IsObject() || info[0]->IsFunction())
  //   return Nan::ThrowTypeError("Expected a font descriptor");

  // Local<Object> desc = info[0].As<Object>();
  Napi::Object desc = info[0].ToObject();
  FontDescriptor *descriptor = new FontDescriptor(desc);

  // if (async) {
  //   if (info.Length() < 2 || !info[1]->IsFunction())
  //     return Nan::ThrowTypeError("Expected a callback");

  //   AsyncRequest *req = new AsyncRequest(info[1]);
  //   req->desc = descriptor;
  //   uv_queue_work(uv_default_loop(), &req->work, findFontsAsync, (uv_after_work_cb) asyncCallback);

  //   return;
  // } else {
  Napi::Array res = collectResults(info.Env(), findFonts(descriptor));
  delete descriptor;
  return res;
  // }
}

// void findFontAsync(uv_work_t *work) {
//   AsyncRequest *req = (AsyncRequest *) work->data;
//   req->result = findFont(req->desc);
// }

template<bool async>
// NAN_METHOD(findFont) {
Napi::Value findFont(const Napi::CallbackInfo& info) {
  // if (info.Length() < 1 || !info[0]->IsObject() || info[0]->IsFunction())
  //   return Nan::ThrowTypeError("Expected a font descriptor");

  // Local<Object> desc = info[0].As<Object>();
  Napi::Object desc = info[0].ToObject();
  FontDescriptor *descriptor = new FontDescriptor(desc);

  // if (async) {
  //   if (info.Length() < 2 || !info[1]->IsFunction())
  //     return Nan::ThrowTypeError("Expected a callback");

  //   AsyncRequest *req = new AsyncRequest(info[1]);
  //   req->desc = descriptor;
  //   uv_queue_work(uv_default_loop(), &req->work, findFontAsync, (uv_after_work_cb) asyncCallback);

  //   return;
  // } else {
  Napi::Object res = wrapResult(info.Env(), findFont(descriptor));
  delete descriptor;
  return res;
  // }
}

// void substituteFontAsync(uv_work_t *work) {
//   AsyncRequest *req = (AsyncRequest *) work->data;
//   req->result = substituteFont(req->postscriptName, req->substitutionString);
// }

template<bool async>
// NAN_METHOD(substituteFont) {
Napi::Value substituteFont(const Napi::CallbackInfo& info) {
  // if (info.Length() < 1 || !info[0]->IsString())
  //   return Nan::ThrowTypeError("Expected postscript name");

  // if (info.Length() < 2 || !info[1]->IsString())
  //   return Nan::ThrowTypeError("Expected substitution string");

  // Nan::Utf8String postscriptName(info[0]);
  // Nan::Utf8String substitutionString(info[1]);

  // if (async) {
  //   if (info.Length() < 3 || !info[2]->IsFunction())
  //     return Nan::ThrowTypeError("Expected a callback");

  //   // copy the strings since the JS garbage collector might run before the async request is finished
  //   char *ps = new char[postscriptName.length() + 1];
  //   strcpy(ps, *postscriptName);

  //   char *sub = new char[substitutionString.length() + 1];
  //   strcpy(sub, *substitutionString);

  //   AsyncRequest *req = new AsyncRequest(info[2]);
  //   req->postscriptName = ps;
  //   req->substitutionString = sub;
  //   uv_queue_work(uv_default_loop(), &req->work, substituteFontAsync, (uv_after_work_cb) asyncCallback);

  //   return;
  // } else {
  // info.GetReturnValue().Set(wrapResult(substituteFont(*postscriptName, *substitutionString)));
  return wrapResult(info.Env(), substituteFont((char*)info[0].ToString().Utf8Value().c_str(), (char*)info[1].ToString().Utf8Value().c_str()));
  // }
}

void Init(Napi::Env env, Napi::Object exports)
{
	exports.Set(
		Napi::String::New(env, "getAvailableFonts"),
		Napi::Function::New(env, getAvailableFonts<true>));
	exports.Set(
		Napi::String::New(env, "getAvailableFontsSync"),
		Napi::Function::New(env, getAvailableFonts<false>));
	exports.Set(
		Napi::String::New(env, "findFonts"),
		Napi::Function::New(env, findFonts<true>));
	exports.Set(
		Napi::String::New(env, "findFontsSync"),
		Napi::Function::New(env, findFonts<false>));
	exports.Set(
		Napi::String::New(env, "findFont"),
		Napi::Function::New(env, findFont<true>));
	exports.Set(
		Napi::String::New(env, "findFontSync"),
		Napi::Function::New(env, findFont<false>));
	exports.Set(
		Napi::String::New(env, "substituteFont"),
		Napi::Function::New(env, substituteFont<true>));
	exports.Set(
		Napi::String::New(env, "substituteFontSync"),
		Napi::Function::New(env, substituteFont<false>));
}

Napi::Object main_node(Napi::Env env, Napi::Object exports) {
    Init(env, exports);
    return exports;
}

NODE_API_MODULE(node_fontmanager, main_node)
