
#include "stdint.h"
#include <vector>
#include <array>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <chrono>
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
BaseComponent::IDCounter BaseComponent::family_counter_ = 1;

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

struct ComponentList {
	std::vector<Metatype> metatypes;

	void BuildHash() {
		cached_hash = 0;

		for (auto m : metatypes)
		{
			long hash = hash64shift(m.name_hash);
			cached_hash |= hash;
		}

	}
	std::size_t cached_hash{0};
};

struct Archetype{

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
	
};
constexpr size_t ARRAY_SIZE = 4086;


struct ArchetypeComponentArray {
	void * data;
	Metatype metatype;

	ArchetypeComponentArray(Metatype type) {
		data = malloc(ARRAY_SIZE * type.size);
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
	T * data{nullptr};
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
};
struct ArchetypeBlockStorage;
struct ArchetypeBlock;
void DeleteBlock(ArchetypeBlockStorage * store, ArchetypeBlock * blk);
struct ArchetypeBlock {

	Archetype myArch;
	ArchetypeBlock(const Archetype &arch) {

		for (auto &m : arch.componentlist.metatypes)
		{
			ArchetypeComponentArray newArray = ArchetypeComponentArray(m);
			componentArrays.push_back(newArray);
		}

		myArch = arch;
		myArch.componentlist.BuildHash();
		last = 0;
	}

