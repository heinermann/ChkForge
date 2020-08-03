#ifndef BWGAME_KOREAN_H
#define BWGAME_KOREAN_H

#include "containers.h"

#include <cstdint>

namespace bwgame {
  namespace korean {
    uint32_t cp949_to_unicode(const char*& ptr, const char* end);
    bool korean_locale_to_utf8(const a_string& src, a_string& dst);
  }
}

#endif
