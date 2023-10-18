#include "UnionAfx.h"
#include "resource.h"

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;

#include "Macro.h"
#include "Utility/Common.h"
#include "Application.h"

#ifdef __G1
#define NAMESPACE Gothic_I_Classic
#define ENGINE Engine_G1
#define ZENDEF(g1, g1a, g2, g2a) g1
#include "Utility/Engine.h"
#include "Options.h"
#include "Includes.h"
#undef ZENDEF
#undef ENGINE
#undef NAMESPACE
#endif

#ifdef __G1A
#define NAMESPACE Gothic_I_Addon
#define ENGINE Engine_G1A
#define ZENDEF(g1, g1a, g2, g2a) g1a
#include "Utility/Engine.h"
#include "Options.h"
#include "Includes.h"
#undef ZENDEF
#undef ENGINE
#undef NAMESPACE
#endif

#ifdef __G2
#define NAMESPACE Gothic_II_Classic
#define ENGINE Engine_G2
#define ZENDEF(g1, g1a, g2, g2a) g2
#include "Utility/Engine.h"
#include "Options.h"
#include "Includes.h"
#undef ZENDEF
#undef ENGINE
#undef NAMESPACE
#endif

#ifdef __G2A
#define NAMESPACE Gothic_II_Addon
#define ENGINE Engine_G2A
#define ZENDEF(g1, g1a, g2, g2a) g2a
#include "Utility/Engine.h"
#include "Options.h"
#include "Includes.h"
#undef ZENDEF
#undef ENGINE
#undef NAMESPACE
#endif
