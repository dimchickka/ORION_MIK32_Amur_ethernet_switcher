#ifndef TYPES_H
#define TYPES_H

// ==== Return values ====
enum class retv {
    Ok      = 0,
    Fail    = 1,
    Timeout = 2,
};

enum class ledTypes{
    correct = 0,
    error   = 1,
};


#define BIT(n) (1 << (n))

#endif