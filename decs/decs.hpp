#pragma once

#include "stdint.h"
#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <chrono>
#include "plf_colony.h"
#include <cassert>
#include <deque>
#include <algorithm>
#include <execution>
#include <xmmintrin.h>
using ComponentGUID = uint64_t;

struct Metatype {

	using ConstructorFn = void(void*);
	using DestructorFn = void(void*);

	size_t name_hash{0};
	const char * name{"none"};
	ConstructorFn *constructor;
	DestructorFn *destructor;
	uint16_t size{ 0 };
	uint16_t align{ 0 };

	template<typename T>
	constexpr static Metatype BuildMetatype() {

		Metatype meta;

		meta.name_hash = typeid(T).hash_code(); 
		meta.name = typeid(T).name();
		if ( std::is_empty<T>::value )
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
};



struct BaseComponent {
public:

	using IDCounter = size_t;
protected:

	static IDCounter family_counter_;
};


struct ComponentList {
	std::vector<Metatype> metatypes;
	
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
	void sort() {
		std::sort(metatypes.begin(),metatypes.end(),[](auto left, auto right){
		return left.name_hash < right.name_hash;
		});
	}

	void build_hash() {
		cached_hash = 0;
		and_hash = 0;
		for (auto m : metatypes)
		{			
			cached_hash += m.name_hash;
			size_t keyhash = hash_64_fnv1a(&m.name_hash,sizeof(size_t));

			and_hash |=(uint64_t) 0x1L << (uint64_t)((keyhash )% 63L);
		}

	}

	template<typename ...C>
	static ComponentList build_component_list() {
		ComponentList list;
		list.metatypes = {Metatype::BuildMetatype<C>()...} ;
		
		list.finish();
		return list;
	}
	
	void finish() {
		sort();
		build_hash();		
	}
	std::size_t and_hash{ 0 };
	std::size_t cached_hash{ 0 };
};

namespace ecs {

	
}


struct Archetype {

	Archetype& operator= (const Archetype &rhs) {
		componentlist.metatypes.clear();
		for (auto c : rhs.componentlist.metatypes)
		{
			componentlist.metatypes.push_back(c);
		}
		componentlist.finish();
		
		return *this;
	}

	//clear constructor
	Archetype() {

	}
	template<typename C>
	void set_add(const Archetype &rhs)
	{
		static const Metatype m(Metatype::BuildMetatype<C>());

		//Archetype arc;
		componentlist.metatypes.clear();
		componentlist.metatypes.reserve(rhs.componentlist.metatypes.size() + 1);
		bool added = false;

		for (auto c : rhs.componentlist.metatypes)
		{
			componentlist.metatypes.push_back(c);
			if (c.name_hash == m.name_hash) {
				added = true;
			}
		}
		if (!added)
		{
			componentlist.metatypes.push_back(Metatype::BuildMetatype<C>());
		}
		componentlist.finish();
	}
	template<typename C>
	void set_remove(const Archetype &rhs)
	{
		static const Metatype m(Metatype::BuildMetatype<C>());

		componentlist.metatypes.clear();
		componentlist.metatypes.reserve(rhs.componentlist.metatypes.size());
		
		for (auto c : rhs.componentlist.metatypes)
		{
			if (c.name_hash != m.name_hash) {
				componentlist.metatypes.push_back(c);
			}
		}
		componentlist.finish();
	}
	//registers a new component to the archetype metatypes
	template<typename C>
	void add_component()
	{
		Metatype mt = Metatype::BuildMetatype<C>();

		for (auto m : componentlist.metatypes)
		{
			if (m.name_hash == mt.name_hash)
			{
				//already there
				return;
			}
		}

		componentlist.metatypes.push_back(mt);

		componentlist.finish();
	}
	template<typename C>
	void remove_component()
	{
		Metatype mt = Metatype::BuildMetatype<C>();

		ComponentList newList;
		newList.metatypes.reserve(componentlist.metatypes.size());
		for (auto m : componentlist.metatypes)
		{
			if (m.name_hash != mt.name_hash)
			{
				//already there
				newList.metatypes.push_back(m);
			}
		}

		componentlist = newList;
		componentlist.finish();
	}

