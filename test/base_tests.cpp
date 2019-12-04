#include <iostream>
#include <cstddef>
#include <cstdint>
#include <chrono>
#include <iterator>
#include <gtest/gtest.h>
#include <decs.h>
#include <random>
struct TestComp { int i; };
struct TestCompB { double f; };
TEST(Basic,EntityCreationDeletion)
{
	using namespace decs;
	{
		ECSWorld world{};

		std::vector<EntityID> entities;

		for (int i = 0; i < 1000; i++) {
			entities.push_back(world.new_entity());
		}

		EXPECT_EQ(world.entities.size() , 1000);
		EXPECT_EQ(world.live_entities , 1000);

		for (auto eid : entities) {
			world.destroy(eid);
		}

		EXPECT_EQ(world.entities.size() , 1000);
		EXPECT_EQ(world.live_entities, 0);

		for (int i = 0; i < 1000; i++) {
			world.new_entity();
		}
		EXPECT_EQ(world.entities.size(), 1000);
		EXPECT_EQ(world.live_entities ,1000);

		entities.clear();

		for (int i = 0; i < 1000; i++) {
			entities.push_back(world.new_entity<TestComp>());
		}
		EXPECT_EQ(world.live_entities, 2000);
		std::random_device rd;
		std::mt19937 g(rd());
		std::shuffle(entities.begin(), entities.end(), g);

		for (auto eid : entities) {
			world.destroy(eid);
		}
		EXPECT_EQ(world.live_entities, 1000);
	}	
}

TEST(Basic, NullEntities)
{
	using namespace decs;
	{
		ECSWorld world{};

		std::vector<EntityID> entities;

		for (int i = 0; i < 1000; i++) {
			entities.push_back(world.new_entity());
		}
		EXPECT_EQ(world.entities.size(), 1000);
		EXPECT_EQ(world.live_entities, 1000);

		int count = 0;
		for (auto chnk : world.archetypes[0]->chunks) {
			count += chnk->header.last;
		}
		EXPECT_EQ(count, 1000);
	}
}


TEST(Basic, Components)
{
	using namespace decs;
	{
		ECSWorld world{};

		std::vector<EntityID> entities;

		
		auto et = world.new_entity();
		
		world.add_component<TestComp>(et, TestComp{1});

		EXPECT_EQ(world.get_component<TestComp>(et).i, TestComp{ 1 }.i);
	
		world.add_component<TestCompB>(et, TestCompB{});

		EXPECT_EQ(world.get_component<TestComp>(et).i, TestComp{ 1 }.i);
		EXPECT_TRUE(world.has_component<TestComp>(et));

		world.remove_component<TestCompB>(et);

		EXPECT_EQ(world.get_component<TestComp>(et).i, TestComp{ 1 }.i);
		EXPECT_FALSE(world.has_component<TestCompB>(et));
	}
}


TEST(Basic, Iteration) {
	using namespace decs;
	{
		ECSWorld world{};

		std::vector<EntityID> entities;

		if constexpr (true)
		{

			EXPECT_EQ(2, 2);
		}

		auto e1 = world.new_entity<TestComp>();
		auto e2 = world.new_entity<TestComp,TestCompB>();

		int count = 0;
		world.for_each([&](decs::EntityID id, TestComp& comp) {
			
			count++;
			});

		EXPECT_EQ(count, 2);

		world.destroy(e2);
		
		count = 0; 
		world.for_each([&](EntityID id,const TestComp& comp) {
			count++;
		});
		
		EXPECT_EQ(count, 1);
		
		world.destroy(e1);
		
		count = 0;
		world.for_each([&](EntityID id, TestComp& comp, TestCompB&comp2) {
			count++;
		});
		EXPECT_EQ(count, 0);
	}
}