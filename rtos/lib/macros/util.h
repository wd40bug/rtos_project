#ifndef UTIL_H
#define UTIL_H

#define NULL_GUARD(type, var) \
  type var##_backup;\
  if (var == NULL) {\
    var = &var##_backup;\
  }

#endif /* ifndef UTIL_H */
