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
	ECS.Blocks.reserve(20);

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
	ECS.Blocks.reserve(20);

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