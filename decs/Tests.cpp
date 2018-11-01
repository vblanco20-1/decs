#include "catch.hpp"
#include "decs.hpp"


BaseComponent::IDCounter BaseComponent::family_counter_ = 1;


TEST_CASE("Archetype: 2 Components Added - Normal") {

	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	//check size is 2
	REQUIRE(PosRot.componentlist.metatypes.size() == 2);
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
	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	Archetype PosOnly;
	PosOnly.AddComponent<Position>();
	

	Archetype PosRotBig;
	PosRotBig.AddComponent<Position>();
	PosRotBig.AddComponent<Rotation>();
	PosRotBig.AddComponent<BigComponent>();

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntity(PosRot);

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		REQUIRE(ECS.BlockStorage[0].first->last == 1);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntity(PosRotBig);

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		REQUIRE(ECS.BlockStorage[0].first->last == 1);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntity(PosOnly);

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		REQUIRE(ECS.BlockStorage[0].first->last == 1);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}



TEST_CASE("Entity Creation: 1 Entity - Batched") {
	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	Archetype PosOnly;
	PosOnly.AddComponent<Position>();


	Archetype PosRotBig;
	PosRotBig.AddComponent<Position>();
	PosRotBig.AddComponent<Rotation>();
	PosRotBig.AddComponent<BigComponent>();

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(PosRot, 1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		REQUIRE(ECS.BlockStorage[0].first->last == 1);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosRotBig, 1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		REQUIRE(ECS.BlockStorage[0].first->last == 1);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosOnly, 1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		REQUIRE(ECS.BlockStorage[0].first->last == 1);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}


TEST_CASE("Entity Creation: 1 Maxed Entity Block - Batched") {
	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	Archetype PosOnly;
	PosOnly.AddComponent<Position>();


	Archetype PosRotBig;
	PosRotBig.AddComponent<Position>();
	PosRotBig.AddComponent<Rotation>();
	PosRotBig.AddComponent<BigComponent>();

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(PosRot, Archetype::ARRAY_SIZE)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);		
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosRot, Archetype::ARRAY_SIZE)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosRot, Archetype::ARRAY_SIZE)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 1);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}


TEST_CASE("Entity Creation: 1 Maxed Entity Block + 1 - Batched") {
	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	Archetype PosOnly;
	PosOnly.AddComponent<Position>();


	Archetype PosRotBig;
	PosRotBig.AddComponent<Position>();
	PosRotBig.AddComponent<Rotation>();
	PosRotBig.AddComponent<BigComponent>();

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(PosRot, PosRot.ARRAY_SIZE +1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosRot, PosRot.ARRAY_SIZE +1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);

		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosRot, PosRot.ARRAY_SIZE +1)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);

		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
	}
}


TEST_CASE("Entity Creation: 1 Million entities - Batched") {
	//archetype with posrot
	Archetype PosRot;
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();

	Archetype PosOnly;
	PosOnly.AddComponent<Position>();


	Archetype PosRotBig;
	PosRotBig.AddComponent<Position>();
	PosRotBig.AddComponent<Rotation>();
	PosRotBig.AddComponent<BigComponent>();

	ECSWorld ECS;
	

	SECTION("Adding Pos + Rot (2 Components)") {

		EntityHandle handle = ECS.CreateEntityBatched(PosRot,  1000000)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);
		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
		REQUIRE(ECS.ValidateAll());
	}
	SECTION("Adding Pos + Rot + Big(3 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosRot, 1000000)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);

		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
		REQUIRE(ECS.ValidateAll());
	}
	SECTION("Adding Pos (1 Components)") {
		EntityHandle handle = ECS.CreateEntityBatched(PosRot, 1000000)[0];

		REQUIRE(handle.id == 0);
		REQUIRE(handle.generation == 1);
		REQUIRE(ECS.BlockStorage.size() == 1);
		//REQUIRE(ECS.BlockStorage[0].nblocks == 2);
		//test block validity
		REQUIRE(ECS.BlockStorage[0].first != nullptr);
		REQUIRE(ECS.BlockStorage[0].last != nullptr);

		//block stores the correct handle
		REQUIRE(ECS.BlockStorage[0].first->entities[0] == handle);
		//entity stores the correct pointer
		REQUIRE(ECS.Entities[0].block == ECS.BlockStorage[0].first);
		REQUIRE(ECS.ValidateAll());
	}
}

TEST_CASE("Iteration: 1 entity - Queries ")
{
	Archetype PosRotBig;
	PosRotBig.AddComponent<Position>();
	PosRotBig.AddComponent<Rotation>();
	PosRotBig.AddComponent<BigComponent>();

	Archetype PosOnly;
	PosOnly.AddComponent<Position>();

	ComponentList PositionSearch = PosOnly.componentlist;
	ComponentList PosRotBigSearch = PosRotBig.componentlist;
	ComponentList empty;

	ECSWorld ECS;

	int BlocksIterated = 0;
	int EntitiesIterated = 0;
	int HandleMatches = 0;

	SECTION("Match one component - basic") {

		EntityHandle handle = ECS.CreateEntityBatched(PosRotBig, 1)[0];


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
		REQUIRE(BlocksIterated == 1);


	}
	SECTION("Match one component - cant match") {

		EntityHandle handle = ECS.CreateEntityBatched(PosOnly, 1)[0];

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
		REQUIRE(BlocksIterated == 0);
	}
}
TEST_CASE("Iteration: 1 entity - Negative Queries ")
	{
		Archetype PosRotBig;
		PosRotBig.AddComponent<Position>();
		PosRotBig.AddComponent<Rotation>();
		PosRotBig.AddComponent<BigComponent>();

		Archetype PosOnly;
		PosOnly.AddComponent<Position>();

		Archetype RotOnly;
		RotOnly.AddComponent<Rotation>();

		ComponentList PositionSearch = PosOnly.componentlist;
		ComponentList RotationSearch = RotOnly.componentlist;
		ComponentList PosRotBigSearch = PosRotBig.componentlist;
		ComponentList empty;

		ECSWorld ECS;

		int BlocksIterated = 0;
		int EntitiesIterated = 0;
		int HandleMatches = 0;

	SECTION("Match one component - query against itself") {

		EntityHandle handle = ECS.CreateEntityBatched(PosOnly, 1)[0];


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
		REQUIRE(BlocksIterated == 0);
	}

	SECTION("Match one component - negative query on big") {

		EntityHandle handle = ECS.CreateEntityBatched(PosRotBig, 1)[0];


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
		REQUIRE(BlocksIterated == 0);
	}
	SECTION("Match one component - big negaive query") {

		EntityHandle handle = ECS.CreateEntityBatched(RotOnly, 1)[0];


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
		REQUIRE(BlocksIterated == 0);
	}
}