	template<typename C>
	bool has() {
		Metatype mt = Metatype::BuildMetatype<C>();

		for (auto m : componentlist.metatypes)
		{
			if (m.name_hash == mt.name_hash)
			{
				return true;
			}
		}
		return false;
	}
	bool match_and_hash(const ComponentList & Components) {
		const size_t ListHash = Components.and_hash;
		const size_t ThisHash = componentlist.and_hash;
		const size_t andhash = ListHash & ThisHash;
		return andhash  == ListHash;
	}
	int match(const ComponentList & Components)
	{
		int matches = 0;
		const auto & A_Met = Components.metatypes;
		const auto & B_Met = componentlist.metatypes;
		int iA = 0;
		int iB = 0;
		while (iA < A_Met.size() && iB < B_Met.size()) {
			const size_t hashA = A_Met[iA].name_hash;
			const size_t hashB = B_Met[iB].name_hash;
			if (hashA == hashB)
			{
				matches++;
				iA++;
				iB++;
			}
			else {
				if (hashA < hashB) {
					iA++;
				}
				else {
					iB++;
				}
			}
		}
		
		return matches;
	}

	bool exact_match(const ComponentList & Components) {
		if (Components.cached_hash != componentlist.cached_hash) {
			return false;
		}
		if (Components.metatypes.size() != componentlist.metatypes.size())
		{
			return false;
		}
		//else if (Components.metatypes.size() == 0)
		//{
		//	return true;
		//}
		for (int i = 0; i < Components.metatypes.size(); i++)
		{
			if (Components.metatypes[i].name_hash != componentlist.metatypes[i].name_hash)
			{
				return false;
			}
		}
		//for (auto c : Components.metatypes)
		//{
		//	bool bFound = false;
		//	for (auto a : componentlist.metatypes)
		//	{
		//		if (a.name_hash == c.name_hash)
		//		{
		//			bFound = true;
		//		}
		//	}
		//	if (!bFound)
		//	{
		//		return false;
		//	}
		//
		//}
		return true;
	}


	ComponentList componentlist;

	static constexpr size_t ARRAY_SIZE = 4096;
};



struct ArchetypeComponentArray {
	void * data;	
	Metatype metatype;
	ArchetypeComponentArray() {
		data = nullptr;
	}
	ArchetypeComponentArray(Metatype type, void* alloc) {
		data = alloc;
		metatype = type;
	}	

	// Simple move constructor
	ArchetypeComponentArray(ArchetypeComponentArray&& arg) // the expression "arg.member" is lvalue
	{
		data = arg.data;
		arg.data = nullptr;
		metatype = std::move(arg.metatype);
	}
	// Simple move assignment operator
	ArchetypeComponentArray& operator=(ArchetypeComponentArray&& other) {
		data = other.data;
		other.data = nullptr;
		metatype = std::move(other.metatype);
		return *this;
	}

	void copy(uint64_t src, uint64_t dst)
	{
		char *psrc = (char*)data + src * metatype.size;
		char *pdst = (char*)data + dst * metatype.size;

		memcpy(pdst, psrc, metatype.size);
	}
	void create(uint64_t idx) {
		void* ptr = (char*)data + idx * metatype.size;
		metatype.constructor(ptr);
	}
	void destroy(uint64_t idx) {
		void* ptr = (char*)data + idx * metatype.size;
		metatype.destructor(ptr);
	}
	//copy from a different array
	void copy_from_outer(uint64_t src, uint64_t dst, ArchetypeComponentArray * other)
	{
		char *psrc = (char*)other->data + src * metatype.size;
		char *pdst = (char*)data + dst * metatype.size;

		memcpy(pdst, psrc, metatype.size);
	}	
};
template<typename T>
struct MutableComponentArray {
	
	MutableComponentArray() = default;
	MutableComponentArray(const ArchetypeComponentArray &tarray) {
		data = (T*)tarray.data;
	}

	T&Get(size_t index) {
		return data[index];
	}
	T& operator[](size_t index) {
		return data[index];
	}
	bool valid()const{
		return data != nullptr;
	}
private:
	T * data{ nullptr };

};
template<typename T>
struct ComponentArray {

	ComponentArray() = default;
	ComponentArray(const ArchetypeComponentArray &tarray) {
		data = (T*)tarray.data;
	}
	
