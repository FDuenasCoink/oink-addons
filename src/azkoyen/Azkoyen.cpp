#include "Azkoyen.h"

Napi::FunctionReference Azkoyen::constructor;

Napi::Object Azkoyen::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "Azkoyen", {
    InstanceMethod("connect", &Azkoyen::Connect),
    InstanceMethod("checkDevice", &Azkoyen::CheckDevice),
    InstanceMethod("startReader", &Azkoyen::StartReader),
    InstanceMethod("getCoin", &Azkoyen::GetCoin),
    InstanceMethod("getLostCoins", &Azkoyen::GetLostCoins),
    InstanceMethod("modifyChannels", &Azkoyen::ModifyChannels),
    InstanceMethod("stopReader", &Azkoyen::StopReader),
    InstanceMethod("resetDevice", &Azkoyen::ResetDevice),
    InstanceMethod("testStatus", &Azkoyen::TestStatus),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Azkoyen", func);
  return exports;
}

Azkoyen::Azkoyen(const Napi::CallbackInfo& info) : Napi::ObjectWrap<Azkoyen>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length(); 
  if (length != 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }

  Napi::Object params = info[0].As<Napi::Object>();
  if (
    !params.Has("warnToCritical") || 
    !params.Has("maxCritical") ||
    !params.Has("maximumPorts") ||
    !params.Has("logPath") ||
    !params.Has("logLevel")
  ) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  
  Napi::Number WarnToCritical = params.Get("warnToCritical").ToNumber();
  Napi::Number MaxCritical = params.Get("maxCritical").ToNumber();
  Napi::Number MaximumPorts = params.Get("maximumPorts").ToNumber();
  Napi::Number LogLvl = params.Get("logLevel").ToNumber();
  Napi::String LogFilePath = params.Get("logPath").ToString();

  this->azkoyenControl_ = new AzkoyenControlClass();
  this->azkoyenControl_->WarnToCritical = WarnToCritical.Int32Value();
  this->azkoyenControl_->MaxCritical = MaxCritical.Int32Value();
  this->azkoyenControl_->MaximumPorts = MaximumPorts.Int32Value();
  this->azkoyenControl_->LogLvl = LogLvl.Uint32Value();
  this->azkoyenControl_->Path = LogFilePath.Utf8Value();

  this->azkoyenControl_->InitLog();
}

Napi::Value Azkoyen::Connect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->azkoyenControl_->Connect();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Azkoyen::CheckDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->azkoyenControl_->CheckDevice();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Azkoyen::StartReader(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->azkoyenControl_->StartReader();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Azkoyen::GetCoin(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  CoinError_t response = this->azkoyenControl_->GetCoin();
  Napi::Object object = Napi::Object::New(env);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  object["event"] = Napi::Number::New(env, response.Event);
  object["coin"] = Napi::Number::New(env, response.Coin);
  object["message"] = Napi::String::New(env, response.Message);
  object["remaining"] = Napi::Number::New(env, response.Remaining);
  return object;
}

Napi::Value Azkoyen::GetLostCoins(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  CoinLost_t response = this->azkoyenControl_->GetLostCoins();
  Napi::Object object = Napi::Object::New(env);
  object["50"] = Napi::Number::New(env, response.CoinCinc);
  object["100"] = Napi::Number::New(env, response.CoinCien);
  object["200"] = Napi::Number::New(env, response.CoinDosc);
  object["500"] = Napi::Number::New(env, response.CoinQuin);
  object["1000"] = Napi::Number::New(env, response.CoinMil);
  return object;
}

Napi::Value Azkoyen::ModifyChannels(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
     Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  Napi::Number InhibitMask1 = info[0].As<Napi::Number>();
  Napi::Number InhibitMask2 = info[1].As<Napi::Number>();

  Response_t response = this->azkoyenControl_->ModifyChannels(InhibitMask1.Int32Value(), InhibitMask2.Int32Value());
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Azkoyen::StopReader(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->azkoyenControl_->StopReader();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Azkoyen::ResetDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->azkoyenControl_->ResetDevice();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value Azkoyen::TestStatus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  TestStatus_t response = this->azkoyenControl_->TestStatus();
  Napi::Object object = Napi::Object::New(env);
  object["version"] = Napi::String::New(env, response.Version);
  object["device"] = Napi::Number::New(env, response.Device);
  object["errorType"] = Napi::Number::New(env, response.ErrorType);
  object["errorCode"] = Napi::Number::New(env, response.ErrorCode);
  object["message"] = Napi::String::New(env, response.Message);
  object["aditionalInfo"] = Napi::String::New(env, response.AditionalInfo);
  object["priority"] = Napi::Number::New(env, response.Priority);
  return object;
}

Napi::Value Azkoyen::CleanDevice(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  return Napi::Number::New(env, 0);
}
