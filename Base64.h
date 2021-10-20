// Copyright (c) Elucid Bioimaging

#ifndef BASE64_H
#define BASE64_H

#include <string.h>
#include <string>

/**
 *  \brief base64 encoding of strings
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup cap
 **/

class Base64
{
public:
  explicit Base64();
  std::string base64_encode(std::string bytes_to_encode);

private:

  std::string base64_chars;

  static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
  }
};

#endif // BASE64_H