	const T& operator[](size_t index) {
		return data[index];
	}
	bool valid()const {
		return data != nullptr;
	}
private:
	T * data{ nullptr };
};
struct EntityHandle {
	size_t id;
	size_t generation;
	EntityHandle() {
		id = -1;
		generation = -1;
	}
	bool operator ==(const EntityHandle &b) const {
		return id == b.id && generation == b.generation;
	}
};
struct ArchetypeBlockStorage;
struct ArchetypeBlock;
struct ECSWorld;

//8 kilobytes
static const size_t BLOCK_MEMORY = 16384;//8192;
struct ArchetypeBlock {
	using byte = unsigned char ;

	size_t canary;
	Archetype myArch;
	uint16_t fullsize;
	void* memory;
	
	ArchetypeComponentArray* _componentArrays;
	uint16_t _array_count;

	//handle array
	EntityHandle * entities;
	
	//max index that has an entity
	int16_t last{ 0 };
	//new entities that shouldnt be iterated yet
	int16_t newets{ 0 };
	uint64_t lastIteration{ 0 };	

	ArchetypeBlockStorage * storage;
	ArchetypeBlock(const Archetype &arch) {
		myArch = arch;
		myArch.componentlist.finish();		
	}
	void initialize_block()
	{
		//size in bytes per entity
		uint16_t entity_size = sizeof(EntityHandle);
		uint16_t header_size = 0;
		int count = 0;
		for (auto &m : myArch.componentlist.metatypes)
		{
			
			if (m.size != 0)
			{
				count++;
				entity_size += m.size;
			}
		}

		_array_count = count;
		header_size += sizeof(ArchetypeComponentArray) * _array_count;


		memory = malloc(BLOCK_MEMORY);
		fullsize = ((BLOCK_MEMORY -header_size) / entity_size) - 2;
		byte* alloc = (byte*)memory;
		_componentArrays = new(alloc) ArchetypeComponentArray[_array_count];
		alloc += header_size;
		entities = new(alloc) EntityHandle[fullsize];
		//advance stack allocate
		alloc += sizeof(EntityHandle) * (fullsize + 1);

		int i = 0;
		for (auto &m : myArch.componentlist.metatypes)
		{			
			if (m.size != 0)
			{		
				_componentArrays[i] = ArchetypeComponentArray(m, alloc);
				i++;
				//componentArrays.push_back(std::move(ArchetypeComponentArray(m, alloc)));
				alloc += m.size * (fullsize + 1);
			}
		}


		canary = 0xDEADBEEF;
		newets = 0;		
		last = 0;
	}
	

	~ArchetypeBlock()
	{
		last = 0;
		canary = 0;
		if (memory != nullptr)
		{
			free(memory);
		}
	}

	// Simple move constructor
	ArchetypeBlock(ArchetypeBlock&& arg) 
	{
		canary = arg.canary;
		myArch = arg.myArch;
		fullsize = arg.fullsize;
		memory = arg.memory;
		arg.memory = nullptr;
		//componentArrays = std::move(arg.componentArrays);

		//handle array
		entities = arg.entities;

		//max index that has an entity
		last = arg.last;
		//new entities that shouldnt be iterated yet
		newets= arg.newets;
		lastIteration= arg.lastIteration;
	}
	// Simple copy constructor (move it)
	ArchetypeBlock(const ArchetypeBlock& arg)
	{
		canary = arg.canary;
		myArch = arg.myArch;
		//fullsize = arg.fullsize;
		//memory = arg.memory;
		////arg.memory = nullptr;
		//componentArrays = (arg.componentArrays);
		//
		////handle array
		//entities = arg.entities;
		//
		////max index that has an entity
		//last = arg.last;
		////new entities that shouldnt be iterated yet
		//newets = arg.newets;
		//lastIteration = arg.lastIteration;
	}
	// Simple move assignment operator
	ArchetypeBlock& operator=(ArchetypeBlock&& arg) {
		canary = arg.canary;
		myArch = arg.myArch;
		fullsize = arg.fullsize;
		memory = arg.memory;
		arg.memory = nullptr;
		//componentArrays = std::move(arg.componentArrays);

		//handle array
		entities = arg.entities;

		//max index that has an entity
		last = arg.last;
		//new entities that shouldnt be iterated yet
		newets = arg.newets;
		lastIteration = arg.lastIteration;
		return *this;
	}
	bool checkSanity() {
		if (canary != 0xDEADBEEF) return false;
		for (int i = 0; i < fullsize; i++)
		{
			if (entities[i].generation != 1 && i < last)
			{
				//std::cout << "what the fuck";
				return false;
			}
		}
		return true;
	}
	template<typename F>
	void foreach_array(F&&functor) {
		for (int i = 0; i < _array_count; i++) {
			functor(_componentArrays[i]);
		}
	}

