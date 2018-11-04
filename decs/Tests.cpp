

#include "decs.hpp"
#include "ExampleComponents.h"

BaseComponent::IDCounter BaseComponent::family_counter_ = 1;


#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

Archetype Arc_Pos_Rot;
Archetype Arc_Position;
Archetype Arc_Pos_Rot_Big;
Archetype Arc_BigComponent;
Archetype Arc_Rotation;

int main(int argc, char* argv[]) {
	// global setup...

	//initialize some common archetypes
	Arc_Pos_Rot.AddComponent<Position>();
	Arc_Pos_Rot.AddComponent<Rotation>();	

	Arc_Pos_Rot_Big.AddComponent<Position>();
	Arc_Pos_Rot_Big.AddComponent<Rotation>();
	Arc_Pos_Rot_Big.AddComponent<BigComponent>();

	Arc_Position.AddComponent<Position>();

	Arc_BigComponent.AddComponent<BigComponent>();

	Arc_Rotation.AddComponent<Rotation>();




	int result = Catch::Session().run(argc, argv);

	// global clean-up...

	return result;
}

TEST_CASE("Archetype: 2 Components Added - Normal") {

	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	//check size is 2
	REQUIRE(Arc_Pos_Rot.componentlist.metatypes.size() == 2);
}
TEST_CASE("Archetype: 2 Components Added - Same Component twice") {

	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Position>();

	//check size is 2
	REQUIRE(PosRot.componentlist.metatypes.size() == 1);
}
TEST_CASE("Archetype: 2 Components Added - 1 Removed") {

	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	
	PosRot.RemoveComponent<Rotation>();
	auto mt = Metatype::BuildMetatype<Position>();
	//check size is 2
	REQUIRE(PosRot.componentlist.metatypes.size() == 1);
	REQUIRE(PosRot.componentlist.metatypes[0].name_hash == mt.name_hash);
}

TEST_CASE("Archetype: Matches itself") {

	Archetype PosRotBig;
	PosRotBig.AddComponent<Position>();
	PosRotBig.AddComponent<Rotation>();
	PosRotBig.AddComponent<BigComponent>();
	
	//check size is 2
	REQUIRE(PosRotBig.Match(PosRotBig.componentlist) == 3);
	REQUIRE(PosRotBig.ExactMatch(PosRotBig.componentlist));
}

TEST_CASE("Archetype: 2 Components Added - 2 Removed") {

	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	PosRot.RemoveComponent<Position>();
	PosRot.RemoveComponent<Rotation>();

	//check size is 2
	REQUIRE(PosRot.componentlist.metatypes.size() == 0);
}

TEST_CASE("Archetype: 2 Components Added - Repeated remove") {

	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	PosRot.RemoveComponent<Rotation>();
	PosRot.RemoveComponent<Rotation>();
	PosRot.RemoveComponent<Rotation>();
	PosRot.RemoveComponent<Rotation>();
	PosRot.RemoveComponent<Rotation>();

	//check size is 2
	REQUIRE(PosRot.componentlist.metatypes.size() == 1);
}

TEST_CASE("Entity Creation: 1 Entity") {


	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntity(Arc_Pos_Rot);

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);		
		
		//entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntity(Arc_Pos_Rot_Big);

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);		
		
		//entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntity(Arc_Position);

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		
		//entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}

