#ifndef FONT_DESCRIPTOR_H
#define FONT_DESCRIPTOR_H
// #include <node.h>
// #include <v8.h>
// #include <nan.h>
#include <napi.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// using namespace v8;

enum FontWeight {
  FontWeightUndefined   = 0,
  FontWeightThin        = 100,
  FontWeightUltraLight  = 200,
  FontWeightLight       = 300,
  FontWeightNormal      = 400,
  FontWeightMedium      = 500,
  FontWeightSemiBold    = 600,
  FontWeightBold        = 700,
  FontWeightUltraBold   = 800,
  FontWeightHeavy       = 900
};

enum FontWidth {
  FontWidthUndefined      = 0,
  FontWidthUltraCondensed = 1,
  FontWidthExtraCondensed = 2,
  FontWidthCondensed      = 3,
  FontWidthSemiCondensed  = 4,
  FontWidthNormal         = 5,
  FontWidthSemiExpanded   = 6,
  FontWidthExpanded       = 7,
  FontWidthExtraExpanded  = 8,
  FontWidthUltraExpanded  = 9
};

struct FontDescriptor {
public:
  const char *path;
  const char *postscriptName;
  const char *family;
  const char *style;
  FontWeight weight;
  FontWidth width;
  bool italic;
  bool oblique;
  bool monospace;
  
  FontDescriptor(Napi::Object obj) {
    path = NULL;
    postscriptName = obj.Get("postscriptName").ToString().Utf8Value().c_str();
    family = obj.Get("family").ToString().Utf8Value().c_str();
    style = obj.Get("style").ToString().Utf8Value().c_str();
    weight = (FontWeight) obj.Get("weight").ToNumber().Int64Value();
    width = (FontWidth) obj.Get("width").ToNumber().Int64Value();
    italic = obj.Get("italic").ToBoolean().Value();
    oblique = obj.Get("oblique").ToBoolean().Value();
    monospace = obj.Get("monospace").ToBoolean().Value();
  }

  FontDescriptor() {
    path = NULL;
    postscriptName = NULL;
    family = NULL;
    style = NULL;
    weight = FontWeightUndefined;
    width = FontWidthUndefined;
    italic = false;
    oblique = false;
    monospace = false;
  }
  
  FontDescriptor(const char *path, const char *postscriptName, const char *family, const char *style, 
                 FontWeight weight, FontWidth width, bool italic, bool oblique, bool monospace) {
    this->path = copyString(path);
    this->postscriptName = copyString(postscriptName);
    this->family = copyString(family);
    this->style = copyString(style);
    this->weight = weight;
    this->width = width;
    this->italic = italic;
    this->oblique = oblique;
    this->monospace = monospace;
  }

  FontDescriptor(FontDescriptor *desc) {
    path = copyString(desc->path);
    postscriptName = copyString(desc->postscriptName);
    family = copyString(desc->family);
    style = copyString(desc->style);
    weight = desc->weight;
    width = desc->width;
    italic = desc->italic;
    oblique = desc->oblique;
    monospace = desc->monospace;
  }
  
  ~FontDescriptor() {
    if (path)
      delete path;
    
    if (postscriptName)
      delete postscriptName;
    
    if (family)
      delete family;
    
    if (style)
      delete style;
    
    postscriptName = NULL;
    family = NULL;
    style = NULL;
  }
  
  Napi::Object toJSObject(napi_env env) {
    Napi::Object res = Napi::Object::New(env);
    res.Set("path", Napi::String::New(env, path));
    res.Set("postscriptName", Napi::String::New(env, postscriptName));
    res.Set("family", Napi::String::New(env, family));
    res.Set("style", Napi::String::New(env, style));
    res.Set("weight", Napi::Number::New(env, weight));
    res.Set("width", Napi::Number::New(env, width));
    res.Set("italic", Napi::Boolean::New(env, italic));
    res.Set("oblique", Napi::Boolean::New(env, oblique));
    res.Set("monospace", Napi::Boolean::New(env, monospace));
    return res;
  }
  
private:
  char *copyString(const char *input) {
    if (!input)
      return NULL;

    char *str = new char[strlen(input) + 1];
    strcpy(str, input);
    return str;
  }
  
  // char *getString(napi_env env, Napi::Object obj, const char *name) {
  //   // Nan::HandleScope scope;
  //   // Local<Value> value = obj->Get(Nan::New<String>(name).ToLocalChecked());
  //   Napi::String value = Napi::String::New(env, name);
    
  //   if (value.IsString()) {
  //     return value.Utf8Value().c_str();
  //   }
  
  //   return NULL;
  // }
  
  // int getNumber(Local<Object> obj, const char *name) {
  //   Nan::HandleScope scope;
  //   Local<Value> value = obj->Get(Nan::New<String>(name).ToLocalChecked());
    
  //   if (value->IsNumber()) {
  //     return value->Int32Value();
  //   }
    
  //   return 0;
  // }
  
  // bool getBool(Local<Object> obj, const char *name) {
  //   Nan::HandleScope scope;
  //   Local<Value> value = obj->Get(Nan::New<String>(name).ToLocalChecked());
    
  //   if (value->IsBoolean()) {
  //     return value->BooleanValue();
  //   }
    
  //   return false;
  // }
};

class ResultSet : public std::vector<FontDescriptor *> {
public:
  ~ResultSet() {
    for (ResultSet::iterator it = this->begin(); it != this->end(); it++) {
      delete *it;
    }
  }
};

#endif