	template<typename C>
	MutableComponentArray<C> get_component_array_mutable() {
		Metatype mc = Metatype::BuildMetatype<C>();
		
		for (int i = 0; i < _array_count; i++) {
			if (_componentArrays[i].metatype.name_hash == mc.name_hash) {
				MutableComponentArray<C> tarray = MutableComponentArray<C>(_componentArrays[i]);
				return tarray;
			}
		}		

		return MutableComponentArray<C>();
	}

	uint16_t add_entity(EntityHandle handle)
	{
		uint16_t pos = last + newets;
		assert(pos < fullsize);

		entities[pos] = handle;		

		//last++;
		newets++;
		//entities[last].generation = 7;
		return pos;
	}
	
	void initialize_entity(uint64_t entity) {
		foreach_array([=](auto &clist){
			clist.create(entity);
		});
		
	}
	void destroy_entity(uint64_t entity) {
		foreach_array([=](auto &clist) {
			clist.destroy(entity);
		});		
	}

	void destroy_all() {
		for (uint64_t i = 0; i < last + newets; i++)
		{
			foreach_array([=](auto &clist) {
				clist.create(i);
			});			
		}
		
	}
	//copy a entity from a different block into this one
	void copy_entity_from_block(uint64_t destEntity, uint64_t srcEntity, ArchetypeBlock * otherblock)
	{
		//copy components to the index
		for (int j = 0; j < otherblock->_array_count; j++) 
		//for (auto &csrc : otherblock->componentArrays)
		
		{
			auto &csrc =  otherblock->_componentArrays[j];

			for (int i = 0; i < _array_count; i++) 
			//for (auto &cthis : componentArrays)
			{
				auto &cthis = _componentArrays[i];
				//find a component metatype match, and copy it to this block
				if (csrc.metatype.name_hash == cthis.metatype.name_hash)
				{
					cthis.copy_from_outer(srcEntity, destEntity, &csrc);
					//break first for
					break;
				}
			}
		}

		//copy entity handle	
		entities[destEntity] = otherblock->entities[srcEntity];
		//if (destEntity >= last)
		//{
		//	assert(false);
		//	last++;
		//}
	}

	EntityHandle GetLastEntity() {
		return entities[last - 1];
	}
	int GetSpace() {
		return fullsize - last - newets;
	}
	void refresh(uint64_t iterationIDX) {
		//assert(canary ==)
		assert(canary == 0xDEADBEEF);
		lastIteration = iterationIDX;
		last += newets;
		newets = 0;
	}
	
};


struct ArchetypeBlockStorage {


	Archetype myArch;
	int nblocks;
	ArchetypeBlockStorage(const Archetype & arch) {
		myArch = arch;
		
		nblocks = 0;
		
		myArch.componentlist.finish();

	}

	~ArchetypeBlockStorage() {
		
	}
	ArchetypeBlock * create_new_block() {
		nblocks++;
				
		auto b = block_colony.insert( myArch );

		ArchetypeBlock * blk = &(*b);
		blk->initialize_block();
		
		blk->storage = this;

		assert(nblocks == block_colony.size());
		return blk;
	}


	void delete_block(ArchetypeBlock * blk) {

		freeblock = nullptr;


		nblocks--;

		bool bDeleted = false;
		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{			
			if (&(*it) == blk)
			{
				it->destroy_all();
				it->last = 0;
				block_colony.erase(it);
				bDeleted = true;
				break;
			}
		}
		assert(bDeleted);
		assert(nblocks == block_colony.size());
	}
	template<typename F>
	void iterate(F&& f) {

		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{
			it->refresh(0);
			if (it->last != 0)
			{
				f((*it));
			}
		}
	}
	template<typename F>
	void iterate_par(F&&f) {
		if (block_colony.size() > 4) {
			iterate(f);
		}
		else {
			std::for_each(std::execution::par,block_colony.begin(),block_colony.end(),[&](auto& it){
				it.refresh(0);
				if (it.last != 0)
				{
					f((it));
				}
			});
		}

	}
	void add_blocks(std::vector<ArchetypeBlock*>& blocks) {

		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{
			assert(it->canary == 0xDEADBEEF);
			it->refresh(0);
			if (it->last != 0)
			{

				blocks.push_back(&(*it));
			}
		}
	}

