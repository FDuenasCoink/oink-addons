#include <napi.h>
#include "azkoyen/Azkoyen.h"
#include "pelicano/Pelicano.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  Pelicano::Init(env, exports);
  Azkoyen::Init(env, exports);
  return exports;
}

NODE_API_MODULE(oink_addons, InitAll);