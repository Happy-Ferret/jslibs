
#pragma once

#define DEF(RET, NAME, ARGS) RET(*NAME)ARGS;
struct JLApi {
#include "jlapi.tbl"
};
#undef DEF

#define DEF(RET, NAME, ARGS) RET(*NAME)ARGS;
#include "jlapi.tbl"
#undef DEF

void ImportJLApi(const JLApi &api) {
#define DEF(RET, NAME, ARGS) NAME = api.NAME;
#include "jlapi.tbl"
#undef DEF
}
