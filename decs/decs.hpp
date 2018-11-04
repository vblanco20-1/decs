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
using ComponentGUID = uint64_t;

struct Metatype {
	size_t name_hash;
	size_t size;
	size_t align;

	template<typename T>
	static Metatype BuildMetatype() {

		Metatype meta;

		meta.name_hash = (size_t)T::GUID();
		meta.size = sizeof(T);
		meta.align = alignof(T);

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

	long hash64shift(long key)
	{
		key = (~key) + (key << 21); // key = (key << 21) - key - 1;
		key = key ^ (key >> 24);
		key = (key + (key << 3)) + (key << 8); // key * 265
		key = key ^ (key >> 14);
		key = (key + (key << 2)) + (key << 4); // key * 21
		key = key ^ (key >> 28);
		key = key + (key << 31);
		return key;
	};


	void BuildHash() {
		cached_hash = 0;

		for (auto m : metatypes)
		{
			long hash = hash64shift(m.name_hash);
			cached_hash |= hash;
		}

	}
	std::size_t cached_hash{ 0 };
};

struct Archetype {

	Archetype& operator= (const Archetype &rhs) {
		componentlist.metatypes.clear();
		for (auto c : rhs.componentlist.metatypes)
		{
			componentlist.metatypes.push_back(c);
		}
		return *this;
	}

	//clear constructor
	Archetype() {

	}
	template<typename C>
	void SetAdd(const Archetype &rhs)
	{
		//Archetype arc;
		componentlist.metatypes.clear();
		componentlist.metatypes.reserve(rhs.componentlist.metatypes.size() + 1);

		for (auto c : rhs.componentlist.metatypes)
		{
			componentlist.metatypes.push_back(c);
		}
		componentlist.metatypes.push_back(Metatype::BuildMetatype<C>());
		componentlist.BuildHash();
		//return std::move(arc);
	}
	template<typename C>
	void SetRemove(const Archetype &rhs)
	{
		componentlist.metatypes.clear();
		componentlist.metatypes.reserve(rhs.componentlist.metatypes.size());
		auto m = Metatype::BuildMetatype<C>();
		for (auto c : rhs.componentlist.metatypes)
		{
			if (c.name_hash != m.name_hash) {
				componentlist.metatypes.push_back(c);
			}
		}

		componentlist.BuildHash();
		//return std::move(arc);
	}
	//registers a new component to the archetype metatypes
	template<typename C>
	void AddComponent()
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

		componentlist.BuildHash();
	}
	template<typename C>
	void RemoveComponent()
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

		componentlist = newList;//.metatypes.push_back(mt);

		componentlist.BuildHash();
	}

	int Match(const ComponentList & Components)
	{

		int matches = 0;
		for (auto c : Components.metatypes)
		{
			bool bFound = false;
			for (auto a : componentlist.metatypes)
			{
				if (a.name_hash == c.name_hash)
				{
					bFound = true;
					break;
				}
			}
			if (bFound)
			{
				matches++;
			}

		}
		return matches;
	}

	bool ExactMatch(const ComponentList & Components) {
		if (Components.cached_hash != componentlist.cached_hash) {
			return false;
		}
		if (Components.metatypes.size() != componentlist.metatypes.size())
		{
			return false;
		}
		else if (Components.metatypes.size() == 0)
		{
			return true;
		}
		for (auto c : Components.metatypes)
		{
			bool bFound = false;
			for (auto a : componentlist.metatypes)
			{
				if (a.name_hash == c.name_hash)
				{
					bFound = true;
				}
			}
			if (!bFound)
			{
				return false;
			}

		}
		return true;
	}


	ComponentList componentlist;

	static constexpr size_t ARRAY_SIZE = 4096;
};



struct ArchetypeComponentArray {
	void * data;
	Metatype metatype;

	ArchetypeComponentArray(Metatype type) {
		data = malloc(Archetype::ARRAY_SIZE * type.size);
		metatype = type;
	}

