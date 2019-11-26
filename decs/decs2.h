#pragma once
#include <vector>
#include <deque>
#include <algorithm>
#include <typeinfo>
#include <../../10.0.16299.0/ucrt/assert.h>
#include <K:\Programming\decs\decs\robin_hood.h>

#include <immintrin.h>
#include <tuple>
#include <type_traits>

struct TestC1 {
	float x;
	float y;
	float z;
};
struct TestC2 {
	std::vector<TestC1> cmps;
};

#pragma warning( disable : 4267 )

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
		size_t matcher_hash{};
		const char* name{ "none" };
		ConstructorFn* constructor;
		DestructorFn* destructor;
		uint16_t size{ 0 };
		uint16_t align{ 0 };

		bool is_empty() const { return align == 0; };
	};
	struct MetatypeHash {
		size_t name_hash{ 0 };
		size_t matcher_hash{ 0 };
	};
	template<typename T>
	static const  Metatype build_metatype();

	template<typename T>
	static const  MetatypeHash build_metatype_hash();

	//has stable pointers, use name_hash for it
	static robin_hood::unordered_node_map<uint64_t, Metatype> metatype_cache;

	struct EntityID {
		uint32_t index;
		uint32_t generation;
	};

	struct DataChunkHeader {
		//pointer to the signature for this block
		struct ChunkComponentList* componentList;
		struct Archetype* ownerArchetype{ nullptr };
		struct DataChunk* prev{ nullptr };
		struct DataChunk* next{ nullptr };
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

		const T& operator[](size_t index) const {
			return data[index];
		}
		T& operator[](size_t index) {
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
		DataChunk* chunkOwner{ nullptr };
	};



	struct Archetype {
		ChunkComponentList* componentList;
		struct ECSWorld* ownerWorld;
		size_t componentHash;
		int full_chunks;
		//full chunks allways on the start of the array
		std::vector<DataChunk*> chunks;
	};

	struct EntityStorage {
		DataChunk* chunk;
		uint32_t generation;
		uint16_t chunkIndex;
	};

	struct Query {
		std::vector<MetatypeHash> require_comps;
		std::vector<MetatypeHash> exclude_comps;
		std::vector<MetatypeHash> optional_comps;

		size_t require_matcher{ 0 };
		size_t exclude_matcher{ 0 };
		size_t optional_matcher{ 0 };

		bool built{ false };

		template<typename... C>
		Query& With() {
			require_comps.insert(require_comps.end(), { build_metatype_hash<C>()... });

			return *this;
		}
		template<typename... C>
		Query& Exclude() {
			exclude_comps.insert(exclude_comps.end(), { build_metatype_hash<C>()... });

			return *this;
		}

		Query& Build() {
			auto compare_hash = [](const MetatypeHash& A, const MetatypeHash& B) {
				return A.name_hash < B.matcher_hash;
			};

			auto build_matcher = [](const std::vector<MetatypeHash>& types) {
				size_t and_hash = 0;

				for (auto type : types)
				{
					and_hash |= type.matcher_hash;
				}
				return and_hash;
			};

			auto remove_eid = [](const MetatypeHash& type) {
				return (type.name_hash == build_metatype_hash<EntityID>().name_hash);
			};
			require_comps.erase(std::remove_if(require_comps.begin(), require_comps.end(), remove_eid), require_comps.end());
			exclude_comps.erase(std::remove_if(exclude_comps.begin(), exclude_comps.end(), remove_eid), exclude_comps.end());
			optional_comps.erase(std::remove_if(optional_comps.begin(), optional_comps.end(), remove_eid), optional_comps.end());

			std::sort(require_comps.begin(), require_comps.end(), compare_hash);
			std::sort(exclude_comps.begin(), exclude_comps.end(), compare_hash);
			std::sort(optional_comps.begin(), optional_comps.end(), compare_hash);

			require_matcher = build_matcher(require_comps);
			exclude_matcher = build_matcher(exclude_comps);
			optional_matcher = build_matcher(optional_comps);
			built = true;
			return *this;
		}
	};

	struct ECSWorld {
		std::vector<EntityStorage> entities;
		std::deque<uint32_t> deletedEntities;

		robin_hood::unordered_flat_map<uint64_t, std::vector<Archetype*>> archetype_signature_map{};
		robin_hood::unordered_flat_map<uint64_t, Archetype*> archetype_map{};
		std::vector<Archetype*> archetypes;
		//unique archetype hashes
		std::vector<size_t> archetypeHashes;
		//bytemask hash for checking
		std::vector<size_t> archetypeSignatures;

		int live_entities{ 0 };
		int dead_entities{ 0 };
		ECSWorld() = default;
		~ECSWorld()
		{
			for (Archetype* arch : archetypes)
			{
				for (DataChunk* chunk : arch->chunks) {
					delete chunk;
				}
				delete arch;
			}
		};

		template<typename Func>
		void for_each(Query& query, Func&& function);

		template<typename Func>
		void for_each(Func&& function);

		template<typename ... Comps>
		EntityID create_entity();
	};

	





	template<typename T>
	static const  Metatype build_metatype() {
		static const Metatype type = []() {
			Metatype meta;
			meta.name_hash = typeid(T).hash_code();
			meta.name = typeid(T).name();
			meta.matcher_hash |= (uint64_t)0x1L << (uint64_t)((meta.name_hash) % 63L);
			if (std::is_empty<T>::value)
			{
				meta.align = 0;
				meta.size = 0;
			}
			else {
				meta.align = alignof(T);
				meta.size = sizeof(T);
			}



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
	static const  MetatypeHash build_metatype_hash() {
		MetatypeHash hash;

		hash.name_hash = typeid(T).hash_code();
		hash.matcher_hash |= (uint64_t)0x1L << (uint64_t)((hash.name_hash) % 63L);
		return hash;
	}

	template<typename T>
	static const Metatype* get_metatype() {
		static const Metatype* mt = []() {
			auto name_hash = typeid(T).hash_code();

			auto type = metatype_cache.find(name_hash);
			if (type == metatype_cache.end()) {
				Metatype type = build_metatype<T>();
				metatype_cache[name_hash] = type;
			}
			return &metatype_cache[name_hash];
		}();
		return mt;
	}


	//reorder archetype with the fullness
	inline void set_chunk_full(DataChunk* chunk) {

		Archetype* arch = chunk->header.ownerArchetype;
		if (arch) {
			arch->full_chunks++;

			//do it properly later
			int archsize = arch->componentList->chunkCapacity;

			std::partition(arch->chunks.begin(), arch->chunks.end(), [archsize](DataChunk* cnk) {
				return cnk->header.last == archsize;
				});
		}


	}
	//reorder archetype with the fullness
	inline void set_chunk_partial(DataChunk* chunk) {
		Archetype* arch = chunk->header.ownerArchetype;
		arch->full_chunks--;

		//do it properly later

		int archsize = arch->componentList->chunkCapacity;

		std::partition(arch->chunks.begin(), arch->chunks.end(), [archsize](DataChunk* cnk) {
			return cnk->header.last == archsize;
			});
	}

	inline ChunkComponentList* build_component_list(const Metatype** types, size_t count) {
		ChunkComponentList* list = new ChunkComponentList();

		int compsize = sizeof(EntityID);
		for (size_t i = 0; i < count; i++) {

			compsize += types[i]->size;
		}

		size_t availibleStorage = sizeof(DataChunk::storage);
		//2 less than the real count to account for sizes and give some slack
		size_t itemCount = (availibleStorage / compsize) - 2;

		uint32_t offsets = sizeof(DataChunkHeader);
		offsets += sizeof(EntityID) * itemCount;

		for (size_t i = 0; i < count; i++) {
			const Metatype* type = types[i];

			if (type->align != 0) {
				//align properly
				size_t remainder = offsets % type->align;
				size_t oset = type->align - remainder;
				offsets += oset;
			}

			list->components.push_back({ type,offsets });

			if (type->align != 0) {
				offsets += type->size * (itemCount);
			}

		}

		//implement proper size handling later
		assert(offsets <= BLOCK_MEMORY_16K);

		list->chunkCapacity = itemCount;

		return list;
	}

	inline int insert_entity_in_chunk(DataChunk* chunk, EntityID EID, bool bInitializeConstructors = true) {
		int index = -1;

		ChunkComponentList* cmpList = chunk->header.componentList;

		if (chunk->header.last < cmpList->chunkCapacity) {

			index = chunk->header.last;
			chunk->header.last++;

			if (bInitializeConstructors) {
				//initialize component
				for (auto& cmp : cmpList->components) {
					const Metatype* mtype = cmp.type;

					if (!mtype->is_empty()) {
						void* ptr = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * index));

						mtype->constructor(ptr);
					}
				}
			}


			//insert eid
			EntityID* eidptr = ((EntityID*)chunk);
			eidptr[index] = EID;

			//if full, reorder it on archetype
			if (chunk->header.last == cmpList->chunkCapacity) {
				set_chunk_full(chunk);
			}
		}

		return index;
	}

	//returns ID of the moved entity
	inline EntityID erase_entity_in_chunk(DataChunk* chunk, uint16_t index) {

		ChunkComponentList* cmpList = chunk->header.componentList;

		bool bWasFull = chunk->header.last == cmpList->chunkCapacity;
		assert(chunk->header.last > index);
		if (chunk->header.last > index) {

			bool bPop = chunk->header.last > 1 && index != (chunk->header.last - 1);
			int popIndex = chunk->header.last - 1;

			chunk->header.last--;

			//clear and pop last
			for (auto& cmp : cmpList->components) {
				const Metatype* mtype = cmp.type;

				if (!mtype->is_empty()) {
					void* ptr = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * index));

					mtype->destructor(ptr);

					if (bPop) {
						void* ptrPop = (void*)((byte*)chunk + cmp.chunkOffset + (mtype->size * popIndex));
						memcpy(ptr, ptrPop, mtype->size);
					}
				}
			}

			EntityID* eidptr = ((EntityID*)chunk);
			eidptr[index] = EntityID{};

			if (bWasFull) {
				set_chunk_partial(chunk);
			}

			if (bPop) {

				chunk->header.ownerArchetype->ownerWorld->entities[eidptr[popIndex].index].chunkIndex = index;
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
	inline auto get_chunk_array(DataChunk* chunk) {

		using ActualT = ::std::remove_reference_t<T>;

		if constexpr (std::is_same<ActualT, EntityID>::value)
		{
			EntityID* ptr = ((EntityID*)chunk);
			return ComponentArray<EntityID>(ptr, chunk);
		}
		else {
			const Metatype* type = get_metatype<ActualT>();

			for (auto cmp : chunk->header.componentList->components) {
				if (cmp.type->name_hash == type->name_hash)
				{
					void* ptr = (void*)((byte*)chunk + cmp.chunkOffset);
					return ComponentArray<ActualT>(ptr, chunk);
				}
			}

			return ComponentArray<ActualT>();
		}
	}

	inline size_t build_signature(const Metatype** types, size_t count) {
		size_t and_hash = 0;
		//for (auto m : types)
		for (int i = 0; i < count; i++)
		{
			//consider if the fancy hash is needed, there is a big slowdown
			//size_t keyhash = ash_64_fnv1a(&types[i]->name_hash, sizeof(size_t));
			//size_t keyhash = types[i]->name_hash;

			and_hash |= types[i]->matcher_hash;
			//and_hash |=(uint64_t)0x1L << (uint64_t)((types[i]->name_hash) % 63L);
		}
		return and_hash;
	}

	inline DataChunk* create_chunk_for_archetype(Archetype* arch) {
		DataChunk* chunk = build_chunk(arch->componentList);

		chunk->header.ownerArchetype = arch;
		arch->chunks.push_back(chunk);
		return chunk;
	}
	inline bool compare_metatypes(const Metatype* A, const Metatype* B) {
		//return A->name_hash < B->name_hash;
		return A < B;
	}
	inline size_t join_metatypes(const Metatype* ATypes[], size_t Acount, const Metatype* BTypes[], size_t Bcount, const Metatype* output[]) {

		const Metatype** AEnd = ATypes + Acount;
		const Metatype** BEnd = BTypes + Bcount;

		const Metatype** A = ATypes;
		const Metatype** B = BTypes;
		const Metatype** C = output;

		while (true)
		{
			if (A == AEnd) {
				std::copy(B, BEnd, C);
			}
			if (B == BEnd) {
				std::copy(A, AEnd, C);
			}

			if (*A < *B) { *C = *A; ++A; }
			else if (*B < *A) { *C = *B; ++B; }
			else { *C = *A; ++A; ++B; }
			++C;
		}

		return C - output;
	}

	inline void sort_metatypes(const Metatype** types, size_t count) {
		std::sort(types, types + count, [](const Metatype* A, const Metatype* B) {
			return compare_metatypes(A, B);
			});
	}
	inline bool is_sorted(const Metatype** types, size_t count) {
		for (int i = 0; i < count - 1; i++) {
			if (types[i] > types[i + 1]) {
				return false;
			}
		}
		return true;
	}

	inline Archetype* find_or_create_archetype(ECSWorld* world, const Metatype** types, size_t count) {
		const Metatype* temporalMetatypeArray[32];
		assert(count < 32);

		const Metatype** typelist;

		if (!is_sorted(types, count)) {
			for (int i = 0; i < count; i++) {
				temporalMetatypeArray[i] = types[i];

			}
			sort_metatypes(temporalMetatypeArray, count);
			typelist = temporalMetatypeArray;
		}
		else {
			typelist = types;
		}

		const uint64_t matcher = build_signature(typelist, count);

		//try in the hashmap
		auto iter = world->archetype_signature_map.find(matcher);
		if (iter != world->archetype_signature_map.end()) {

			auto& archvec = iter->second;//world->archetype_signature_map[matcher];
			for (int i = 0; i < archvec.size(); i++) {

				auto componentList = archvec[i]->componentList;
				int ccount = componentList->components.size();
				if (ccount == count) {
					for (int j = 0; j < ccount; j++) {

						if (componentList->components[j].type != typelist[j])
						{
							//mismatch, inmediately continue
							goto contA;
						}
					}

					//everything matched. Found. Return inmediately
					return archvec[i];
				}

			contA:;
			}
		}

		//not found, create a new one

		Archetype* newArch = new Archetype();

		newArch->full_chunks = 0;
		newArch->componentList = build_component_list(typelist, count);
		//newArch->componentHash;
		newArch->ownerWorld = world;
		world->archetypes.push_back(newArch);
		world->archetypeSignatures.push_back(matcher);
		world->archetype_signature_map[matcher].push_back(newArch);
		//robin_hood::unordered_map<uint64_t, std::vector<Archetype*>> archetype_signature_map;
		//robin_hood::unordered_map<uint64_t, Archetype*> archetype_map;


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
		if (world->entities.size() > id.index&& id.index >= 0) {

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

	inline void destroy_entity(ECSWorld* world, EntityID id) {
		assert(is_entity_valid(world, id));
		erase_entity_in_chunk(world->entities[id.index].chunk, world->entities[id.index].chunkIndex);
		deallocate_entity(world, id);
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


	inline void move_entity_to_archetype(Archetype* newarch, EntityID id, bool bInitializeConstructors = true) {

		//insert into new chunk
		DataChunk* oldChunk = newarch->ownerWorld->entities[id.index].chunk;
		DataChunk* newChunk = find_free_chunk(newarch);

		int newindex = insert_entity_in_chunk(newChunk, id, bInitializeConstructors);
		int oldindex = newarch->ownerWorld->entities[id.index].chunkIndex;

		int oldNcomps = oldChunk->header.componentList->components.size();
		int newNcomps = newChunk->header.componentList->components.size();

		auto& oldClist = oldChunk->header.componentList;
		auto& newClist = newChunk->header.componentList;

		//copy all data from old chunk into new chunk
		//bad iteration, fix later

		struct Merge {
			int msize;
			int idxOld;
			int idxNew;
		};
		int mergcount = 0;
		Merge mergarray[32];

		for (int i = 0; i < oldNcomps; i++) {
			const Metatype* mtCp1 = oldClist->components[i].type;
			if (!mtCp1->is_empty()) {
				for (int j = 0; j < newNcomps; j++) {
					const Metatype* mtCp2 = newClist->components[j].type;

					//pointers are stable
					if (mtCp2 == mtCp1) {
						mergarray[mergcount].idxNew = j;
						mergarray[mergcount].idxOld = i;
						mergarray[mergcount].msize = mtCp1->size;
						mergcount++;
					}
				}
			}
		}

		for (int i = 0; i < mergcount; i++) {
			//const Metatype* mtCp1 = mergarray[i].mtype;

			//pointer for old location in old chunk
			void* ptrOld = (void*)((byte*)oldChunk + oldClist->components[mergarray[i].idxOld].chunkOffset + (mergarray[i].msize * oldindex));

			//pointer for new location in new chunk
			void* ptrNew = (void*)((byte*)newChunk + newClist->components[mergarray[i].idxNew].chunkOffset + (mergarray[i].msize * newindex));

			//memcopy component data from old to new
			memcpy(ptrNew, ptrOld, mergarray[i].msize);
		}

		//delete entity from old chunk
		erase_entity_in_chunk(oldChunk, oldindex);

		//assign entity chunk data
		newarch->ownerWorld->entities[id.index].chunk = newChunk;
		newarch->ownerWorld->entities[id.index].chunkIndex = newindex;
	}
	inline void set_entity_archetype(Archetype* arch, EntityID id) {

		//if chunk is null, we are a empty entity
		if (arch->ownerWorld->entities[id.index].chunk == nullptr) {

			DataChunk* targetChunk = find_free_chunk(arch);

			int index = insert_entity_in_chunk(targetChunk, id);
			arch->ownerWorld->entities[id.index].chunk = targetChunk;
			arch->ownerWorld->entities[id.index].chunkIndex = index;
		}
		else {
			move_entity_to_archetype(arch, id, false);
		}
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

	//template<typename F>
	//void iterate_matching_archetypes(ECSWorld* world, const Metatype** types,int count, F&& function);
	//
	//template<typename F>
	//void iterate_matching_chunks(ECSWorld* world, const Metatype** types, int count, F&& function);

	template<typename C>
	bool has_component(ECSWorld* world, EntityID id);

	template<typename C>
	C& get_component(ECSWorld* world, EntityID id);

	template<typename C>
	void add_component_to_entity(ECSWorld* world, EntityID id, C&& comp);

	template<typename C>
	void remove_component_from_entity(ECSWorld* world, EntityID id);

	//template<typename CA, typename F>
	//void iterate_entities(ECSWorld* world, F&& function);
	//template<typename CA, typename CB, typename F>
	//void iterate_entities(ECSWorld* world, F&& function);

	template<typename F>
	void iterate_matching_archetypes(ECSWorld* world, const Query& query, F&& function) {

		for (int i = 0; i < world->archetypeSignatures.size(); i++)
		{
			//if there is a good match, doing an and not be 0
			uint64_t includeTest = world->archetypeSignatures[i] & query.require_matcher;

			//implement later
			uint64_t excludeTest = world->archetypeSignatures[i] & query.exclude_matcher;
			if (includeTest != 0) {

				auto componentList = world->archetypes[i]->componentList;

				//might match an excluded component, check here
				if (excludeTest != 0) {

					bool invalid = false;
					//dumb algo, optimize later					
					for (int mtA = 0; mtA < query.exclude_comps.size(); mtA++) {

						for (auto cmp : componentList->components) {

							if (cmp.type->name_hash == query.exclude_comps[mtA].name_hash) {
								//any check and we out
								invalid = true;
								break;
							}
						}
						if (invalid) {
							break;
						}
					}
					if (invalid) {
						continue;
					}
				}

				//dumb algo, optimize later
				int matches = 0;
				for (int mtA = 0; mtA < query.require_comps.size(); mtA++) {

					for (auto cmp : componentList->components) {

						if (cmp.type->name_hash == query.require_comps[mtA].name_hash) {
							matches++;
							break;
						}
					}
				}
				//all perfect
				if (matches == query.require_comps.size()) {

					function(world->archetypes[i]);
				}
			}


		}

	}

	//template<typename F>
	//void iterate_matching_archetypes(ECSWorld* world, const Metatype** types, int count, F&& function)
	//{
	//	const Metatype* temporalMetatypeArray[32];
	//	assert(count < 32);
	//
	//	const Metatype** typelist;
	//
	//	if (!is_sorted(types, count)) {
	//		for (int i = 0; i < count; i++) {
	//			temporalMetatypeArray[i] = types[i];
	//
	//		}
	//		sort_metatypes(temporalMetatypeArray, count);
	//		typelist = temporalMetatypeArray;
	//	}
	//	else {
	//		typelist = types;
	//	}
	//
	//	const uint64_t matcher = build_signature(typelist, count);
	//
	//	for (int i = 0; i < world->archetypeSignatures.size(); i++) {
	//
	//		//if there is a good match, doing an and not be 0
	//		uint64_t andTest = world->archetypeSignatures[i] & matcher;
	//
	//		if (andTest != 0) {
	//
	//			auto componentList = world->archetypes[i]->componentList;
	//
	//			//dumb algo, optimize later
	//			int matches = 0;
	//			for (int mtA = 0; mtA < count; mtA++) {
	//
	//				for (auto cmp : componentList->components) {
	//					//if (cmp.type->name_hash == typelist[mtA]->name_hash) {
	//					if (cmp.type == typelist[mtA]) {
	//						matches++;
	//						break;
	//					}
	//				}
	//			}
	//			//all perfect
	//			if (matches == count) {
	//
	//				function(world->archetypes[i]);
	//			}
	//		}
	//	}
	//}


	//template<typename F>
	//void iterate_matching_chunks(ECSWorld* world, const Metatype** types, int count, F&& function)
	//{
	//	iterate_matching_archetypes(world, types, count, [&](Archetype* arch) {
	//
	//		for (auto chunk : arch->chunks) {
	//			function(chunk);
	//		}
	//	});
	//}

	template<typename C>
	C& get_component(ECSWorld* world, EntityID id)
	{

		EntityStorage& storage = world->entities[id.index];

		auto acrray = get_chunk_array<C>(storage.chunk);
		assert(acrray.chunkOwner != nullptr);
		return acrray[storage.chunkIndex];
	}


	template<typename C>
	bool has_component(ECSWorld* world, EntityID id)
	{
		EntityStorage& storage = world->entities[id.index];

		auto acrray = get_chunk_array<C>(storage.chunk);
		return acrray.chunkOwner != nullptr;
	}

	template<typename C>
	void add_component_to_entity(ECSWorld* world, EntityID id, C&& comp)
	{
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
		if (!type->is_empty()) {
			get_component<C>(world, id) = comp;
		}

	}

	template<typename C>
	void remove_component_from_entity(ECSWorld* world, EntityID id)
	{
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


	template<typename... Type>
	struct type_list {};

	template<typename Class, typename Ret, typename... Args>
	type_list<Args...> args(Ret(Class::*)(Args...) const);

	//template<typename A, typename B, typename C, typename D, typename Func>
	//void entity_chunk_iterate(DataChunk* chnk, Func&& function) {
	//
	//	auto array0 = get_chunk_array<A>(chnk);
	//	auto array1 = get_chunk_array<B>(chnk);
	//	auto array2 = get_chunk_array<C>(chnk);
	//	auto array3 = get_chunk_array<D>(chnk);
	//
	//	assert(array0.chunkOwner == chnk);
	//	assert(array1.chunkOwner == chnk);
	//	assert(array2.chunkOwner == chnk);
	//	assert(array3.chunkOwner == chnk);
	//	for (int i = chnk->header.last - 1; i >= 0; i--) {
	//		function(array0[i], array1[i], array2[i], array3[i]);
	//	}
	//}

	//by skypjack
	template<typename... Args, typename Func>
	void entity_chunk_iterate(DataChunk* chnk, Func&& function) {
		auto tup = std::make_tuple(get_chunk_array<Args>(chnk)...);
		(assert(std::get<decltype(get_chunk_array<Args>(chnk))>(tup).chunkOwner == chnk), ...);

		for (int i = chnk->header.last - 1; i >= 0; i--) {
			function(std::get<decltype(get_chunk_array<Args>(chnk))>(tup)[i]...);
		}
	}

	template<typename ...Args, typename Func>
	void unpack_chunk(type_list<Args...> types, DataChunk* chunk, Func&& function) {
		entity_chunk_iterate<Args...>(chunk, function);
	}
	template<typename ...Args>
	Query& unpack_querywith(type_list<Args...> types, Query& query) {
		return query.With<Args...>();
	}

	template<typename Func>
	void decs2::ECSWorld::for_each(Query& query, Func&& function)
	{
		using params = decltype(args(&Func::operator()));

		iterate_matching_archetypes(this, query, [&](Archetype* arch) {

			for (auto chnk : arch->chunks) {

				unpack_chunk(params{}, chnk, function);
			}
			});
	}
	template<typename Func>
	void decs2::ECSWorld::for_each(Func&& function)
	{
		using params = decltype(args(&Func::operator()));

		Query query;
		unpack_querywith(params{}, query).Build();

		for_each<Func>(query, std::move(function));
	}


	template<typename ...Comps>
	EntityID decs2::ECSWorld::create_entity()
	{
		const Metatype* types[] = { get_metatype<Comps>()... };
		size_t num = (sizeof(types) / sizeof(*types));

		Archetype* arch = find_or_create_archetype(this, types, num);
		return create_entity_with_archetype(arch);
	}
}

