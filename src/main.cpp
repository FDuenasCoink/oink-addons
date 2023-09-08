#include <napi.h>
#include "Pelicano.h"

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return Pelicano::Init(env, exports);
}

NODE_API_MODULE(pelicano, InitAll);