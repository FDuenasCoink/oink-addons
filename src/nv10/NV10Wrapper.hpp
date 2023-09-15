#include <napi.h>
#include <thread>
#include <chrono>
#include "NV10Control.hpp"

using namespace NV10Control;

class NV10Wrapper : public Napi::ObjectWrap<NV10Wrapper> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NV10Wrapper(const Napi::CallbackInfo& info);
  private:
    static Napi::FunctionReference constructor;
    Napi::Value Connect(const Napi::CallbackInfo& info);
    Napi::Value CheckDevice(const Napi::CallbackInfo& info);
    Napi::Value StartReader(const Napi::CallbackInfo& info);
    Napi::Value GetBill(const Napi::CallbackInfo& info);
    Napi::Value ModifyChannels(const Napi::CallbackInfo& info);
    Napi::Value StopReader(const Napi::CallbackInfo& info);
    Napi::Value Reject(const Napi::CallbackInfo& info);
    Napi::Value TestStatus(const Napi::CallbackInfo& info);
    Napi::Value OnBill(const Napi::CallbackInfo& info);
    NV10ControlClass *nv10Control_;
};