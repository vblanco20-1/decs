#pragma once
#include <vector>
#include <typeinfo>
#include <../../10.0.16299.0/ucrt/assert.h>

struct TestC1 {
	float x;
	float y;
	float z;
};
struct TestC2 {
	std::vector<TestC1> cmps;
};

namespace decs2 {
	using byte = unsigned char;
	static_assert ( sizeof(byte) == 1 , "size of 1 byte isnt 1 byte");
	static const size_t BLOCK_MEMORY_16K = 16384;
	static const size_t BLOCK_MEMORY_8K = 8192;

	struct ChunkComponentList;

	struct Metatype {

		using ConstructorFn = void(void*);
		using DestructorFn = void(void*);

		size_t name_hash{ 0 };
		const char* name{ "none" };
		ConstructorFn* constructor;
		DestructorFn* destructor;
		uint16_t size{ 0 };
		uint16_t align{ 0 };
	};

	struct EntityID {
		uint32_t index;
		uint32_t generation;
	};

	struct DataChunkHeader {
		//pointer to the signature for this block
		ChunkComponentList * componentList;
		struct DataChunk* prev;
		struct DataChunk* next;
		//max index that has an entity
		int16_t last{ 0 };
	};
	struct alignas(32)DataChunk {			
			byte storage[BLOCK_MEMORY_16K - sizeof(DataChunkHeader)];	
			DataChunkHeader header;
	};
	static_assert(sizeof(DataChunk) == BLOCK_MEMORY_16K, "chunk size isnt 16kb");

	struct ChunkComponentList {
		struct CmpPair {
			Metatype* type;
			int32_t chunkOffset;
		};
		int16_t chunkCapacity;
		std::vector<CmpPair> components;
	};

	template<typename T>
	struct ComponentArray {

		ComponentArray() = default;
		ComponentArray(void* pointer, DataChunk* owner) {
			data = (T*)pointer;
			chunkOwner = owner;
		}

		const T& operator[](size_t index) const{
			return data[index];
		}
		T& operator[](size_t index){
			return data[index];
		}
		bool valid()const {
			return data != nullptr;
		}
		T* begin() {
			return data;
		}
		T* end() {
			return data + chunkOwner->header.last; 
		}
		int16_t size() {
			return chunkOwner->header.last;
		}
		T* data{ nullptr };
		DataChunk* chunkOwner{nullptr};
	};


	template<typename T>
	static const  Metatype build_metatype()  {

		Metatype meta;
		meta.name_hash = typeid(T).hash_code();
		meta.name = typeid(T).name();
		if (std::is_empty<T>::value)
		{
			meta.size = 0;
		}
		else {
			meta.size = sizeof(T);
		}

		meta.align = alignof(T);

		meta.constructor = [](void* p) {
			new(p) T{};
		};
		meta.destructor = [](void* p) {
			((T*)p)->~T();
		};

		return meta;
	}

	inline ChunkComponentList* build_component_list(Metatype** types, size_t count) {
		ChunkComponentList* list = new ChunkComponentList();

		int compsize = sizeof(EntityID);
		for (size_t i = 0; i < count;i++) {
			Metatype *type = types[i];
			compsize += type->size;
		}

		size_t availibleStorage = sizeof(DataChunk::storage);
		//2 less than the real count to account for sizes and give some slack
		int itemCount = (availibleStorage / compsize)-2;

		int offsets = sizeof(DataChunkHeader);
		offsets += sizeof(EntityID) * itemCount;

		for (size_t i = 0; i < count; i++) {
			Metatype* type = types[i];

			//align properly
			size_t remainder = offsets % type->align;
			size_t oset = type->align - remainder;
			offsets += oset;

			list->components.push_back({type,offsets});			
			
			offsets += type->size * (itemCount);		
		}

		//implement proper size handling later
		assert(offsets <= BLOCK_MEMORY_16K);

		list->chunkCapacity = itemCount;

		return list;
	}
	inline int insert_entity_in_chunk(DataChunk* chunk, EntityID EID) {
		int index = -1;

		ChunkComponentList* cmpList = chunk->header.componentList;
		
		if (chunk->header.last < cmpList->chunkCapacity) {

			index = chunk->header.last;
			chunk->header.last++;

			//initialize component
			for (auto& cmp : cmpList->components) {
				Metatype* mtype = cmp.type;

				void* ptr = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * index));

				mtype->constructor(ptr);
			}

			//insert eid
			EntityID* eidptr = ((EntityID*)chunk);
			eidptr[index] = EID;
		}

		return index;
	}

	//returns ID of the moved entity
	inline EntityID erase_entity_in_chunk(DataChunk* chunk, uint16_t index) {

		ChunkComponentList* cmpList = chunk->header.componentList;

		if (chunk->header.last > index) {

			bool bPop = chunk->header.last > 1 && index != (chunk->header.last-1);
			int popIndex = chunk->header.last - 1;

			chunk->header.last--;

			//clear and pop last
			for (auto& cmp : cmpList->components) {
				Metatype* mtype = cmp.type;

				void* ptr = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * index));				
		
				mtype->destructor(ptr);

				if (bPop) {
					void* ptrPop = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * popIndex));
					memcpy(ptr, ptrPop, mtype->size);
				}
			}

			EntityID* eidptr = ((EntityID*)chunk);
			eidptr[index] = EntityID{};

			if (popIndex) {
				eidptr[index] = eidptr[popIndex];
				return eidptr[index];
			}			
		}
		return EntityID{};
	}
	
	inline DataChunk* build_chunk(ChunkComponentList* cmpList) {

		DataChunk* chunk = new DataChunk();
		chunk->header.last = 0;
		chunk->header.componentList = cmpList;		

		return chunk;
	}

	
	template<typename T>
	inline ComponentArray<T> get_chunk_array(DataChunk* chunk) {

		static const Metatype type = build_metatype<T>();

		for (auto cmp : chunk->header.componentList->components) {
			if (cmp.type->name_hash == type.name_hash)
			{
				void* ptr = (void*)((byte*)chunk + cmp.chunkOffset);
				return ComponentArray<T>(ptr, chunk);
			}
		}

		return ComponentArray<T>();
	}

	template<>
	inline ComponentArray<EntityID> get_chunk_array(DataChunk* chunk) {
		EntityID* ptr = ((EntityID*)chunk);
		return ComponentArray<EntityID>(ptr, chunk);
	}
}