	ArchetypeBlock * find_free_block() {
		//cached freeblock
		if (freeblock != nullptr && freeblock->GetSpace() > 0)
		{
			return freeblock;
		}

		ArchetypeBlock * ptr = nullptr;

		//iterate linked list
		for (auto & b : block_colony)
		{
			ptr = &b;

			if (ptr->GetSpace() > 0)
			{
				freeblock = ptr;
				return ptr;
			}
		}

		freeblock = nullptr;

		return nullptr;
	}

	ArchetypeBlock * freeblock{ nullptr };	

	plf::colony<ArchetypeBlock> block_colony;

	static void delete_block(ArchetypeBlockStorage * store, ArchetypeBlock * blk)
	{
		store->delete_block(blk);
	}
};





struct EntityStorage {
	size_t generation{ 0 };
	//size_t block;
	ArchetypeBlock * block{ nullptr };
	uint16_t blockindex{ 0 };
};

struct Matcher {
	std::vector<size_t> archetype_indices;
	ComponentList required_components;
	ComponentList forbidden_components;
};

struct ECSWorld {

	bool MatchAndHash(size_t ListHash, size_t MatchHash) {

		const size_t andhash = ListHash & MatchHash;
		return andhash == MatchHash;
	}
	std::vector<ArchetypeBlock*> IterationBlocks;
	
	uint64_t iterationIdx{ 0 };
	template<typename F>
	void IterateBlocks(const ComponentList &AllOfList, const ComponentList& NoneOfList, F && f, bool bParallel = false) {
		iterationIdx++;

		bIsIterating = true;

		Matcher& mt = get_matcher(AllOfList,NoneOfList);
		if (!bParallel) {
			std::for_each(mt.archetype_indices.begin(), mt.archetype_indices.end(), [&](size_t arch) {
				BlockStorage[arch].iterate(f);
			});
		}
		else
		{
			std::for_each(std::execution::par, mt.archetype_indices.begin(), mt.archetype_indices.end(), [&](size_t arch) {
				BlockStorage[arch].iterate_par(f);
			});
		}

		bIsIterating = false;
	}
	template<typename F>
	void IterateBlocks(const ComponentList &AllOfList, F && f, bool bParallel = false) {
		iterationIdx++;
		
		bIsIterating = true;

		Matcher& mt = get_matcher(AllOfList);
		if (!bParallel){
			std::for_each(mt.archetype_indices.begin(), mt.archetype_indices.end(), [&](size_t arch) {
				BlockStorage[arch].iterate(f);
			});
		}
		else
		{	std::for_each(std::execution::par, mt.archetype_indices.begin(), mt.archetype_indices.end(), [&](size_t arch) {				
			    BlockStorage[arch].iterate_par(f);
			});
		}

		bIsIterating = false;
	}
	bool ValidateAll() {
		bool valid = true;
		for (ArchetypeBlockStorage & b : BlockStorage)
		{
			b.iterate([&](auto & blk) {
				bool b = blk.checkSanity();
				if (b == false)
				{
					valid = false;
				}
			});
		}
		return valid;

	}

	ArchetypeBlock * FindOrCreateBlockForArchetype(const Archetype & arc)
	{
		ArchetypeBlock * entityBlock = nullptr;
		//find the free block	
		const size_t numComponents = arc.componentlist.metatypes.size();
		
		const size_t blocksize = BlockStorage.size();
		const size_t cached_hash = arc.componentlist.cached_hash;
		//for (ArchetypeBlockStorage & b : BlockStorage)
		for (size_t i = 0; i < blocksize; i++)
		{			
			//block needs to have the same amount of components, and match all of them
			if(BlockHashes[i] != cached_hash)
			{
				continue;
			}
			else
			{
				ArchetypeBlockStorage * b = &BlockStorage[i];
			//if (b->myArch.ExactMatch(arc.componentlist))
			
				entityBlock = b->find_free_block();
				if (entityBlock == nullptr)
				{
					entityBlock = b->create_new_block();
					break;

				}
			}
		}
		//block not found, create a new one
		if (entityBlock == nullptr)
		{
			
			//auto str = ArchetypeBlockStorage(arc);
			BlockStorage.push_back(ArchetypeBlockStorage(arc));
			BlockHashes.push_back(arc.componentlist.cached_hash);
			BlockANDHashes.push_back(arc.componentlist.and_hash);
			entityBlock = BlockStorage[BlockStorage.size() - 1].create_new_block();
			add_archetype_to_matchers(BlockStorage.size() - 1);
			//entityBlock = CreateBlock(arc);
		}

		return entityBlock;
	}

