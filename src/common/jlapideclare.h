#pragma once
#define DEF(RET, NAME, ARGS) extern RET(*NAME)ARGS;
#include "jlapi.tbl"
#undef DEF
}
