#pragma once
#include <vector>
#include <deque>
#include <algorithm>
#include <typeinfo>
#include <../../10.0.16299.0/ucrt/assert.h>
#include <K:\Programming\decs\decs\robin_hood.h>

struct TestC1 {
	float x;
	float y;
	float z;
};
struct TestC2 {
	std::vector<TestC1> cmps;
};



inline const uint64_t hash_64_fnv1a(const void* key, const uint64_t len) {

	const char* data = (char*)key;
	uint64_t hash = 0xcbf29ce484222325;
	uint64_t prime = 0x100000001b3;

	for (int i = 0; i < len; ++i) {
		uint8_t value = data[i];
		hash = hash ^ value;
		hash *= prime;
	}

	return hash;
}

namespace decs2 {
	using byte = unsigned char;
	static_assert (sizeof(byte) == 1, "size of 1 byte isnt 1 byte");
	static const size_t BLOCK_MEMORY_16K = 16384;
	static const size_t BLOCK_MEMORY_8K = 8192;

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

	//has stable pointers, use name_hash for it
	static robin_hood::unordered_node_map<uint64_t,Metatype> metatype_cache;

	struct EntityID {
		uint32_t index;
		uint32_t generation;
	};

	struct DataChunkHeader {
		//pointer to the signature for this block
		struct ChunkComponentList * componentList;
		struct Archetype* ownerArchetype;
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
			const Metatype* type;
			uint32_t chunkOffset;
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
		static const Metatype type = []() {
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
		}();
		return type;
	}

	template<typename T>
	static const Metatype* get_metatype() {
		auto name_hash = typeid(T).hash_code();
		
		auto type = metatype_cache.find(name_hash);
		if (type == metatype_cache.end()) {
			Metatype type = build_metatype<T>();
			metatype_cache[name_hash] = type;
		}
		return &metatype_cache[name_hash];
	}


	struct Archetype {
		ChunkComponentList* componentList;
		struct ECSWorld* ownerWorld;
		size_t componentHash;

		//full chunks allways on the start of the array
		std::vector<DataChunk*> chunks;
	};

	struct EntityStorage {
		DataChunk* chunk;
		uint32_t generation;
		uint16_t chunkIndex;
	};

	struct ECSWorld {
		std::vector<EntityStorage> entities;
		std::deque<uint32_t> deletedEntities;

		std::vector<Archetype*> archetypes;
		//unique archetype hashes
		std::vector<size_t> archetypeHashes;
		//bytemask hash for checking
		std::vector<size_t> archetypeSignatures;

		int live_entities{0};
		int dead_entities{0};
	};