	std::vector<EntityHandle> CreateEntityBatched(Archetype & arc, size_t amount)
	{
		arc.componentlist.build_hash();
		//Entities.reserve(Entities.size() + amount);
		std::vector<EntityHandle> Handles;
		Handles.reserve(amount);
		size_t amount_left = amount;
		bool bCanReuseEntities = CanReuseEntity();
		int reuses = (int)deletedEntities.size();
		int newcreations = (int)amount - reuses;
		if (newcreations > 0)
		{
			//Entities.reserve(Entities.size() + newcreations);
		}

		while (amount_left > 0)
		{
			ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);
			int freespace = entityBlock->GetSpace();
			for (int i = 0; i < freespace; i++) {

				if (amount_left <= 0)
				{
					return Handles;
				}
				EntityHandle newEntity;
				bool bReuse = CanReuseEntity();
				size_t index = 0;
				if (bCanReuseEntities && bReuse) {

					index = deletedEntities.front();
					deletedEntities.pop_front();
					assert(Entities[index].block == nullptr);
					newEntity.id = index;
					newEntity.generation = Entities[index].generation + 1;
				}
				else
				{
					bCanReuseEntities = false;
					newEntity.id = Entities.size();
					index = newEntity.id;
					newEntity.generation = 1;
				}

				uint16_t pos = entityBlock->add_entity(newEntity);
				entityBlock->initialize_entity(pos);

				EntityStorage et;
				et.block = entityBlock;
				et.blockindex = pos;
				et.generation = newEntity.generation;

				if (bReuse) {
					Entities[index] = et;
				}
				else {
					Entities.push_back(et);
				}

				//Entities.push_back(et);
				Handles.push_back(newEntity);
				amount_left--;
			}
		}
		return Handles;
	}
	EntityHandle CreateEntity(Archetype & arc) {
		arc.componentlist.build_hash();
		EntityHandle newEntity;

		bool bReuse = CanReuseEntity();
		size_t index = 0;
		if (bReuse) {
			
			index = deletedEntities.front();
			assert(Entities[index].block == nullptr);
			deletedEntities.pop_front();
			newEntity.id = index;
			newEntity.generation = Entities[index].generation + 1;
		}
		else
		{
			newEntity.id = Entities.size();
			index = Entities.size();
			newEntity.generation = 1;
		}


		ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);

		//insert entity handle into the block
		uint16_t pos = entityBlock->add_entity(newEntity);
		entityBlock->initialize_entity(pos);

		EntityStorage et;
		et.block = entityBlock;
		et.blockindex = pos;
		et.generation = newEntity.generation;

