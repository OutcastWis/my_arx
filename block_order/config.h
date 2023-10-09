#pragma once

#ifdef BLOCKORDER_EXPORTS
#define DLLIMPEXP __declspec(dllexport)
#else
#define DLLIMPEXP __declspec( dllimport )
#endif
