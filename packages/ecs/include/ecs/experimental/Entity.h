#pragma once

#include "meta/Reflection.h"
#include "logging/Log.h"
#include "logging/Debug.h"

#ifndef ECS_SLOT_SIZE_BIT_COUNT
#define ECS_SLOT_SIZE_BIT_COUNT 8
#endif

namespace l {
namespace ecs {

	/*
	The block vector contain blocks and is stored in a std::vector
	Blocks are abstract memory areas
	Blocks denote the memory occupied by an _Element_
	_Elements_ contain a header and a semi-dynamic data area
	_Elements_ ability to grow are constrained by upper boundary occupied blocks
	_Elements_ can shrink but only grow to the boundary of the next occupied element
	Slots are an abstract section of memory occupied by an actual object within the block vector
	Slots may span several _Elements_ in which case the element header holds the size of the slot	
	*/
	namespace details {
		class SlotInfo {
		public:
			static const size_t MaxLength = (1 << ECS_SLOT_SIZE_BIT_COUNT) - 1;

			uint16_t occupied : 8, length : ECS_SLOT_SIZE_BIT_COUNT;
		};
	
		template<const uint16_t DataSize>
		class Element {
		public:

			Element() {
				ASSERT((DataSize & (~SlotInfo::MaxLength)) == 0) << "Data size is out of bounds, size is " << DataSize;
				auto size = sizeof(SlotInfo);
				auto bytes = DataSize - static_cast<uint16_t>(size);
				info().length = bytes;
				fill(VALUE_INITIALIZED);
			}
			~Element() {
				fill(VALUE_UNDEFINED);
			}

			__forceinline uint8_t* data() {
				return mData + sizeof(SlotInfo);
			}

			__forceinline SlotInfo& info() {
				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				return *info;
			}

			template<class T>
			T* get(size_t index = 0) {
				ASSERT((index + 1) * sizeof(T) <= size()) << "Slot is out of bounds (" << std::to_string(index) << ") for type " << meta::class_name<T>();
				if (empty()) {
					return nullptr;
				}

				void* p = data() + index * sizeof(T);
				return reinterpret_cast<T*>(p);
			}

			bool set_size(size_t s) {
				if (s > size()) {
					LOG(LogError) << "Slots cannot grow, only shrink. This slot maximum size is (" << size() << ")";
					return false;
				}

				if (s == 0 || s > SlotInfo::MaxLength) {
					LOG(LogError) << "Failed to set size (" << s << "). It cannot be larger than " << SlotInfo::MaxLength;
					return false;
				}

				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				info->length = s;
				info->occupied = 1;
				return true;
			}

			bool scary_set_size(size_t size) {
				ASSERT(size > 0 && size <= SlotInfo::MaxLength) << "Failed to set size (" << size << "). It cannot be larger than " << SlotInfo::MaxLength;
				if (size == 0 || size > SlotInfo::MaxLength) {
					LOG(LogError) << "Failed to set size (" << size << "). It cannot be larger than " << SlotInfo::MaxLength;
					return false;
				}

				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				info->length = size;
				info->occupied = 1;
				return true;
			}

			__forceinline size_t size_plus_header() {
				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				return info->length + sizeof(SlotInfo);
			}

			__forceinline size_t size() {
				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				return info->length;
			}

			__forceinline bool empty() {
				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				return info->occupied == 0;
			}

			__forceinline void set_empty() {
				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				info->occupied = 0;
			}

			__forceinline void set_not_empty() {
				SlotInfo* info = reinterpret_cast<SlotInfo*>(mData);
				info->occupied = 1;
			}

			void erase() {
				memset(data(), VALUE_UNDEFINED, size());
				set_empty();
			}

			void fill(uint8_t value) {
				memset(data(), value, size());
				set_empty();
			}

		private:
			uint8_t mData[DataSize] = { VALUE_INITIALIZED };
		};
	}

	using namespace details;

	template<const uint16_t DataSize, const uint16_t Size = DataSize + sizeof(SlotInfo)>
	using Data = Element<Size>;

	template<const size_t BlockSize_>
	class Entities {
	public:

		static const size_t BlockSize = sizeof(Element<BlockSize_>);

		template<class T>
		static size_t blocks_required_for_storage() {
			size_t slotInfoSize = sizeof(SlotInfo);
			size_t templateSize = sizeof(T);
			return 1 + (slotInfoSize + templateSize - 1) / BlockSize; // +1 for the extra mSize byte, -1 for not crossing BlockSize boundary until size is equal to Blocksize or more.
		}

		static size_t blocks_required_for(size_t bytes) {
			size_t slotInfoSize = sizeof(SlotInfo);
			return 1 + (slotInfoSize + bytes - 1) / BlockSize;
		}

		Entities(size_t reserve) {
			mBlocks.reserve(reserve);
		}
		virtual ~Entities() = default;

