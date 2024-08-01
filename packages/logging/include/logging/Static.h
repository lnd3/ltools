#pragma once

#include "Macro.h"

namespace l {

struct Static_
{
	template<class T> 
	Static_(T only_once) { only_once(); }

	~Static_() {}  // to counter "warning: unused variable"
};

/* Usage:
STATIC {
	.. statement
};
*/
#define STATIC static l::Static_ UNIQUE(block) = [&]() -> void

/* Usage:
STATIC_CALL([]() {
	.. statement
	return 0;
});
*/
#define STATIC_CALL(f) static int UNIQUE(block) = f()



}