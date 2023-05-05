#pragma once

#ifndef _DEBUG
#define ENG_DEBUG(fmt, ...)
#else
#define ENG_DEBUG(fmt, ...) printf("[%s %s:%i] " fmt "\n", __FILE__, __func__, __LINE__, __VA_ARGS__)
#endif