		if (bReuse) {
			Entities[index] = et;
		}
		else {

			Entities.push_back(et);
			index = Entities.size() - 1;
		}

		
		ArchetypeBlock* block = Entities[newEntity.id].block;
		auto blockidx = Entities[newEntity.id].blockindex;
		assert(block->entities[blockidx] == newEntity);
		return newEntity;
	}

	template<typename C>
	void AddComponent(EntityHandle entity)		//, C&comp)
	{
		if (Valid(entity)) {
			const EntityStorage & et = Entities[entity.id];
			//valid entity

			static Archetype cached;

			cached.set_add<C>(et.block->myArch);

			//static Archetype newarch = et.block->myArch;
			//newarch.AddComponent<C>();
			//Archetype newarch = Archetype::CreateAdd<C>(et.block->myArch);
			SetEntityArchetype(entity, cached);// newarch);
			new(&GetComponent<C>(entity)) C{};
		}
	}
	template<typename C>
	void HasComponent(EntityHandle entity)
	{
		if (Valid(entity)) {
			const EntityStorage & et = Entities[entity.id];
			return et.block->myArch->has<C>();
		}
		else {
			return false;
		}
	}


	template<typename C>
	void RemoveComponent(EntityHandle entity)		//, C&comp)
	{
		const EntityStorage & et = Entities[entity.id];
		//valid entity
		if (Valid(entity)) {

			static Archetype cached;
			cached.set_remove<C>(et.block->myArch);

			//Archetype newarch = Archetype::CreateRemove<C>(et.block->myArch);// et.block->myArch;
			//newarch.RemoveComponent<C>();

			GetComponent<C>(entity).~C();
			SetEntityArchetype(entity, cached);
			//ValidateAll();

		}
	}

	void DeleteEntity(EntityHandle entity) {
		assert(Valid(entity));
		EntityStorage & et = Entities[entity.id];

		//delete the entity from its block
		ArchetypeBlock * oldBlock = et.block;
		oldBlock->refresh(0);
		auto oldpos = et.blockindex;
		EntityHandle Swapped;

		oldBlock->destroy_entity(oldpos);
		if (RemoveAndSwapFromBlock(oldpos, *oldBlock, Swapped))
		{
			oldBlock->storage->delete_block(oldBlock);
		}
		if (Valid(Swapped, false))
		{
			Entities[Swapped.id].blockindex = oldpos;
		}
		
		deletedEntities.push_back(entity.id);

		et.block = nullptr;
		et.blockindex = 0;
		et.generation++;
	}

	//returns true if the block got deleted
	bool RemoveAndSwapFromBlock(uint16_t idx, ArchetypeBlock & Block, EntityHandle & SwappedEntity) {
		assert(Block.entities[idx].generation != -1);
		assert(idx < Block.last + Block.newets);
		//shrink
		//if (newets != 0)
		{
			//	newets--;
		}
		bool bIsInitialized = false;
		if (idx < Block.last)
		{
			bIsInitialized = true;
		}
		uint16_t swap_entity = (Block.last + Block.newets) - 1;

		

		if (bIsInitialized)
		{
			Block.last--;
		}
		else {
			Block.newets--;
		}


		assert(Block.canary == 0xDEADBEEF);

		if (Block.last <= 0 && Block.newets <= 0)
		{
			//block emptied

			return true;
		}
		assert(idx <= swap_entity);
		if (idx == swap_entity)
		{
			//assert(false);
			Block.entities[swap_entity].generation = -1;
			Block.entities[swap_entity].id = -1;
			return false;
		}

		//copy components to the index
		Block.foreach_array( [&](auto&ca) {
			ca.copy(swap_entity, idx);
		});
		//for (auto &ca : Block.componentArrays)
		//{
		//	ca.copy(swap_entity, idx);
		//}

		//copy entity handle
		Block.entities[idx] = Block.entities[swap_entity];

		Entities[Block.entities[idx].id].blockindex = idx;

		SwappedEntity = Block.entities[swap_entity];
		Block.entities[swap_entity].generation = -1;
		Block.entities[swap_entity].id = -1;
		return false;
	}

	bool Valid(EntityHandle entity, bool perform_asserts = true)
	{
		const bool bEnableAsserts = false;
		perform_asserts = perform_asserts && bEnableAsserts;

		if (perform_asserts) assert(entity.id < Entities.size());
		if (entity.id >= Entities.size()) return false;
		const EntityStorage & et = Entities[entity.id];
		if (et.block != nullptr)
		{
			if (perform_asserts) assert(et.block->canary == 0xDEADBEEF);
			if (et.block->canary != 0xDEADBEEF) return false;
		}
		if (perform_asserts)assert(et.blockindex <= et.block->fullsize);
		if (perform_asserts)assert(et.blockindex >= 0);

		//valid entity
		return (et.generation == entity.generation  && et.block != nullptr && et.block->last + et.block->newets <= Archetype::ARRAY_SIZE);

	}

	void SetEntityArchetype(EntityHandle entity, Archetype &arc) {

		EntityStorage & et = Entities[entity.id];

		assert(Valid(entity));

		ArchetypeBlock * oldBlock = et.block;
		if (oldBlock->myArch.exact_match(arc.componentlist))
		{
			return;
		}

		assert(oldBlock->canary == 0xDEADBEEF);

		ArchetypeBlock * newBlock = FindOrCreateBlockForArchetype(arc);

		//create entity in the new block
		auto pos = newBlock->add_entity(entity);
		auto oldpos = et.blockindex;
		//copy old block entity into new block entity
		newBlock->copy_entity_from_block(pos, oldpos, oldBlock);

		//clear old entity slot
		EntityHandle SwapEntity;// = oldBlock->GetLastEntity();

		if (RemoveAndSwapFromBlock(oldpos, *oldBlock, SwapEntity))
		{
			//if (bIsIterating)
			//{
			//	bWantsDeletes = true;
			//}
			//else
			//{
			auto storage = oldBlock->storage;
			storage->delete_block(oldBlock);
			//}
			//oldBlock->storage->DeleteBlock(oldBlock);	
		}
		if (Valid(SwapEntity, false))
		{
			Entities[SwapEntity.id].blockindex = oldpos;
		}
		//update the low level data
		et.block = newBlock;
		et.blockindex = pos;

	}

	bool CanReuseEntity() {
		return !deletedEntities.empty();
	}

	template<typename Component>
	Component & GetComponent(EntityHandle entity) {

		const auto et = Entities[entity.id];
		return et.block->get_component_array_mutable<Component>().Get(et.blockindex);

	}

	Matcher& get_matcher(const ComponentList& AdditiveList,const ComponentList& SubstractiveList) {
	
		Matcher* search = nullptr;
		for (Matcher& mt : Matchers) {
			if (mt.required_components.cached_hash == AdditiveList.cached_hash && mt.forbidden_components.cached_hash == SubstractiveList.cached_hash) {
				search = &mt;
			}
		}
		if (!search) {
			Matcher newMatcher;
			newMatcher.required_components = AdditiveList;
			Matchers.push_back(newMatcher);
			search = &Matchers.back();
			for (size_t i = 0; i < BlockStorage.size(); i++)
			{
				
				if (MatchAndHash(BlockANDHashes[i], AdditiveList.and_hash))
				{					
					ArchetypeBlockStorage & b = BlockStorage[i];
					if (b.myArch.match_and_hash(AdditiveList) && b.myArch.match(AdditiveList) == AdditiveList.metatypes.size())
					{
						if (b.myArch.match(SubstractiveList) == 0)
						{
							search->archetype_indices.push_back(i);
						}
					}
				}
			}
		}

		return *search;		
	}
	Matcher& get_matcher(const ComponentList& AdditiveList) {
		return get_matcher(AdditiveList,ComponentList());
	}
	void add_archetype_to_matchers(size_t index) {
		for (auto & mt : Matchers) {
			if (MatchAndHash(BlockANDHashes[index], mt.required_components.and_hash))
			{
				ArchetypeBlockStorage & b = BlockStorage[index];
				if (b.myArch.match_and_hash(mt.required_components) && b.myArch.match(mt.required_components) == mt.required_components.metatypes.size())
				{
					if (b.myArch.match(mt.forbidden_components) == 0)
					{
						mt.archetype_indices.push_back(index);
					}
				}
			}
		}
	}

	std::vector<EntityStorage> Entities;
	std::deque<size_t> deletedEntities;

	std::vector<Matcher> Matchers;
	std::vector<ArchetypeBlockStorage> BlockStorage;
	std::vector<size_t> BlockHashes;
	std::vector<size_t> BlockANDHashes;
	bool bIsIterating{ false };
	bool bWantsDeletes{ false };

	//EnTT style  wrapper for porting----------------------------------------------
	EntityHandle create() {
		Archetype empty;
		return CreateEntity(empty);
	}
	void destroy(EntityHandle entity)
	{
		DeleteEntity(entity);
	}

	template<typename Component>
	Component & assign(EntityHandle entity, Component & args) {
		AddComponent<Component>(entity);
		Component & comp = GetComponent<Component>(entity);
		comp = args;
		return GetComponent<Component>(entity);
	}
	template<typename Component>
	Component & assign(EntityHandle entity) {
		AddComponent<Component>(entity);
		Component & comp = GetComponent<Component>(entity);
		return GetComponent<Component>(entity);
	}

	template<typename Component>
	bool has(EntityHandle entity) {
		return HasComponent<Component>(entity);
	}

	template<typename Component>
	void remove(EntityHandle entity)
	{
		RemoveComponent<Component>(entity);
	}
	template<typename Component>
	Component & get(EntityHandle entity) {
		return GetComponent<Component>(entity);
	}
	template<typename Component>
	Component & accomodate(EntityHandle entity, Component & args) {
		return assign(entity, args);
	}

};