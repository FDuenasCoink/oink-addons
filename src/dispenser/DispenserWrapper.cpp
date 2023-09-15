#include "DispenserWrapper.hpp"

std::thread nativeThreadDispenser;
Napi::ThreadSafeFunction tsfnDispenser;
static bool isRunningDispenser = true;
static bool threadEndedDispenser = true;

Napi::FunctionReference DispenserWrapper::constructor;

Napi::Object DispenserWrapper::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "Dispenser", {
    InstanceMethod("connect", &DispenserWrapper::Connect),
    InstanceMethod("checkDevice", &DispenserWrapper::CheckDevice),
    InstanceMethod("dispenseCard", &DispenserWrapper::DispenseCard),
    InstanceMethod("recycleCard", &DispenserWrapper::RecycleCard),
    InstanceMethod("endProcess", &DispenserWrapper::EndProcess),
    InstanceMethod("getDispenserFlags", &DispenserWrapper::GetDispenserFlags),
    InstanceMethod("testStatus", &DispenserWrapper::TestStatus),
    InstanceMethod("onDispense", &DispenserWrapper::OnDispense),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("Dispenser", func);
  return exports;
}

DispenserWrapper::DispenserWrapper(const Napi::CallbackInfo& info) : Napi::ObjectWrap<DispenserWrapper>(info)  {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length(); 
  if (length != 1 || !info[0].IsObject()) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }

  Napi::Object params = info[0].As<Napi::Object>();
  if (
    !params.Has("maxInitAttempts") || 
    !params.Has("shortTime") ||
    !params.Has("longTime") ||
    !params.Has("maximumPorts") ||
    !params.Has("logPath") ||
    !params.Has("logLevel")
  ) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  
  Napi::Number MaxInitAttempts = params.Get("maxInitAttempts").ToNumber();
  Napi::Number ShortTime = params.Get("shortTime").ToNumber();
  Napi::Number LongTime = params.Get("longTime").ToNumber();
  Napi::Number MaximumPorts = params.Get("maximumPorts").ToNumber();
  Napi::Number LogLvl = params.Get("logLevel").ToNumber();
  Napi::String LogFilePath = params.Get("logPath").ToString();

  this->dispenserControl_ = new DispenserControlClass();
  this->dispenserControl_->Path = LogFilePath.Utf8Value();
  this->dispenserControl_->LogLvl = LogLvl.Uint32Value();
  this->dispenserControl_->MaximumPorts = MaximumPorts.Int32Value();
  this->dispenserControl_->MaxInitAttempts = MaxInitAttempts.Int32Value();
  this->dispenserControl_->ShortTime = ShortTime.Int32Value();
  this->dispenserControl_->LongTime = LongTime.Int32Value();

  this->dispenserControl_->InitLog();
}

Napi::Value DispenserWrapper::Connect(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->dispenserControl_->Connect();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value DispenserWrapper::CheckDevice(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->dispenserControl_->CheckDevice();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value DispenserWrapper::DispenseCard(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Response_t response = this->dispenserControl_->DispenseCard();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value DispenserWrapper::RecycleCard(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  isRunningDispenser = false;
  Response_t response = this->dispenserControl_->RecycleCard();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value DispenserWrapper::EndProcess(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  isRunningDispenser = false;
  Response_t response = this->dispenserControl_->EndProcess();
  Napi::Object object = Napi::Object::New(env);
  object["message"] = Napi::String::New(env, response.Message);
  object["statusCode"] = Napi::Number::New(env, response.StatusCode);
  return object;
}

Napi::Value DispenserWrapper::GetDispenserFlags(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Flags_t response = this->dispenserControl_->GetDispenserFlags();
  Napi::Object object = Napi::Object::New(env);
  object["rficCardInG"] = Napi::Boolean::New(env, response.RFICCardInG);
  object["recyclingBoxF"] = Napi::Boolean::New(env, response.RecyclingBoxF);
  object["cardInG"] = Napi::Boolean::New(env, response.CardInG);
  object["cardsInD"] = Napi::Boolean::New(env, response.CardsInD);
  object["dispenserF"] = Napi::Boolean::New(env, response.DispenserF);
  return object;
}

Napi::Value DispenserWrapper::TestStatus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  TestStatus_t response = this->dispenserControl_->TestStatus();
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

Napi::Value DispenserWrapper::OnDispense(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 1 || !info[0].IsFunction()) {
    Napi::TypeError::New(env, "Invalid params").ThrowAsJavaScriptException();
  }
  
  Napi::Function napiFunction = info[0].As<Napi::Function>();
  tsfnDispenser = Napi::ThreadSafeFunction::New(
    env, 
    napiFunction, 
    "Callback", 
    0, 
    1,
    []( Napi::Env ) {
      nativeThreadDispenser.join();
    });

  nativeThreadDispenser = std::thread ( [this] {
    auto callback = [](Napi::Env env, Napi::Function jsCallback, Response_t* status) {
      Napi::Object object = Napi::Object::New(env);
      object["statusCode"] = Napi::Number::New(env, status->StatusCode);
      object["message"] = Napi::String::New(env, status->Message);
      jsCallback.Call({object});
      delete status;
    };
    isRunningDispenser = true;
    threadEndedDispenser = false;
    while (isRunningDispenser) {
      std::this_thread::sleep_for( std::chrono::milliseconds(10));
      Response_t response = this->dispenserControl_->CheckDevice();
      if (response.StatusCode == 301) continue;
      Response_t *value = new Response_t(response);
      napi_status status = tsfnDispenser.BlockingCall(value, callback);
      if ( status != napi_ok ) break;
      isRunningDispenser = false;
      break;
    }
    threadEndedDispenser = true;
    tsfnDispenser.Release();
  });

  auto finishFn = [] (const Napi::CallbackInfo& info) {
    isRunningDispenser = false;
    while (!threadEndedDispenser);
    std::this_thread::sleep_for( std::chrono::milliseconds(50));
    return;
  };

  return Napi::Function::New(env, finishFn);
}