	void Copy(uint64_t src, uint64_t dst)
	{
		char *psrc = (char*)data + src * metatype.size;
		char *pdst = (char*)data + dst * metatype.size;

		memcpy(pdst, psrc, metatype.size);
	}

	//copy from a different array
	void CopyFromOuter(uint64_t src, uint64_t dst, ArchetypeComponentArray * other)
	{
		char *psrc = (char*)other->data + src * metatype.size;
		char *pdst = (char*)data + dst * metatype.size;

		memcpy(pdst, psrc, metatype.size);
	}


	template<typename T>
	T &Get(size_t index) {
		//assert(Metatype::BuildMetatype<T>().name_hash == metatype.name_hash);

		T * ptr = (T*)data;

		return ptr[index];
	}
};
template<typename T>
struct TypedArchetypeComponentArray {
	T * data{ nullptr };
	TypedArchetypeComponentArray() = default;
	TypedArchetypeComponentArray(const ArchetypeComponentArray &tarray) {
		data = (T*)tarray.data;
	}

	T&Get(size_t index) {
		return data[index];
	}
};
struct EntityHandle {
	size_t id;
	size_t generation;
	bool operator ==(const EntityHandle &b) const {
		return id == b.id && generation == b.generation;
	}
};
struct ArchetypeBlockStorage;
struct ArchetypeBlock;
//void ArchetypeBlockStorage::DeleteBlock(ArchetypeBlockStorage * store, ArchetypeBlock * blk);
struct ArchetypeBlock {

	Archetype myArch;
	ArchetypeBlock(const Archetype &arch) {

		for (auto &m : arch.componentlist.metatypes)
		{
			ArchetypeComponentArray newArray = ArchetypeComponentArray(m);
			componentArrays.push_back(newArray);
		}

		//prev = nullptr;
		//
		//next = nullptr;


		myArch = arch;
		myArch.componentlist.BuildHash();
		last = 0;
	}

	bool checkSanity() {
		for (int i = 0; i < Archetype::ARRAY_SIZE; i++)
		{
			if (entities[i].generation != 1 && i < last)
			{
				//std::cout << "what the fuck";
				return false;
			}
		}
		return true;
	}

	template<typename C>
	TypedArchetypeComponentArray<C> GetComponentArray() {
		Metatype mc = Metatype::BuildMetatype<C>();

		for (auto &c : componentArrays)
		{
			if (c.metatype.name_hash == mc.name_hash)
			{
				TypedArchetypeComponentArray<C> tarray = TypedArchetypeComponentArray<C>(c);

				return tarray;
			}
		}

		return TypedArchetypeComponentArray<C>();
	}
	
	uint16_t AddEntity(EntityHandle handle)
	{
		uint16_t pos = last;
		assert(pos < myArch.ARRAY_SIZE);

		entities[pos] = handle;
		last++;
		//entities[last].generation = 7;
		return pos;
	}

	//copy a entity from a different block into this one
	void CopyEntityFromBlock(uint64_t destEntity, uint64_t srcEntity, ArchetypeBlock * otherblock)
	{
		//copy components to the index
		for (auto &csrc : otherblock->componentArrays)
		{
			for (auto &cthis : componentArrays)
			{
				//find a component metatype match, and copy it to this block
				if (csrc.metatype.name_hash == cthis.metatype.name_hash)
				{
					cthis.CopyFromOuter(srcEntity, destEntity, &csrc);
					//break first for
					break;
				}
			}
		}

		//copy entity handle	
		entities[destEntity] = otherblock->entities[srcEntity];
		if (destEntity >= last)
		{
			last = destEntity + 1;
		}
	}
	//returns true if the block got deleted
	bool RemoveAndSwap(uint64_t idx) {

		//shrink
		last--;


		if (last <= 0)
		{
			//block emptied
			
			return true;
		}
		if (idx >= last)
		{

			entities[last].generation = 2;
			entities[last].id = 0;
			return false;
		}

		//copy components to the index
		for (auto &ca : componentArrays)
		{
			ca.Copy(last, idx);
		}

		//copy entity handle
		entities[idx] = entities[last];
		entities[last].generation = 0;
		entities[last].id = 0;
		return false;

		//if (last == 0)
		//{
		//	if (prev != nullptr)
		//	{
		//		prev->next = 
		//	}
		//}
	}
	EntityHandle GetLastEntity() {
		return entities[last - 1];
	}
	int GetFreeSpace() {
		return myArch.ARRAY_SIZE - last;
	}

