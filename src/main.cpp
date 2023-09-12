#include <napi.h>
#include "azkoyen/Azkoyen.hpp"
#include "pelicano/Pelicano.h"
#include "dispenser/DispenserWrapper.hpp"
#include "nv10/NV10Wrapper.hpp"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Pelicano::Init(env, exports);
  Azkoyen::Init(env, exports);
  DispenserWrapper::Init(env, exports);
  NV10Wrapper::Init(env, exports);
  return exports;
}

NODE_API_MODULE(oink_addons, InitAll);