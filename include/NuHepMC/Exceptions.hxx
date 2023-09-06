#pragma once

#include <stdexcept>

namespace NuHepMC {
struct except : public std::exception {
  std::stringstream msgstrm;
  std::string msg;
  except() : msgstrm(), msg() {}
  except(except const &other) : msgstrm(), msg() {
    msgstrm << other.msg;
    msg = other.msg;
  }
  const char *what() const noexcept { return msg.c_str(); }

  template <typename T> except &operator<<(T const &obj) {
    msgstrm << obj;
    msg = msgstrm.str();
    return (*this);
  }
};

} // namespace NuHepMC

#define NEW_NuHepMC_EXCEPT(EXCEPT_NAME)                                        \
  struct EXCEPT_NAME : public NuHepMC::except {                                \
    EXCEPT_NAME() : NuHepMC::except() {}                                       \
    EXCEPT_NAME(EXCEPT_NAME const &other) : except(other) {}                   \
    template <typename T> EXCEPT_NAME &operator<<(T const &obj) {              \
      msgstrm << obj;                                                          \
      msg = std::string("[") + #EXCEPT_NAME + "]: " + msgstrm.str();           \
      return (*this);                                                          \
    }                                                                          \
  }