	std::vector<ArchetypeComponentArray> componentArrays;

	//handle array
	std::array<EntityHandle, Archetype::ARRAY_SIZE> entities;

	//max index that has an entity
	uint16_t last{ 0 };

	//linked list
	//ArchetypeBlock * prev;
	//ArchetypeBlock * next;

	ArchetypeBlockStorage * storage;
};


struct ArchetypeBlockStorage {


	Archetype myArch;
	int nblocks;
	ArchetypeBlockStorage(const Archetype & arch) {
		myArch = arch;
		//last = nullptr;
		//first = nullptr;
		nblocks = 0;
		//CreateNewBlock();
		//last = first;
		myArch.componentlist.BuildHash();

	}

	~ArchetypeBlockStorage() {
		//ArchetypeBlock * ptr = first;
		////iterate linked list
		//while (ptr != nullptr)
		//{
		//	auto d = ptr;
		//	ptr = ptr->next;
		//	delete d;			
		//}
	}
	ArchetypeBlock * CreateNewBlock() {
		nblocks++;

		auto b = block_colony.insert(myArch);

		ArchetypeBlock * blk = &(*b);
		//blk->next = nullptr;
		blk->storage = this;
		
		
		return blk;
	}


	void DeleteBlock(ArchetypeBlock * blk) {
		
		freeblock = nullptr;
		
		
		nblocks--;
		
		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{
			//for (auto &it : block_colony)
			//{
			if (&(*it) == blk)
			{
				it->last = 0;
				block_colony.erase(it);
				return;
			}
		}
		
	}
	template<typename F>
	void Iterate(F&& f) {

		for (auto it = block_colony.begin(); it != block_colony.end(); ++it)
		{
			if (it->last != 0)
			{
				f((*it));
			}
			
		}

		//ArchetypeBlock * ptr = first;
		////iterate linked list
		//while (ptr != nullptr)
		//{
		//	f(*ptr);
		//	ptr = ptr->next;
		//}
	}

	ArchetypeBlock * FindFreeBlock() {
		//cached freeblock
			if (freeblock != nullptr && freeblock->last < (Archetype::ARRAY_SIZE - 1))
			{
				return freeblock;
			}

		ArchetypeBlock * ptr = nullptr;
		//iterate linked list

		for (auto & b : block_colony)
		{
			ptr = &b;
			
				if (ptr->last < (Archetype::ARRAY_SIZE - 1))
				{
					freeblock = ptr;
					return ptr;
				}

				//ptr = ptr->next;
			
		}

		freeblock = nullptr;

		return nullptr;
	}

	ArchetypeBlock * freeblock{ nullptr };
	//ArchetypeBlock * first{ nullptr };
	//ArchetypeBlock * last{ nullptr };

	plf::colony<ArchetypeBlock> block_colony;

	static void DeleteBlock(ArchetypeBlockStorage * store, ArchetypeBlock * blk)
	{
		store->DeleteBlock(blk);
	}
};



template <typename Derived>
struct Component : public BaseComponent {

	//private:
	friend class EntityManager;

	static ComponentGUID GUID();
};

template <typename C>
ComponentGUID Component<C>::GUID() {
	static ComponentGUID family = family_counter_++;
	return family;
}


struct Position : public Component<Position> {

