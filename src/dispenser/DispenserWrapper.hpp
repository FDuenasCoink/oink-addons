#include <napi.h>
#include "DispenserControl.hpp"

using namespace DispenserControl;

class DispenserWrapper : public Napi::ObjectWrap<DispenserWrapper> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    DispenserWrapper(const Napi::CallbackInfo& info);
  private:
    static Napi::FunctionReference constructor;
    Napi::Value Connect(const Napi::CallbackInfo& info);
    Napi::Value CheckDevice(const Napi::CallbackInfo& info);
    Napi::Value DispenseCard(const Napi::CallbackInfo& info);
    Napi::Value RecycleCard(const Napi::CallbackInfo& info);
    Napi::Value EndProcess(const Napi::CallbackInfo& info);
    Napi::Value GetDispenserFlags(const Napi::CallbackInfo& info);
    Napi::Value TestStatus(const Napi::CallbackInfo& info);
    DispenserControlClass *dispenserControl_;
};