TEST_CASE("Entity Create and Destroy: 1 Entity") {


	ECSWorld ECS;


	SECTION("Creating and destroying one entity") {

		EntityHandle handle = ECS.CreateEntity(Arc_Pos_Rot);
		ECS.DeleteEntity(handle);
		REQUIRE(handle.id == 0);
		REQUIRE(ECS.Valid(handle) == false);
		REQUIRE(ECS.BlockStorage.size() == 1);

		//entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Creating and destroying one entity: should not match") {
		EntityHandle handle = ECS.CreateEntity(Arc_Pos_Rot);
		ECS.DeleteEntity(handle);
		REQUIRE(handle.id == 0);
		REQUIRE(ECS.Valid(handle) == false);
		
		REQUIRE(ECS.BlockStorage.size() == 1);

		int BlocksIterated = 0;
		int EntitiesIterated = 0;
		int HandleMatches = 0;
		ECS.IterateBlocks(Arc_Rotation.componentlist,[&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(BlocksIterated == 0);
		REQUIRE(EntitiesIterated == 0);
		REQUIRE(HandleMatches == 0);
		//entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	
	SECTION("Creating and destroying one entity 1000 times: should not match") {
		EntityHandle handle;
		for (int i = 0; i < 1000; i++) {

			handle = ECS.CreateEntity(Arc_Pos_Rot);
			ECS.DeleteEntity(handle);
			
		}
		
		REQUIRE(handle.id == 0);
		REQUIRE(ECS.Valid(handle) == false);
		REQUIRE(ECS.BlockStorage.size() == 1);


		int BlocksIterated = 0;
		int EntitiesIterated = 0;
		int HandleMatches = 0;
		ECS.IterateBlocks(Arc_Rotation.componentlist, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(BlocksIterated == 0);
		REQUIRE(EntitiesIterated == 0);
		REQUIRE(HandleMatches == 0);
		//entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}


TEST_CASE("Entity Creation: 1 Entity - Batched") {
	

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot, 1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//REQUIRE(ECS.BlockStorage[0].first->last == 1);
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot_Big, 1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//REQUIRE(ECS.BlockStorage[0].first->last == 1);
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Position, 1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//REQUIRE(ECS.BlockStorage[0].first->last == 1);
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}


TEST_CASE("Entity Creation: 1 Maxed Entity Block - Batched") {
	

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot, Archetype::ARRAY_SIZE)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 1);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);		
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot, Archetype::ARRAY_SIZE)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 1);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot, Archetype::ARRAY_SIZE)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 1);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}


TEST_CASE("Entity Creation: 1 Maxed Entity Block + 1 - Batched") {
	

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot, Arc_Pos_Rot.ARRAY_SIZE +1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot_Big, Arc_Pos_Rot_Big.ARRAY_SIZE +1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Position, Arc_Position.ARRAY_SIZE +1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}


TEST_CASE("Entity Creation: 1 Million entities - Batched") {
	
	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot,  1000000)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
		//REQUIRE(ECS.ValidateAll());
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot_Big, 1000000)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
		//REQUIRE(ECS.ValidateAll());
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot, 1000000)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		//REQUIRE(ECS.BlockStorage[0].first != nullptr);
		//REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//
		////block stores the correct handle
		//REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		////entity stores the correct pointer
		//REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
		//REQUIRE(ECS.ValidateAll());
	}
}