	Position() = default;
	Position(const Position & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};

struct C1 : public Component<C1> {

	C1() = default;
	C1(const C1 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
struct C2 : public Component<C2> {

	C2() = default;
	C2(const C2 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
struct C3 : public Component<C3> {

	C3() = default;
	C3(const C3 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
//};
struct Rotation : public Component<Rotation> {
	Rotation() = default;
	Rotation(const Rotation & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;

};
struct Speed : public Component<Speed> {
	Speed() = default;

	float x;

};
struct Acceleration : public Component<Acceleration> {
	Acceleration() = default;

	float x;

};
struct BigComponent : public Component<BigComponent> {

	//static constexpr ComponentGUID GUID = 324132;
	int data[10000];
};



struct EntityStorage {
	size_t generation{ 0 };
	//size_t block;
	ArchetypeBlock * block{ nullptr };
	uint16_t blockindex{ 0 };
};
struct ECSWorld {



	template<typename F>
	void IterateBlocks(const ComponentList &AllOfList, const ComponentList& NoneOfList, F && f) {

		for (ArchetypeBlockStorage & b : BlockStorage)
		{
			if (b.myArch.Match(AllOfList) == AllOfList.metatypes.size())
			{
				if (b.myArch.Match(NoneOfList) == 0)
				{
					b.Iterate(f);
					//f(b);
				}
			}
		}
	}
	template<typename F>
	void IterateBlocks(const ComponentList &AllOfList, F && f) {

		for (ArchetypeBlockStorage & b : BlockStorage)
		{
			if (b.myArch.Match(AllOfList) == AllOfList.metatypes.size())
			{
				b.Iterate(f);
			}
		}
	}
	bool ValidateAll() {
		bool valid = true;
		for (ArchetypeBlockStorage & b : BlockStorage)
		{
			b.Iterate([&](auto & blk) {
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

		//for (ArchetypeBlockStorage & b : BlockStorage)
		for (size_t i = 0; i < BlockStorage.size(); i++)
		{
			ArchetypeBlockStorage * b = &BlockStorage[i];
			//block needs to have the same amount of components, and match all of them
			if (b->myArch.ExactMatch(arc.componentlist))
			{
				entityBlock = b->FindFreeBlock();
				if (entityBlock == nullptr)
				{
					entityBlock = b->CreateNewBlock();
					break;
					
				}
			}
		}
		//block not found, create a new one
		if (entityBlock == nullptr)
		{
			//auto str = ArchetypeBlockStorage(arc);
			BlockStorage.push_back(ArchetypeBlockStorage(arc));
			entityBlock = BlockStorage[BlockStorage.size() - 1].CreateNewBlock();
			//entityBlock = CreateBlock(arc);
		}

		return entityBlock;
	}

	std::vector<EntityHandle> CreateEntityBatched(Archetype & arc, size_t amount)
	{
		arc.componentlist.BuildHash();
		//Entities.reserve(Entities.size() + amount);
		std::vector<EntityHandle> Handles;
		Handles.reserve(amount);
		size_t amount_left = amount;
		bool bCanReuseEntities = CanReuseEntity();
		int reuses = deletedEntities.size();
		int newcreations = amount - reuses;
		if (newcreations > 0)
		{
			Entities.reserve(Entities.size() + newcreations);
		}

		while (amount_left > 0)
		{
			ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);
			int freespace = entityBlock->GetFreeSpace();
			for (int i = 0; i < freespace; i++) {

				if (amount_left <= 0)
				{
					return Handles;
				}
				EntityHandle newEntity;
				bool bReuse = CanReuseEntity();
				size_t index = 0;
				if (bCanReuseEntities && bReuse) {
					assert(Entities[index].block == nullptr);
					index = deletedEntities.front();
					deletedEntities.pop_front();
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

				uint16_t pos = entityBlock->AddEntity(newEntity);

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
		arc.componentlist.BuildHash();
		EntityHandle newEntity;
		
		bool bReuse = CanReuseEntity();
		size_t index = 0;
		if (bReuse) {
			assert(Entities[index].block == nullptr);
			index = deletedEntities.front();
			deletedEntities.pop_front();
			newEntity.id = index;
			newEntity.generation = Entities[index].generation + 1;
		}
		else
		{
			newEntity.id = Entities.size();
			newEntity.generation = 1;
		}
		

		ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);

		//insert entity handle into the block
		uint16_t pos = entityBlock->AddEntity(newEntity);
		
		EntityStorage et;
		et.block = entityBlock;
		et.blockindex = pos;
		et.generation = newEntity.generation;

		if (bReuse){
			Entities[index] = et;
		}
		else {
			Entities.push_back(et);
		}
		

		return newEntity;
	}

	template<typename C>
	void AddComponent(EntityHandle entity)		//, C&comp)
	{
		const EntityStorage & et = Entities[entity.id];
		//valid entity
		if (Valid(entity)) {
			static Archetype cached;

			cached.SetAdd<C>(et.block->myArch);

			//static Archetype newarch = et.block->myArch;
			//newarch.AddComponent<C>();
			//Archetype newarch = Archetype::CreateAdd<C>(et.block->myArch);
			SetEntityArchetype(entity, cached);// newarch);
		}
	}
	template<typename C>
	void RemoveComponent(EntityHandle entity)		//, C&comp)
	{
		const EntityStorage & et = Entities[entity.id];
		//valid entity
		if (Valid(entity)) {

			static Archetype cached;
			cached.SetRemove<C>(et.block->myArch);

			//Archetype newarch = Archetype::CreateRemove<C>(et.block->myArch);// et.block->myArch;
			//newarch.RemoveComponent<C>();


			SetEntityArchetype(entity, cached);
			//ValidateAll();

		}
	}

	void DeleteEntity(EntityHandle entity) {
		if (Valid(entity)) {
			EntityStorage & et = Entities[entity.id];

			//delete the entity from its block
			ArchetypeBlock * oldBlock = et.block;
			auto oldpos = et.blockindex;
			if (oldBlock->RemoveAndSwap(oldpos))
			{
				oldBlock->storage->DeleteBlock(oldBlock);
			}

			deletedEntities.push_back(entity.id);

			et.block = nullptr;
			et.blockindex = 0;
			et.generation++;

		}
	}
	bool Valid(EntityHandle entity)
	{
		const EntityStorage & et = Entities[entity.id];
		//valid entity
		return (et.generation == entity.generation && et.block != nullptr && et.block->last <= Archetype::ARRAY_SIZE && et.block->last >= 0);

	}

	void SetEntityArchetype(EntityHandle entity, Archetype &arc) {

		EntityStorage & et = Entities[entity.id];

		if (Valid(entity)) {

			ArchetypeBlock * oldBlock = et.block;

			ArchetypeBlock * newBlock = FindOrCreateBlockForArchetype(arc);
		
			//create entity in the new block
			auto pos = newBlock->AddEntity(entity);
			auto oldpos = et.blockindex;
			//copy old block entity into new block entity
			newBlock->CopyEntityFromBlock(pos, oldpos, oldBlock);

			//clear old entity slot
			EntityHandle SwapEntity = oldBlock->GetLastEntity();

			
			Entities[SwapEntity.id].blockindex = oldpos;


			if (oldBlock->RemoveAndSwap(oldpos))
			{
				oldBlock->storage->DeleteBlock(oldBlock);	
			}
			
			//update the low level data
			et.block = newBlock;
			et.blockindex = pos;
		}
	}
	
	bool CanReuseEntity() {
		return !deletedEntities.empty();
	}

	std::vector<EntityStorage> Entities;
	std::deque<size_t> deletedEntities;
	std::vector<ArchetypeBlockStorage> BlockStorage;
};