		struct Iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using value_type = Element<BlockSize_>;
			using pointer = Element<BlockSize_>*;
			using reference = Element<BlockSize_>&;

			Iterator(pointer ptr) : m_ptr(ptr) {}

			reference operator*() const { return *m_ptr; }
			pointer operator->() { return m_ptr; }

			pointer get_ptr() { return m_ptr; }

			template<class T>
			T* get(size_t index = 0) const {
				return m_ptr->get<T>(index);
			}

			template<class T>
			size_t size() const {
				return m_ptr->size() / sizeof(T);
			}

			size_t block_count() const {
				return blocks_required_for(m_ptr->size());
			}

			Iterator& operator++() {
				const size_t blockCount = blocks_required_for(m_ptr->size());
				m_ptr += blockCount;
				return *this; 
			}

			Iterator operator++(int) {
				const size_t blockCount = blocks_required_for(m_ptr->size());
				Iterator tmp = *this;
				m_ptr += blockCount;
				return tmp;
			}

			friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
			friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };

		private:
			pointer m_ptr;
		};

		Iterator begin() {
			return Iterator(mBlocks.data());
		}

		Iterator at(size_t index) {
			auto end_ = end();
			if (index >= mBlocks.size()) {
				return end_;
			}
			size_t i = 0;
			for (auto it = begin(); it != end_; ++it) {
				size_t currentIndex = get_id(it);
				if (index < currentIndex) {
					return end_;
				}
				if (index == currentIndex) {
					return it;
				}
				i++;
			}
			return end_;
		}

		Iterator end() {
			return Iterator(mBlocks.data() + mBlocks.size());
		}

		size_t block_size() const {
			return BlockSize;
		}

		size_t byte_count() const {
			return mBlocks.size() * BlockSize;
		}

		size_t get_id(intptr_t ptr) {
			intptr_t d = ptr - reinterpret_cast<intptr_t>(mBlocks.data());
			ASSERT(d % BlockSize == 0) << "Invalid entity ptr: Block base address is off by " << std::to_string(d % BlockSize) << " bytes.";
			size_t id = d / BlockSize;
			ASSERT(id < mBlocks.size()) << "Invalid entity id: " << std::to_string(id);
			return id;
		}

		size_t get_id(Element<BlockSize_>* ptr) {
			size_t id = ptr - mBlocks.data();
			ASSERT(id < mBlocks.size()) << "Invalid entity id: " << std::to_string(id);
			return id;
		}

		size_t get_id(Iterator it) {
			size_t id = it.get_ptr() - mBlocks.data();
			ASSERT(id < mBlocks.size()) << "Invalid entity id: " << std::to_string(id);
			return id;
		}

		template<class DataType>
		size_t push_back(DataType&& customData) {
			Element<BlockSize_>* ptr = mBlocks.data() + mBlocks.size();
			const size_t blockCount = blocks_required_for_storage<DataType>();
			mBlocks.resize(mBlocks.size() + blockCount);

			// Save new data into block(s)
			memcpy(ptr->data(), (void*)&customData, sizeof(DataType));

			// Save size of new data
			if (!ptr->scary_set_size(sizeof(DataType))) {
				return MAXSIZE_T;
			}

			return mBlocks.size() - blockCount;
		}

		template<class DataType>
		bool scary_set(Iterator it, DataType&& customData) {

			Element<BlockSize_>* ptr = it.get_ptr();

			if (!ptr->set_size(sizeof(DataType))) {
				return false;
			}

			if (sizeof(DataType) > ptr->size()) {
				//ASSERT(sizeof(DataType) <= ptr->size()) << "Slot is to small for the type '" << memory::class_name<DataType>() << "' (" << std::to_string(sizeof(DataType)) << " bytes). Slot has " << std::to_string(ptr->size()) << " bytes.");
				return false;
			}

			const size_t blockCount = blocks_required_for_storage<DataType>();


			// Save new data into block(s)
			memcpy(ptr->data(), (void*)&customData, sizeof(DataType));

			// Save size of new data
			return true;
		}

		size_t slot_size(Iterator it) {
			Element<BlockSize_>* ptr = it.get_ptr();
			return ptr->size_plus_header();
		}

		bool erase(Iterator it) {
			Element<BlockSize_>* ptr = it.get_ptr();

			if (ptr->size() == 0) {
				LOG(LogWarning) << "Failed to erase slot because it is already empty and has a size of 0..";
				return true;
			}

			size_t id = ptr - mBlocks.data();

			if (id >= mBlocks.size()) {
				LOG(LogError) << "id out of range : " + id;
				return false;
			}

			ptr->erase();

			return true;
		}

		size_t count_continouos_free_blocks_at(size_t id) {
			auto it = at(id);
			Element<BlockSize_>* ptr = it.get_ptr();
			if (ptr->empty()) {
				return 0;
			}
			return 0;
		}

	protected:
		std::vector<Element<BlockSize_>> mBlocks;
	};

}
}
