#ifndef UNLIKELY_H
#define UNLIKELY_H

#if defined(__GNUC__) && __GNUC__ >= 3
#define unlikely(x)	__builtin_expect(!!(x), 0)
#else
#define unlikely(x) !!(x)
#endif

#endif //UNLIKELY_H
