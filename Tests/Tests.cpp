#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

struct I1 {
	int val;
};
struct I2 {
	int val[2];
};
struct I3 {
	int val[3];
};

struct alignas(32) IAlign {
	char val;
};

struct IConstr {
	int val{ 42 };
};
using namespace decs2;

const Metatype* I1type = get_metatype<I1>();

const Metatype* I2type = get_metatype<I2>();

const Metatype* I3type = get_metatype<I3>();

const Metatype* IAligntype = get_metatype<IAlign>();

const Metatype* IContype = get_metatype<IConstr>();

namespace Tests
{
	TEST_CLASS(CLists)
	{
	public:
		
		TEST_METHOD(BasicChunkOperations)
		{
			const Metatype* types[] = { I1type,I2type,I3type,IAligntype };

			ChunkComponentList* list1 = build_component_list(types,1);
			Assert::AreEqual(list1 != nullptr, true);
			ChunkComponentList* list3 = build_component_list(types, 4);
			Assert::AreEqual(list3 != nullptr, true);

			DataChunk* chunk = build_chunk(list3);
			Assert::AreEqual(chunk != nullptr, true);

			auto arrayA = get_chunk_array<I1>(chunk);
			auto arrayA_cp = get_chunk_array<I1>(chunk);
			auto arrayB = get_chunk_array<I2>(chunk);
			auto arrayC = get_chunk_array<I3>(chunk);
			auto arrayD = get_chunk_array<IAlign>(chunk);

			
			//alignements

			Assert::AreEqual((size_t)arrayA.data % alignof(I1), (size_t)0);
			Assert::AreEqual((size_t)arrayB.data % alignof(I2), (size_t)0);
			Assert::AreEqual((size_t)arrayC.data % alignof(I3), (size_t)0);
			Assert::AreEqual((size_t)arrayD.data % alignof(IAlign), (size_t)0);
			

			int capacity = list3->chunkCapacity;

			for (int i = 0; i < capacity; i++) {
				arrayA[i] = I1{1};
			}
			for (int i = 0; i < capacity; i++) {
				arrayB[i] = I2{ 2,2 };
			}
			for (int i = 0; i < capacity; i++) {
				arrayC[i] = I3{ 3,3,3 };
			}

			for (int i = 0; i < capacity; i++) {
				arrayD[i] = IAlign{ char(i % 64)};
			}
			
			int count1 = 0;
			for (int i = 0; i < capacity; i++) {
				count1 += get_chunk_array<I1>(chunk)[i].val;
			}
			Assert::AreEqual(count1, capacity);

			bool bValid2 = true;
			for (int i = 0; i < capacity; i++) {
				if (arrayB[i].val[0] != 2 || arrayB[i].val[1] != 2) {
					bValid2 = false;
					break;
				}
			}
			Assert::AreEqual(bValid2, true);

			bool bValid3 = true;
			//for (int i = 0; i < capacity; i++) {
			for(auto vl : arrayC){
				if (vl.val[0] != 3 || vl.val[1] != 3 || vl.val[2] != 3) {
					bValid3 = false;
					break;
				}
			}

			Assert::AreEqual(bValid3, true);
			bool bValid4 = true;
			for (int i = 0; i < capacity; i++) {
				if (get_chunk_array<IAlign>(chunk)[i].val != char(i % 64)) {
					bValid2 = false;
					break;
				}
			}

			Assert::AreEqual(bValid4, true);
		}

		TEST_METHOD(ChunkAddRemove)
		{
			const Metatype* types[] = { I1type,I2type,I3type,IAligntype };

			ChunkComponentList* list3 = build_component_list(types, 4);
			Assert::AreEqual(list3 != nullptr, true);

			DataChunk* chunk = build_chunk(list3);
			
			for (int i = 0; i < list3->chunkCapacity * 2;i++) {
				EntityID testID;
				testID.generation = 1;
				testID.index = i;

				insert_entity_in_chunk(chunk, testID);
			}

			auto arrayA = get_chunk_array<IConstr>(chunk);
			auto arrayET = get_chunk_array<EntityID>(chunk);

			Assert::AreEqual(arrayA.chunkOwner != nullptr, true);
			Assert::AreEqual(arrayET.chunkOwner != nullptr, true);
			bool bValid1 = true;
			for (int i = 0; i < list3->chunkCapacity; i++) {
				if (arrayA[i].val != 42 || arrayET[i].index != i) {
					bValid1 = false;
					break;
				}
			}

			Assert::AreEqual(bValid1, true);

			bool bValid2 = true;

			//repeatedly delete entities
			for (int i = 0; i < (list3->chunkCapacity-1); i++) {
			
				EntityID eid = erase_entity_in_chunk(chunk, 0);
				if (eid.generation != 1) {
					bValid2 = false;
				}
			}
			Assert::AreEqual(bValid2, true);


			//last remaining entity should be number 1			
			Assert::AreEqual(arrayET[0].index == 1, true);
			Assert::AreEqual(chunk->header.last == 1, true);
			//delete last
			EntityID del = erase_entity_in_chunk(chunk, 0);
			Assert::AreEqual(chunk->header.last == 0 && del.generation == 0, true);
		}

		TEST_METHOD(Archetypes)
		{
			ECSWorld world{};

			const Metatype* types[] = { I1type,I2type,I3type,IAligntype };

			Archetype* arch1 = find_or_create_archetype(&world, types, 1);
			Archetype* arch2 = find_or_create_archetype(&world, types, 2);
			Archetype* arch3 = find_or_create_archetype(&world, types, 3);
			Archetype* arch1b = find_or_create_archetype(&world, types, 1);

			Assert::AreEqual(arch1 == arch1b, true);
			Assert::AreEqual(arch2 != arch3, true);
			Assert::AreEqual(arch1 != arch3, true);

			Assert::AreEqual(world.archetypes.size(), (size_t)3);
		
			int test1 = 0;

			iterate_matching_archetypes(&world, types, 1, [&](Archetype*arch) {
				test1++;
			});


			Assert::AreEqual(test1 == 3, true);

			int test2 = 0;
			iterate_matching_archetypes(&world, types+2, 1, [&](Archetype* arch) {
				test2++;
				});


			Assert::AreEqual(test2== 1, true);

			int test3 = 0;
			iterate_matching_archetypes(&world, types + 3, 1, [&](Archetype* arch) {
				test3++;
				});


			Assert::AreEqual(test3 == 0, true);
		}


		TEST_METHOD(Entities)
		{
			ECSWorld world{};
			
			std::vector<EntityID> entities;

			for (int i = 0; i < 1000; i++) {
				entities.push_back(allocate_entity(&world));
			}

			Assert::AreEqual(world.entities.size() == 1000, true);
			Assert::AreEqual(world.live_entities == 1000, true);
			for (auto eid : entities) {
				deallocate_entity(&world,eid);
			}

			Assert::AreEqual(world.entities.size() == 1000, true);
			Assert::AreEqual(world.live_entities == 0, true);

			for (int i = 0; i < 1000; i++) {
				allocate_entity(&world);
			}
			Assert::AreEqual(world.entities.size() == 1000, true);
			Assert::AreEqual(world.live_entities == 1000, true);
		}

		TEST_METHOD(EntityComponentAddRemove) {

			ECSWorld world;
			const Metatype* types[] = { I1type,I2type,I3type,IAligntype };
			Archetype* arch1 = find_or_create_archetype(&world, types, 1);

			auto et = create_entity_with_archetype(arch1);

			Assert::AreEqual(has_component<I1>(&world,et), true);
			Assert::AreEqual(has_component<I2>(&world, et), false);

			add_component_to_entity<I2>(&world, et,I2{});

			Assert::AreEqual(has_component<I2>(&world, et), true);

			remove_component_from_entity<I2>(&world, et);
			Assert::AreEqual(has_component<I2>(&world, et), false);
		}
	};
}