	bool checkSanity() {
		for (int i = 0; i < ARRAY_SIZE; i++)
		{
			if (entities[i].generation != 1 && i < last)
			{
				std::cout << "what the fuck";
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
	/*
	int Match(const ComponentList & Components)
	{
		
		int matches = 0;
		for (auto c : Components.metatypes)
		{
			bool bFound = false;
			for (auto a : componentArrays)
			{
				if (a.metatype.name_hash == c.name_hash)
				{
					bFound = true;
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
		if (Components.cached_hash != myArch.componentlist.cached_hash) {
			return false;
		}
		for (auto c : Components.metatypes)
		{
			bool bFound = false;
			for (auto a : componentArrays)
			{
				if (a.metatype.name_hash != c.name_hash)
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
	*/
	uint16_t AddEntity(EntityHandle handle)
	{
		uint16_t pos = last;
		entities[pos] = handle;
		last++;
		entities[last].generation = 5;
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
			DeleteBlock(storage, this);
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
		entities[last].id  = 0;
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
		return entities[last-1];
	}

	std::vector<ArchetypeComponentArray> componentArrays;

	//handle array
	EntityHandle entities[ARRAY_SIZE];

	//max index that has an entity
	uint16_t last{0};

	//linked list
	ArchetypeBlock * prev{nullptr};
	ArchetypeBlock * next{ nullptr };

	ArchetypeBlockStorage * storage;
};


struct ArchetypeBlockStorage {


	Archetype myArch;
	int nblocks;
	ArchetypeBlockStorage(const Archetype & arch) {
		myArch = arch;
		last = nullptr;
		first = nullptr;
		nblocks = 0;
		//CreateNewBlock();
		//last = first;
		myArch.componentlist.BuildHash();

	}

	ArchetypeBlock * CreateNewBlock() {
		nblocks++;
		ArchetypeBlock * blk = new ArchetypeBlock(myArch);
		blk->next = nullptr;
		blk->storage = this;
		blk->prev = last;
		if (last != nullptr)
		{
			last->next = blk;
		}
		if (first == nullptr)
		{
			first = blk;
		}
		last = blk;
		return blk;
	}

	
	void DeleteBlock(ArchetypeBlock * blk) {
		if (blk == freeblock)
		{
			freeblock = nullptr;
		}
		if (first == nullptr)
		{
			return;
		}
		nblocks--;
		ArchetypeBlock * ptr = first;
		//iterate linked list
		while (ptr != nullptr)
		{
			if (ptr == blk)
			{
				ArchetypeBlock * prev = ptr->prev;
				ArchetypeBlock * next = ptr->next;
				//check that the previus pointer is blid (list head)
				if (prev != nullptr)
				{
					prev->next = next;
				}
				else
				{
					first = next;
				}
				//check that the next pointer is blid (list end)
				if (next != nullptr)
				{
					next->prev = prev;
				}
				else
				{
					last = prev;
				}
				delete ptr;
				return;
			}
			else
			{
				ptr = ptr->next;
			}
		}
	}
	template<typename F>
	void Iterate(F&&f) {
		ArchetypeBlock * ptr = first;
		//iterate linked list
		while (ptr != nullptr)
		{
			f(*ptr);
			ptr = ptr->next;
		}
	}

	ArchetypeBlock * FindFreeBlock() {
		//cached freeblock
		if (freeblock != nullptr && freeblock->last < (ARRAY_SIZE - 1))
		{
			return freeblock;
		}

		ArchetypeBlock * ptr = first;
		//iterate linked list
		while (ptr != nullptr)
		{
			if (ptr->last < (ARRAY_SIZE - 1))
			{
				freeblock = ptr;
				return ptr;
			}
			
			ptr = ptr->next;
		}

		freeblock = ptr;

		return ptr;
	}

	ArchetypeBlock * freeblock{nullptr};
	ArchetypeBlock * first{nullptr};
	ArchetypeBlock * last{ nullptr };
};

void DeleteBlock(ArchetypeBlockStorage * store, ArchetypeBlock * blk)
{
	store->DeleteBlock(blk);
}

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
	size_t generation{0};
	//size_t block;
	ArchetypeBlock * block{nullptr};
	uint16_t blockindex{0};
};
struct ECSWorld {



	template<typename F>
	void IterateBlocks(const ComponentList &AllOfList,const ComponentList& NoneOfList, F&&f  ) {

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

	void ValidateAll() {
		for (ArchetypeBlockStorage & b : BlockStorage)
		{
			b.Iterate([](auto & blk) {
				blk.checkSanity();
			});
		}

	}

	//ArchetypeBlock * CreateBlock(const Archetype & arc) {
	//	
	//	for (auto & b : BlockStorage)
	//	{
	//		if (b.ExactMatch(arc.componentlist))
	//		{
	//			return	b.CreateNewBlock();
	//		}
	//	}
	//
	//	//no archetype found that has that
	//	return nullptr;
	//
	//
	//	//Blocks.push_back( ArchetypeBlock(arc) );
	//	//return &Blocks[Blocks.size() - 1];
	//}

	ArchetypeBlock * FindOrCreateBlockForArchetype(const Archetype & arc)
	{
		//ArchetypeBlock * entityBlock = nullptr;
		////find the free block
		////arc.componentlist.BuildHash();
		//const size_t numComponents = arc.componentlist.metatypes.size();
		//for (ArchetypeBlock & b : Blocks)
		//{
		//	//block needs to have the same amount of components, and match all of them
		//	if (b.componentArrays.size() == numComponents && b.ExactMatch(arc.componentlist))//b.Match(arc.componentlist) == numComponents)
		//	{
		//		//block cant be filled
		//		if (b.last < ARRAY_SIZE - 1)
		//		{
		//			entityBlock = &b;
		//			break;
		//		}
		//
		//	}
		//}
		////block not found, create a new one
		//if (entityBlock == nullptr)
		//{
		//	entityBlock = CreateBlock(arc);
		//}
		//
		//return entityBlock;

		ArchetypeBlock * entityBlock = nullptr;
		//find the free block
		//arc.componentlist.BuildHash();
		const size_t numComponents = arc.componentlist.metatypes.size();
		
		//for (ArchetypeBlockStorage & b : BlockStorage)
		for(size_t i = 0;i < BlockStorage.size(); i++)
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
					//return entityBlock;
				}	
			}
		}
		//block not found, create a new one
		if (entityBlock == nullptr)
		{
			//auto str = ArchetypeBlockStorage(arc);
			BlockStorage.push_back( ArchetypeBlockStorage(arc) );
			entityBlock = BlockStorage[BlockStorage.size() - 1].CreateNewBlock();
			//entityBlock = CreateBlock(arc);
		}

		return entityBlock;
	}

