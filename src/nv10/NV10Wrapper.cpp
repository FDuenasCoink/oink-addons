#include "NV10Wrapper.hpp"

std::thread nativeThreadNv10;
Napi::ThreadSafeFunction tsfnNv10;
static bool isRunningNv10 = true;
static bool threadEndedNv10 = true;

Napi::FunctionReference NV10Wrapper::constructor;

Napi::Object NV10Wrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "NV10", {
    InstanceMethod("connect", &NV10Wrapper::Connect),
    InstanceMethod("checkDevice", &NV10Wrapper::CheckDevice),
    InstanceMethod("startReader", &NV10Wrapper::StartReader),
    InstanceMethod("getBill", &NV10Wrapper::GetBill),
    InstanceMethod("modifyChannels", &NV10Wrapper::ModifyChannels),
    InstanceMethod("stopReader", &NV10Wrapper::StopReader),
    InstanceMethod("reject", &NV10Wrapper::Reject),
    InstanceMethod("testStatus", &NV10Wrapper::TestStatus),
    InstanceMethod("onBill", &NV10Wrapper::OnBill),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("NV10", func);
  return exports;
}

NV10Wrapper::NV10Wrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NV10Wrapper>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length(); 
  if (length != 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }

  Napi::Object params = info[0].As<Napi::Object>();
  if (
    !params.Has("maximumPorts") ||
    !params.Has("logPath") ||
    !params.Has("logLevel")
  ) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }

  Napi::Number MaximumPorts = params.Get("maximumPorts").ToNumber();
  Napi::Number LogLvl = params.Get("logLevel").ToNumber();
  Napi::String LogFilePath = params.Get("logPath").ToString();

  this->nv10Control_ = new NV10ControlClass();
  this->nv10Control_->MaximumPorts = MaximumPorts.Int32Value();
  this->nv10Control_->LogLvl = LogLvl.Uint32Value();
  this->nv10Control_->Path = LogFilePath.Utf8Value();

  this->nv10Control_->InitLog();
}

Napi::Value NV10Wrapper::Connect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->nv10Control_->Connect();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value NV10Wrapper::CheckDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->nv10Control_->CheckDevice();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value NV10Wrapper::StartReader(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->nv10Control_->StartReader();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value NV10Wrapper::GetBill(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  BillError_t response = this->nv10Control_->GetBill();
  Napi::Object object = Napi::Object::New(env);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  object["bill"] = Napi::Number::New(env, response.Bill);
  object["message"] = Napi::String::New(env, response.Message);
  return object;
}

Napi::Value NV10Wrapper::ModifyChannels(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !info[0].IsNumber()) {
     Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  Napi::Number InhibitMask1 = info[0].As<Napi::Number>();

  Response_t response = this->nv10Control_->ModifyChannels(InhibitMask1.Int32Value());
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value NV10Wrapper::StopReader(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  isRunningNv10 = false;
  Response_t response = this->nv10Control_->StopReader();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value NV10Wrapper::Reject(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->nv10Control_->Reject();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value NV10Wrapper::TestStatus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  TestStatus_t response = this->nv10Control_->TestStatus();
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


Napi::Value NV10Wrapper::OnBill(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !info[0].IsFunction()) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  
  Napi::Function napiFunction = info[0].As<Napi::Function>();
  tsfnNv10 = Napi::ThreadSafeFunction::New(
    env, 
    napiFunction, 
    "Callback", 
    0, 
    1,
    []( Napi::Env ) {
      nativeThreadNv10.join();
    });

  nativeThreadNv10 = std::thread ( [this] {
    auto callback = [](Napi::Env env, Napi::Function jsCallback, BillError_t* bill) {
      Napi::Object object = Napi::Object::New(env);
      object["statusCode"] = Napi::Number::New(env, bill->StatusCode);
      object["bill"] = Napi::Number::New(env, bill->Bill);
      object["message"] = Napi::String::New(env, bill->Message);
      jsCallback.Call({object});
      delete bill;
    };
    isRunningNv10 = true;
    threadEndedNv10 = false;
    while (isRunningNv10) {
      BillError_t response = this->nv10Control_->GetBill();
      if (response.StatusCode == 302) continue;
      BillError_t *value = new BillError_t(response);
      napi_status status = tsfnNv10.BlockingCall(value, callback);
      if ( status != napi_ok ) break;
      std::this_thread::sleep_for( std::chrono::milliseconds(10));
    }
    threadEndedNv10 = true;
    tsfnNv10.Release();
  });

  auto finishFn = [] (const Napi::CallbackInfo& info) {
    isRunningNv10 = false;
    while (!threadEndedNv10);
    std::this_thread::sleep_for( std::chrono::milliseconds(50));
    return;
  };

  return Napi::Function::New(env, finishFn);
}