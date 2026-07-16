#ifndef TYPES_H
#define TYPES_H

// ==== Return values ====
enum class retv {
    Ok      = 0,
    Fail    = 1,
    Timeout = 2,
    NonValideData = 3,
};

enum class ledTypes{
    correct = 0,
    error   = 1,
};


#define BIT(n) (1 << (n))
#define CHECK(expr) do { retv r = (expr); if(r != retv::Ok) return r; } while(0)
#define IMPULSE_DURATION_FOR_RELE_MS 100

#endif