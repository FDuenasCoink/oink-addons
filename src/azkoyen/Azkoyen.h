#include <napi.h>
#include "AzkoyenControl.hpp"

using namespace AzkoyenControl;

class Azkoyen : public Napi::ObjectWrap<Azkoyen> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    Azkoyen(const Napi::CallbackInfo& info);
  private:
    static Napi::FunctionReference constructor;
    Napi::Value Connect(const Napi::CallbackInfo& info);
    Napi::Value CheckDevice(const Napi::CallbackInfo& info);
    Napi::Value StartReader(const Napi::CallbackInfo& info);
    Napi::Value GetCoin(const Napi::CallbackInfo& info);
    Napi::Value GetLostCoins(const Napi::CallbackInfo& info);
    Napi::Value ModifyChannels(const Napi::CallbackInfo& info);
    Napi::Value StopReader(const Napi::CallbackInfo& info);
    Napi::Value ResetDevice(const Napi::CallbackInfo& info);
    Napi::Value TestStatus(const Napi::CallbackInfo& info);
    Napi::Value CleanDevice(const Napi::CallbackInfo& info);
    AzkoyenControlClass *azkoyenControl_;
};