	inline ChunkComponentList* build_component_list(const Metatype** types, size_t count) {
		ChunkComponentList* list = new ChunkComponentList();

		int compsize = sizeof(EntityID);
		for (size_t i = 0; i < count;i++) {
			
			compsize += types[i]->size;
		}

		size_t availibleStorage = sizeof(DataChunk::storage);
		//2 less than the real count to account for sizes and give some slack
		size_t itemCount = (availibleStorage / compsize)-2;

		uint32_t offsets = sizeof(DataChunkHeader);
		offsets += sizeof(EntityID) * itemCount;

		for (size_t i = 0; i < count; i++) {
			const Metatype* type = types[i];

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
				const Metatype* mtype = cmp.type;

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
				const Metatype* mtype = cmp.type;

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

		const Metatype* type = get_metatype<T>();

		for (auto cmp : chunk->header.componentList->components) {
			if (cmp.type->name_hash == type->name_hash)
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

	inline size_t build_signature(const Metatype** types, size_t count) {
		size_t and_hash = 0;
		//for (auto m : types)
		for(int i = 0; i < count;i++)
		{			
			size_t keyhash = hash_64_fnv1a(&types[i]->name_hash, sizeof(size_t));

			and_hash |= (uint64_t)0x1L << (uint64_t)((keyhash) % 63L);
		}
		return and_hash;
	}
	inline DataChunk* create_chunk_for_archetype(Archetype* arch) {
		DataChunk* chunk = build_chunk(arch->componentList);

		chunk->header.ownerArchetype = arch;
		arch->chunks.push_back(chunk);
		return chunk;
	}

	inline void sort_metatypes(const Metatype** types, size_t count) {
		std::sort(types, types + count, [](const Metatype* A, const Metatype* B) {
			return A->name_hash < B->name_hash;
		});
	}

	inline Archetype* find_or_create_archetype(ECSWorld* world,const Metatype** types, size_t count) {
		static const Metatype* temporalMetatypeArray[32];
		assert(count < 32);

		for (int i = 0; i < count; i++) {
			temporalMetatypeArray[i] = types[i];
		}

		uint64_t matcher = build_signature(temporalMetatypeArray, count);

		sort_metatypes(temporalMetatypeArray,count);

		
		for (int i = 0; i < world->archetypeSignatures.size(); i++) {

			//if there is a perfect match, doing a xor will have it be 0
			uint64_t xorTest = world->archetypeSignatures[i] ^ matcher;

			if (xorTest == 0) {
				
				auto componentList = world->archetypes[i]->componentList;
				if (componentList->components.size() == count) {
					for (int j = 0; j < componentList->components.size(); j++) {

						if (componentList->components[j].type->name_hash != temporalMetatypeArray[j]->name_hash)
						{
							//mismatch, inmediately continue
							goto cont;
						}
					}

					//everything matched. Found. Return inmediately
					return world->archetypes[i];
				}				
			}

		cont:;
		}

		//not found, create a new one
		
		Archetype* newArch = new Archetype();
		
		newArch->componentList = build_component_list(temporalMetatypeArray, count);
		//newArch->componentHash;
		newArch->ownerWorld = world;
		world->archetypes.push_back(newArch);
		world->archetypeSignatures.push_back(matcher);

		//we want archs to allways have 1 chunk at least, create initial
		create_chunk_for_archetype(newArch);
		return newArch;
	}

	

	inline EntityID allocate_entity(ECSWorld* world) {
		EntityID newID;
		if (world->dead_entities == 0) {
			int index = world->entities.size();

			EntityStorage newStorage;
			newStorage.chunk = nullptr;
			newStorage.chunkIndex = 0;
			newStorage.generation = 1;

			world->entities.push_back(newStorage);

			newID.generation = 1;
			newID.index = index;
		}
		else {
		int index = world->deletedEntities.front();
		world->deletedEntities.pop_front();

		world->entities[index].generation++;

		newID.generation = world->entities[index].generation;
		newID.index = index;
		world->dead_entities--;
		}

		world->live_entities++;
		return newID;
	}
	
	inline bool is_entity_valid(ECSWorld* world, EntityID id) {
		//index check
		if (world->entities.size() > id.index && id.index >= 0) {

			//generation check
			if (id.generation != 0 && world->entities[id.index].generation == id.generation)
			{
				return true;
			}
		}
		return false;
	}
	inline void deallocate_entity(ECSWorld* world, EntityID id) {
		//todo add valid check
		world->deletedEntities.push_back(id.index);
		world->entities[id.index].generation++;
		world->entities[id.index].chunk = nullptr;
		world->entities[id.index].chunkIndex = 0;

		world->live_entities--;
		world->dead_entities++;
	}
	inline DataChunk* find_free_chunk(Archetype* arch) {
		DataChunk* targetChunk = nullptr;
		if (arch->chunks.size() == 0) {
			targetChunk = create_chunk_for_archetype(arch);
		}
		else {
			targetChunk = arch->chunks[arch->chunks.size() - 1];
			//chunk is full, create a new one
			if (targetChunk->header.last == arch->componentList->chunkCapacity) {
				targetChunk = create_chunk_for_archetype(arch);
			}
		}
		return targetChunk;
	}

	inline void move_entity_to_archetype(Archetype* newarch, EntityID id);
	inline void set_entity_archetype(Archetype* arch, EntityID id) {

		//if chunk is null, we are a empty entity
		if (arch->ownerWorld->entities[id.index].chunk == nullptr) {

			DataChunk* targetChunk = find_free_chunk(arch);
			
			int index = insert_entity_in_chunk(targetChunk, id);
			arch->ownerWorld->entities[id.index].chunk = targetChunk;
			arch->ownerWorld->entities[id.index].chunkIndex = index;
		}
		else {
			move_entity_to_archetype(arch, id);
		}		
	}
	inline void move_entity_to_archetype(Archetype* newarch, EntityID id) {

		//insert into new chunk
		DataChunk* oldChunk = newarch->ownerWorld->entities[id.index].chunk;
		DataChunk* newChunk = find_free_chunk(newarch);

		int newindex = insert_entity_in_chunk(newChunk, id);
		int oldindex = newarch->ownerWorld->entities[id.index].chunkIndex;

		int oldNcomps = oldChunk->header.componentList->components.size();
		int newNcomps = newChunk->header.componentList->components.size();

		auto& oldClist = oldChunk->header.componentList;
		auto& newClist = newChunk->header.componentList;

		//copy all data from old chunk into new chunk
		//bad iteration, fix later
		for (int i = 0; i < oldNcomps; i++) {
			const Metatype* mtCp1 = oldClist->components[i].type;
			for (int j  = 0; j < newNcomps; j++) {
				const Metatype* mtCp2 = newClist->components[j].type;
				if (mtCp2->name_hash == mtCp1->name_hash) {

					//pointer for old location in old chunk
					void* ptrOld = (void*)((byte*)oldChunk + oldClist->components[i].chunkOffset + (mtCp1->size * oldindex));

					//pointer for new location in new chunk
					void* ptrNew = (void*)((byte*)newChunk + newClist->components[j].chunkOffset + (mtCp2->size * newindex));
					
					//memcopy component data from old to new
					memcpy(ptrNew, ptrOld, mtCp1->size);
				}
			}
		}

		//delete entity from old chunk
		erase_entity_in_chunk(oldChunk, oldindex);

		//assign entity chunk data
		newarch->ownerWorld->entities[id.index].chunk = newChunk;
		newarch->ownerWorld->entities[id.index].chunkIndex = newindex;
	}
	inline EntityID create_entity_with_archetype(Archetype* arch) {
		ECSWorld* world = arch->ownerWorld;

		EntityID newID = allocate_entity(world);

		set_entity_archetype(arch, newID);

		return newID;
	}
	inline Archetype* get_entity_archetype(ECSWorld* world, EntityID id)
	{
		assert(is_entity_valid(world, id));

		return world->entities[id.index].chunk->header.ownerArchetype;
	}

	template<typename F>
	void iterate_matching_archetypes(ECSWorld* world, const Metatype** types,int count, F&& function) {
		const Metatype* temporalMetatypeArray[32];
		assert(count < 32);

		for (int i = 0; i < count; i++) {
			temporalMetatypeArray[i] = types[i];
		}

		uint64_t matcher = build_signature(temporalMetatypeArray, count);

		sort_metatypes(temporalMetatypeArray, count);

		for (int i = 0; i < world->archetypeSignatures.size(); i++) {

			//if there is a good match, doing an and not be 0
			uint64_t andTest = world->archetypeSignatures[i] & matcher;

			if (andTest != 0) {

				auto componentList = world->archetypes[i]->componentList;
				
				//dumb algo, optimize later
				int matches = 0;
				for (int mtA = 0; mtA < count; mtA++) {
					
					for (auto cmp : componentList->components) {
						if (cmp.type->name_hash == temporalMetatypeArray[mtA]->name_hash) {
							matches++;
							break;
						}
					}
				}
				//all perfect
				if (matches == count) {

					function(world->archetypes[i]);
				}			
			}
		}
	}
	template<typename F>
	void iterate_matching_chunks(ECSWorld* world, const Metatype** types, int count, F&& function) {

		iterate_matching_archetypes(world, types, count, [&](Archetype* arch) {
			
			for (auto chunk : arch->chunks) {
				function(chunk);
			}
		});
	}

	template<typename C>
	C& get_component(ECSWorld* world, EntityID id) {
				
		EntityStorage &storage = world->entities[id.index];
		
		auto acrray = get_chunk_array<C>(storage.chunk);
		return acrray[storage.chunkIndex];
	}
	template<typename C>
	bool has_component(ECSWorld* world, EntityID id) {
		EntityStorage& storage = world->entities[id.index];

		auto acrray = get_chunk_array<C>(storage.chunk);
		return acrray.chunkOwner != nullptr;
	}

	template<typename C>
	void add_component_to_entity(ECSWorld* world, EntityID id, C&& comp) {
		const Metatype* temporalMetatypeArray[32];

		const Metatype* type = get_metatype<C>();

		Archetype* oldarch = get_entity_archetype(world, id);
		ChunkComponentList* oldlist = oldarch->componentList;
		bool typeFound = false;
		int lenght = oldlist->components.size();
		for (int i = 0; i < oldlist->components.size(); i++) {
			temporalMetatypeArray[i] = oldlist->components[i].type;

			//the pointers for metatypes are allways fully stable
			if (temporalMetatypeArray[i] == type) {
				typeFound = true;
			}
		}

		Archetype* newArch = oldarch;
		if (!typeFound) {
			temporalMetatypeArray[lenght] = type;
			sort_metatypes(temporalMetatypeArray, lenght + 1);
			lenght++;

			newArch = find_or_create_archetype(world, temporalMetatypeArray, lenght);

			set_entity_archetype(newArch, id);
		}
		//optimize later
		get_component<C>(world, id) = comp;		
	}


	template<typename C>
	void remove_component_from_entity(ECSWorld* world, EntityID id) {
		const Metatype* temporalMetatypeArray[32];

		const Metatype* type = get_metatype<C>();

		Archetype* oldarch = get_entity_archetype(world, id);
		ChunkComponentList* oldlist = oldarch->componentList;
		bool typeFound = false;
		int lenght = oldlist->components.size();
		for (int i = 0; i < lenght; i++) {
			temporalMetatypeArray[i] = oldlist->components[i].type;

			//the pointers for metatypes are allways fully stable
			if (temporalMetatypeArray[i] == type) {
				typeFound = true;
				//swap last
				temporalMetatypeArray[i] = oldlist->components[lenght - 1].type;
			}
		}

		Archetype* newArch = oldarch;
		if (typeFound) {

			
			lenght--;
			sort_metatypes(temporalMetatypeArray, lenght);			

			newArch = find_or_create_archetype(world, temporalMetatypeArray, lenght);

			set_entity_archetype(newArch, id);
		}
	}


	template<typename CA, typename F>
	void iterate_entities(ECSWorld* world, F&& function) {

		Metatype *type = get_metatype<CA>();
		
		iterate_matching_chunks(world, type, 1, [&](DataChunk* chnk) {
			auto acrray = get_chunk_array<CA>(chnk);
			
			auto earray = get_chunk_array<EntityID>(chnk);
			for (int i = 0; i < chnk->header.last; i++) {
				function(earray[i], acrray[i]);
			}
		});
	}
	template<typename CA,typename CB, typename F>
	void iterate_entities(ECSWorld* world, F&& function) {

		const Metatype* types[] = {get_metatype<CA>(),get_metatype<CB>() };
		iterate_matching_chunks(world, types, 1, [&](DataChunk* chnk) {
			auto acrray = get_chunk_array<CA>(chnk);
			auto bcrray = get_chunk_array<CB>(chnk);

			auto earray = get_chunk_array<EntityID>(chnk);
			for (int i = 0; i < chnk->header.last; i++) {
				function(earray[i], acrray[i], bcrray[i]);
			}
		});
	}
}