TEST_CASE("Iteration: 1 entity - Queries ")
{
	

	ComponentList PositionSearch = Arc_Position.componentlist;
	ComponentList PosRotBigSearch = Arc_Pos_Rot_Big.componentlist;
	ComponentList empty;

	ECSWorld ECS;

	int BlocksIterated = 0;
	int EntitiesIterated = 0;
	int HandleMatches = 0;

	SECTION("Match one component - basic") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot_Big, 1)[0];


		ECS.IterateBlocks(PositionSearch, empty, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(BlocksIterated == 1);
		REQUIRE(HandleMatches == 1);
		REQUIRE(EntitiesIterated == 1);


	}
	SECTION("Match empty - cant match") {

		//EntityHandle handle = ECS.CreateEntityBatched(PosOnly, 1)[0];

		ECS.IterateBlocks(PosRotBigSearch, empty, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;
			}
		});
		

		//should not match anything
		REQUIRE(BlocksIterated == 0);
		REQUIRE(HandleMatches == 0);
		REQUIRE(EntitiesIterated == 0);
	}

	SECTION("Match one component - cant match") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Position, 1)[0];

		ECS.IterateBlocks(PosRotBigSearch, empty, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);

		//should not match anything
		REQUIRE(BlocksIterated == 0);
		REQUIRE(HandleMatches == 0);
		REQUIRE(EntitiesIterated == 0);
	}
}
TEST_CASE("Iteration: 1 entity - Negative Queries ")
	{
		

		ComponentList PositionSearch = Arc_Position.componentlist;
		ComponentList RotationSearch = Arc_Rotation.componentlist;
		ComponentList PosRotBigSearch = Arc_Pos_Rot_Big.componentlist;
		ComponentList empty;

		ECSWorld ECS;

		int BlocksIterated = 0;
		int EntitiesIterated = 0;
		int HandleMatches = 0;

	SECTION("Match one component - query against itself") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Position, 1)[0];


		//try to match against its own negative query. Should not match
		ECS.IterateBlocks(PositionSearch, PositionSearch, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);

		//should not match anything
		REQUIRE(BlocksIterated == 0);
		REQUIRE(HandleMatches == 0);
		REQUIRE(EntitiesIterated == 0);
	}

	SECTION("Match one component - negative query on big") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Pos_Rot_Big, 1)[0];


		// Should not match
		ECS.IterateBlocks(PosRotBigSearch, RotationSearch, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);

		//should not match anything
		REQUIRE(BlocksIterated == 0);
		REQUIRE(HandleMatches == 0);
		REQUIRE(EntitiesIterated == 0);
	}
	SECTION("Match one component - big negaive query") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Rotation, 1)[0];


		// Should not match
		ECS.IterateBlocks(RotationSearch, PosRotBigSearch, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);

		//should not match anything
		REQUIRE(BlocksIterated == 0);
		REQUIRE(HandleMatches == 0);
		REQUIRE(EntitiesIterated == 0);
	}
}

TEST_CASE("Iteration: 1 full block - Queries ")
{
	

	ComponentList PositionSearch = Arc_Position.componentlist;
	ComponentList PosRotBigSearch = Arc_Pos_Rot_Big.componentlist;
	ComponentList empty;

	ECSWorld ECS;

	int BlocksIterated = 0;
	int EntitiesIterated = 0;
	int HandleMatches = 0;

	SECTION("Match one block - basic") {

		ECS.CreateEntityBatched(Arc_Pos_Rot_Big, Arc_Pos_Rot_Big.ARRAY_SIZE)[0];


		ECS.IterateBlocks(PositionSearch, empty, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				
			}
		});

		
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(BlocksIterated == 1);		
		REQUIRE(EntitiesIterated == Arc_Pos_Rot_Big.ARRAY_SIZE);

	}
	SECTION("Match one block - complex") {

		ECS.CreateEntityBatched(Arc_Pos_Rot_Big, Arc_Pos_Rot_Big.ARRAY_SIZE)[0];


		ECS.IterateBlocks(PosRotBigSearch, empty, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;


			}
		});


		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(BlocksIterated == 1);
		REQUIRE(EntitiesIterated == Arc_Pos_Rot_Big.ARRAY_SIZE);

	}

	SECTION("Match one component - cant match") {

		EntityHandle handle = ECS.CreateEntityBatched(Arc_Position, Arc_Position.ARRAY_SIZE)[0];

		ECS.IterateBlocks(PosRotBigSearch, empty, [&](ArchetypeBlock & block) {
			BlocksIterated++;
			auto &ap = block.entities;//block.GetComponentArray<Position>();

			for (int i = 0; i < block.last; i++)
			{
				EntitiesIterated++;

				if (ap[i] == handle)
				{
					HandleMatches++;
				}
			}
		});

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);

		//should not match anything
		REQUIRE(BlocksIterated == 0);
		REQUIRE(HandleMatches == 0);
		REQUIRE(EntitiesIterated == 0);
	}
}