	std::vector<EntityHandle> CreateEntityBatched(Archetype & arc, size_t amount)
	{
		arc.componentlist.BuildHash();
		Entities.reserve(Entities.size() + amount);
		std::vector<EntityHandle> Handles;
		Handles.reserve(amount);
		size_t amount_left = amount;
		while (amount_left > 0)
		{
			ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);

			for (int i = 0; i < ((ARRAY_SIZE - 1) - entityBlock->last); i++) {

				if (amount_left <= 0)
				{
					return Handles;
				}
				EntityHandle newEntity;
				newEntity.id = Entities.size();
				newEntity.generation = 1;

				uint16_t pos = entityBlock->AddEntity(newEntity);				

				EntityStorage et;
				et.block = entityBlock;
				et.blockindex = pos;
				et.generation = 1;

				Entities.push_back(et);
				Handles.push_back(newEntity);
				amount_left--;
			}
		}
		return Handles;
	}
	EntityHandle CreateEntity(Archetype & arc) {
		arc.componentlist.BuildHash();
		EntityHandle newEntity;
		newEntity.id = Entities.size();
		newEntity.generation = 1;

		ArchetypeBlock * entityBlock = FindOrCreateBlockForArchetype(arc);


		//insert entity handle into the block


		uint16_t pos = entityBlock->AddEntity(newEntity);
		//auto pos = entityBlock->last;
		//entityBlock->entities[pos] = newEntity;
		//entityBlock->last++;

		EntityStorage et;
		et.block = entityBlock;
		et.blockindex = pos;
		et.generation = 1;

		Entities.push_back(et);
		//Entities[Entities.size()-1].block = entityBlock;
		//Entities[Entities.size() - 1].blockindex = pos;
		//Entities[Entities.size() - 1].generation = 1;

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

	bool Valid(EntityHandle entity)
	{
		const EntityStorage & et = Entities[entity.id];
		//valid entity
		return (et.generation == entity.generation && et.block != nullptr && et.block->last <= ARRAY_SIZE && et.block->last >= 0);
			
	}

	void SetEntityArchetype(EntityHandle entity, Archetype &arc) {

		EntityStorage & et = Entities[entity.id];

		if (Valid(entity)) {

			ArchetypeBlock * oldBlock = et.block;

			ArchetypeBlock * newBlock = FindOrCreateBlockForArchetype(arc);
			//oldBlock->checkSanity();
			//newBlock->checkSanity();
			//create entity in the new block
			auto pos = newBlock->AddEntity(entity);
			auto oldpos = et.blockindex;
			//copy old block entity into new block entity
			newBlock->CopyEntityFromBlock(pos, oldpos, oldBlock);

			//clear old entity slot
			EntityHandle SwapEntity = oldBlock->GetLastEntity();

			//if (pos > oldBlock->last)
			//{
			//	std::cout << " what";
			//}
			Entities[SwapEntity.id].blockindex = oldpos;
			
			
			if (!oldBlock->RemoveAndSwap(oldpos))
			{
				//oldBlock->checkSanity();				
			}
			
			//newBlock->checkSanity();
			//update the low level data
			et.block = newBlock;
			et.blockindex = pos;
		}
	}

	std::vector<EntityStorage> Entities;
	std::vector<ArchetypeBlock> Blocks;
	std::vector<ArchetypeBlockStorage> BlockStorage;
};
using namespace std;
//using namespace decs;




int main()
{
	for (int r = 0; r < 100; r++)
	{


		Archetype PosRot;
		PosRot.AddComponent<Position>();
		PosRot.AddComponent<Rotation>();
		PosRot.AddComponent<BigComponent>();


		ECSWorld Blocks;
		//Blocks.Blocks.reserve(50000);
		Blocks.BlockStorage.reserve(100);
		//Blocks.CreateBlock(PosRot);


		//ArchetypeBlock block(PosRot);

		std::vector<Position> Positions1;
		std::vector<Rotation> Rotations1;
		std::vector<EntityHandle> Handles;
		Archetype PR;
		PR.AddComponent<Position>();
		PR.AddComponent<Rotation>();

		ComponentList search = PR.componentlist;
		ComponentList empty;
		ComponentList noBig;
		//Blocks.CreateBlock(PR);

		noBig.metatypes.push_back(Metatype::BuildMetatype<BigComponent>());

		auto start = chrono::high_resolution_clock::now();

		//for (int i = 0; i < 1000000; i++)
		//{
		//	/*Handles.push_back(*/ Blocks.CreateEntity(PR);// / / );
		//}
		Handles = Blocks.CreateEntityBatched(PR, 1000000);
		
		auto end = chrono::steady_clock::now();
		//Blocks.ValidateAll();

		auto diff = end - start;

		cout << chrono::duration <double, milli>(diff).count() << " ms: creating 1 million entities" << endl;

		start = chrono::high_resolution_clock::now();
		for (int i = 0; i < Handles.size(); i++)
		{
			if (i % 3 == 0)
			{
				Blocks.AddComponent<Acceleration>(Handles[i]);//, BigComponent());
			}
			//if (i % 5 == 0)
			//{
			//	Blocks.RemoveComponent<Rotation>(Handles[i]);//, BigComponent());
			//}
		}
		end = chrono::steady_clock::now();

		//Blocks.ValidateAll();
		diff = end - start;

		cout << chrono::duration <double, milli>(diff).count() << " ms adding a component to one third of the million entities" << endl;


		start = chrono::high_resolution_clock::now();
		for (int i = 0; i < Handles.size(); i++)
		{
		
			auto eh = Handles[i];
			Blocks.RemoveComponent<Position>(eh);//, BigComponent());
			
		}
		end = chrono::steady_clock::now();
	

		diff = end - start;

		cout << chrono::duration <double, milli>(diff).count() << " ms removing a component from all million entities" << endl;
		
		start = chrono::high_resolution_clock::now();
		for (int i = 0; i < Handles.size(); i++)
		{
			Blocks.AddComponent<Position>(Handles[i]);
			
		}
		end = chrono::steady_clock::now();
		diff = end - start;
		cout << chrono::duration <double, milli>(diff).count() << " ms adding a component to all million entities" << endl;
		start = chrono::high_resolution_clock::now();


		int niter = 0;
		Blocks.IterateBlocks(search, empty, [&](ArchetypeBlock & block) {
			niter++;
			//auto ap = block.GetComponentArray<Position>();
			//	auto ar = block.GetComponentArray<Rotation>();
			//for (int i = 0; i < block.last; i++)
			//{
			//	Position p;
			//	p.x = i;
			//	p.y = i % 2;
			//	p.z = p.x + p.y;
			//	Rotation r;
			//	r.x = p.y;
			//	r.y = p.z;
			//	r.z = p.x;
			//
			//	//Positions1.push_back(p);
			//	//Rotations1.push_back(r);
			//
			//	Position &p1 = ap.Get(i);// = p;
			//	Rotation &r1 = ar.Get(i);// = r;
			//	p1 = p;
			//	r1 = r;
			//}
		});

		end = chrono::steady_clock::now();


		diff = end - start;

		cout << chrono::duration <double, milli>(diff).count() << " ms iterating 1 million entities: " << niter<<" blocks" << endl;

		Blocks.IterateBlocks(search, noBig, [&](ArchetypeBlock & block) {

			for (int i = 0; i < block.last; i++)
			{
				//Position p = Positions1[i];
				//
				//Rotation r = Rotations1[i];

				Position p2 = block.GetComponentArray<Position>().Get(i);// = p;
				Rotation r2 = block.GetComponentArray<Rotation>().Get(i);// = r;
			}
		});


		start = chrono::high_resolution_clock::now();

		Blocks.IterateBlocks(noBig, empty, [&](ArchetypeBlock & block) {

			for (int i = 0; i < block.last; i++)
			{
				//Position p = Positions1[i];
				//
				//Rotation r = Rotations1[i];

				Position p2 = block.GetComponentArray<Position>().Get(i);// = p;
				Rotation r2 = block.GetComponentArray<Rotation>().Get(i);// = r;
			}
		});

		end = chrono::steady_clock::now();

		diff = end - start;

		cout << chrono::duration <double, milli>(diff).count() << " ms iterating 1 million entities: component that doesnt exist" << endl;


	}
	return 0;

	
}
