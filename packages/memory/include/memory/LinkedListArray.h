#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>			// For lower_bound
#include <typeinfo>
#include <optional>
#include <iterator> // For std::forward_iterator_tag
#include <cstddef>  // For std::ptrdiff_t

#include "logging/Log.h"
#include "meta/Reflection.h"

namespace l::container {

	union LLData {
		struct Node {
			Node* next;
			Node* prev;
		};
		struct LinkedList {
			Node* head;
			Node* tail;
		};
		struct LinkedListNode {
			Node* next;
			Node* prev;
			int value;
		};
	};
}
