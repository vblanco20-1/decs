//#define _ITERATOR_DEBUG_LEVEL 0
//#define _SECURE_SCL 0
//#define _SECURE_SCL_THROWS 0
//#define _HAS_ITERATOR_DEBUGGING 0
#include <chrono>
#if 0


#include "decs.hpp"

#include <iostream>
#include <chrono>
#include <random>

#include "ExampleComponents.h"
//#define _ITERATOR_DEBUG_LEVEL 0
//#define _SECURE_SCL 0
//#define _SECURE_SCL_THROWS 0
//#define _HAS_ITERATOR_DEBUGGING 0
#include "entt.hpp"

using namespace std;
BaseComponent::IDCounter BaseComponent::family_counter_ = 1;

struct timer final {
	timer() : start{ std::chrono::system_clock::now() } {}

	double elapsed() {
		auto now = std::chrono::system_clock::now();
		double time = std::chrono::nanoseconds(now - start).count() / 1000000.0;
		start = std::chrono::system_clock::now();
		return time;
	}

private:
	std::chrono::time_point<std::chrono::system_clock> start;
};

void Print_Comparaision(double Entt, double Decs) {

	cout << "    Entt took " << Entt << " ms" << endl;
	cout << "    Decs took " << Decs << " ms" << endl;
	cout << "Ratio is : " << Decs / Entt << "x" << endl;
	cout << "---------------------------------------------------" << endl;
}
void Compare_CreationDeletion()
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(100);
	entt::registry<> Entt_ECS;

	Archetype EmptyArchetype;

	std::cout << "Constructing 1.000.000 entities-------------------------------" << std::endl;
	timer tim;
	//create entt entities
	for (std::uint64_t i = 0; i < 1000000L; i++) {
		Entt_ECS.create();
	}
	double Entt_creation = tim.elapsed();

	auto ets = V_ECS.CreateEntityBatched(EmptyArchetype, 1000000L);

	double Decs_creation = tim.elapsed();

	Print_Comparaision(Entt_creation, Decs_creation);


	std::cout << "Deleting 1.000.000 entities-------------------------------" << std::endl;

	tim.elapsed();

	Entt_ECS.each([&](auto e) {
		Entt_ECS.destroy(e);
	});
	Entt_creation = tim.elapsed();

	for (auto e : ets)
	{
		V_ECS.DeleteEntity(e);
	}
	Decs_creation = tim.elapsed();
	Print_Comparaision(Entt_creation, Decs_creation);
}


struct Screamer {

	Screamer() {
	x = 0; 
	y = 1;
	z = 2;
	}
	~Screamer() {
	//	std::cout << "destroy";
	}
	
	float x, y, z;
};


void Compare_ComponentAdd()
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(10000);
	entt::registry<> Entt_ECS;

	Archetype EmptyArchetype;

	std::cout << "Adding component to 1.000.000 entities-------------------------------" << std::endl;

	//create entt entities
	for (std::uint64_t i = 0; i < 1000000L; i++) {
		Entt_ECS.create();
	}


	auto handles = V_ECS.CreateEntityBatched(EmptyArchetype, 1000000L);



	timer tim;


	Entt_ECS.each([&Entt_ECS](auto entity) {
		Entt_ECS.assign<Position>(entity);
	});
	double Entt_creation = tim.elapsed();

	for (auto e : handles)
	{
		V_ECS.AddComponent<Screamer>(e); 
		
	}
	double Decs_creation = tim.elapsed();


	Print_Comparaision(Entt_creation, Decs_creation);
}

void Compare_ComponentRemove() {
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(100);
	entt::registry<> Entt_ECS;


	Archetype PosRot;
	PosRot.add_component<Position>();
	PosRot.add_component<Rotation>();
	//PosRot.AddComponent<BigComponent>();

	std::cout << "Removing component to 1.000.000 entities-------------------------------" << std::endl;

	//create entt entities
	for (std::uint64_t i = 0; i < 1000000L; i++) {
		auto e = Entt_ECS.create();
		Entt_ECS.assign<Position>(e);
		Entt_ECS.assign<Rotation>(e);
		//Entt_ECS.assign<BigComponent>(e);
	}

	auto handles = V_ECS.CreateEntityBatched(PosRot, 1000000L);

	timer tim;


	Entt_ECS.each([&Entt_ECS](auto entity) {
		Entt_ECS.remove<Position>(entity);
	});
	double Entt_creation = tim.elapsed();

	for (auto e : handles)
	{
		V_ECS.RemoveComponent<Position>(e);
	}
	double Decs_creation = tim.elapsed();


	Print_Comparaision(Entt_creation, Decs_creation);
}



void Compare_SimpleIteration()
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(100);
	entt::registry<> Entt_ECS;


	Archetype PosRot;
	PosRot.componentlist = ComponentList::build_component_list<Position,Rotation>();	
	//PosRot.add_component<Position>();
	//PosRot.add_component<Rotation>();
	Archetype empty;
	ComponentList PosOnly = ComponentList::build_component_list<Position>();	
	V_ECS.get_matcher(PosRot.componentlist);
	V_ECS.get_matcher(PosOnly);
	std::cout << "Iterating 1.000.000 entities 1 Component-------------------------------" << std::endl;

	//create entt entities
	for (std::uint64_t i = 0; i < 1000000L; i++) {
		auto e = Entt_ECS.create();
		Entt_ECS.assign<Position>(e);
		Entt_ECS.assign<Rotation>(e);
		//Entt_ECS.assign<BigComponent>(e);
	}

	auto handles = V_ECS.CreateEntityBatched(PosRot, 1000000L);


	std::mt19937 rng2(0);
	std::uniform_int_distribution<int> uniform_dist2(1, 10);

	std::mt19937 rng1(0);
	std::uniform_int_distribution<int> uniform_dist1(1, 10);


	int p = 0;

	Entt_ECS.view<Position>().each([&](auto entity, auto& c) {
		p++;
		c.x = (float)uniform_dist2(rng2);
		c.y = (float)p;
	});
	p = 0;
	V_ECS.IterateBlocks(PosOnly, [&](ArchetypeBlock & block) {

		auto ap = block.get_component_array_mutable<Position>();

		for (int i = 0; i < block.last; i++)
		{
			p++;
			ap.Get(i).x = (float)uniform_dist1(rng1);
			ap.Get(i).y =  (float)p;
		}
	});



	timer tim;

	int compares = 0;
	Entt_ECS.view<Position>().each([&](auto entity, const auto &c) {

		const int x = (int)c.x;
		const int y = (int)c.y;
		compares += x % 20 + y;
	});
	double Entt_creation = tim.elapsed();
	int compares2 = 0;
	V_ECS.IterateBlocks(PosOnly, [&](ArchetypeBlock & block) {

		auto ap = block.get_component_array_mutable<Position>();
		//auto ar = block.get_component_array_mutable<Rotation>();
		for (int i = 0; i < block.last; i++)


		{
			const int x = (int)ap.Get(i).x;
			const int y = (int)ap.Get(i).y;
			//cout << ":" << x << ";";
			compares2 += x % 20 + y;
		}

	});
	double Decs_creation = tim.elapsed();

	cout << "Total Iterations: " << compares << ":" << compares2 << endl;
	Print_Comparaision(Entt_creation, Decs_creation);
}

void Compare_SimpleIteration_5Comps()
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(100);
	entt::registry<> Entt_ECS;


	Archetype All;
	All.componentlist = ComponentList::build_component_list<Position,Rotation,C1,C2,C3>();
	
	Archetype empty;
	Archetype PosOnly;
	PosOnly.add_component<Position>();
	//PosRot.AddComponent<BigComponent>();

	std::cout << "Iterating 1.000.000 entities 5 Component-------------------------------" << std::endl;

	const auto entitycount = 100000L;

	//create entt entities
	for (std::uint64_t i = 0; i < entitycount; i++) {
		auto e = Entt_ECS.create();
		Entt_ECS.assign<Position>(e);
		Entt_ECS.assign<Rotation>(e);
		Entt_ECS.assign<C1>(e);
		Entt_ECS.assign<C2>(e);
		Entt_ECS.assign<C3>(e);
		//Entt_ECS.assign<BigComponent>(e);
	}

	auto handles = V_ECS.CreateEntityBatched(All, entitycount);
	auto view = Entt_ECS.persistent_view<Position, Rotation, C1, C2, C3>();
	//view.initialize();

	timer tim;

	int compares = 0;
	view.each([&](auto entity, auto &c, auto &c1, auto &c2, auto &c3, auto &c4) {
		c.x = c1.y * c2.z;
		compares+= (int)c.x+1;
	});
	double Entt_creation = tim.elapsed();

	int compares2 = 0;
	V_ECS.IterateBlocks(All.componentlist, [&](ArchetypeBlock & block) {
		auto ap = block.get_component_array_mutable<Position>();
		auto ar = block.get_component_array_mutable<Rotation>();
		auto c1 = block.get_component_array_mutable<C1>();

		for (int i = 0; i < block.last; i++)
		{
			ap[i].x = ar[i].y * c1[i].z;
			compares2+= (int)ap[i].x+1;
		}
	});
	double Decs_creation = tim.elapsed();

	cout << "Total Iterations: " << compares << ":" << compares2 << endl;
	Print_Comparaision(Entt_creation, Decs_creation);
}
float DummyFunction(C1 & ca, C2&cb, C3 & cc)
{
	ca.x = cb.x + cc.y;
	cb.y = ca.z / cc.x;
	cc.x = ca.x;
	cc.z = cb.x * cb.y;
	return cc.z;
}

void AddRandomComponentsToEntity(entt::registry<>::entity_type & EnttEntity ,EntityHandle & DecsEntity, ECSWorld& V_ECS,entt::registry<>&Entt_ECS,  std::mt19937 & rng,std::uniform_int_distribution<int>&uniform_dist) {

	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<Position>(EnttEntity);
		V_ECS.AddComponent<Position>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<Rotation>(EnttEntity);
		V_ECS.AddComponent<Rotation>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<C1>(EnttEntity);
		V_ECS.AddComponent<C1>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<C2>(EnttEntity);
		V_ECS.AddComponent<C2>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<C3>(EnttEntity);
		V_ECS.AddComponent<C3>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<comp<1>>(EnttEntity);
		V_ECS.AddComponent<comp<1>>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<comp<2>>(EnttEntity);
		V_ECS.AddComponent<comp<2>>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<comp<3>>(EnttEntity);
		V_ECS.AddComponent<comp<3>>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<comp<4>>(EnttEntity);
		V_ECS.AddComponent<comp<4>>(DecsEntity);
	}
	if (uniform_dist(rng) < 4)
	{
		Entt_ECS.assign<comp<5>>(EnttEntity);
		V_ECS.AddComponent<comp<5>>(DecsEntity);
	}
}

#define MATCH_5 1

void Compare_Iteration_Pathological(uint64_t numEntities, bool bShuffle = false )
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(100000);
	entt::registry<> Entt_ECS;

	int MatchNumber = 3;

	Archetype All;
	All.componentlist = ComponentList::build_component_list<Position,Rotation,C1,C2,C3>();
	Archetype empty;
	Archetype PosOnly;
	PosOnly.add_component<Position>();
	//PosRot.AddComponent<BigComponent>();
	Archetype Cs;
	Cs.add_component<C1>();
	Cs.add_component<C2>();
	Cs.add_component<C3>();

#if MATCH_5
	Cs.add_component<comp<1>>();
	Cs.add_component<comp<2>>();
	MatchNumber = 5;
#endif 

	std::cout << "Iterating "<< (bShuffle ? " shuffled " : " ordered ")  << "entities: pathological random case 10 Component find "<<MatchNumber <<": Num Entities : " << numEntities <<"  -------------------------------" << std::endl;

	std::mt19937 rng(0);
	std::uniform_int_distribution<int> uniform_dist(1, 10);

	std::vector<entt::registry<>::entity_type > EnttEntities;
	std::vector<EntityHandle> DecsEntities;

	EnttEntities.reserve(numEntities);
	DecsEntities.reserve(numEntities);
	//create entt entities
	for (std::uint64_t i = 0; i < numEntities; i++) {
		auto e1 = Entt_ECS.create();
		auto e2 = V_ECS.CreateEntity(empty);
		EnttEntities.push_back(e1);
		DecsEntities.push_back(e2);


		AddRandomComponentsToEntity(e1,e2,V_ECS,Entt_ECS,rng,uniform_dist);
	}
	//delete half the entities randomly

	if (bShuffle)
	{
		for (int i = 0; i < numEntities; i++)
		{
			if (uniform_dist(rng) < 5)
			{
				Entt_ECS.destroy(EnttEntities[i]);
				V_ECS.destroy(DecsEntities[i]);

				auto e1 = Entt_ECS.create();
				auto e2 = V_ECS.CreateEntity(empty);
				AddRandomComponentsToEntity(e1, e2, V_ECS, Entt_ECS, rng, uniform_dist);
			}
		}
	}



	auto mt = V_ECS.get_matcher(Cs.componentlist);
	//auto handles = V_ECS.CreateEntityBatched(All, 1000000L);
	auto view = Entt_ECS.persistent_view<C1, C2, C3
#if MATCH_5
		,comp<1>,comp<2>
#endif
	
	>();
	//view.initialize();

	timer tim;

	long long compares = 0;
	

	int nIterationsEnTT = 0;
	view.each([&](auto entity, auto &c2, auto &c3, auto &c4
#if MATCH_5
		,auto &c5, auto &c6
#endif
		)
														  
	{// {
		compares+=(int)DummyFunction(c2,c3,c4);
		nIterationsEnTT++;
	});
	double Entt_creation = tim.elapsed();
	long long  compares2 = 0;
	int nIterationsDecs = 0;
	for (int i = 0; i < 1; i++)
	{
	V_ECS.IterateBlocks(Cs.componentlist, [&](ArchetypeBlock & block) {
		auto ap = block.get_component_array_mutable<C1>();
		auto ar = block.get_component_array_mutable<C2>();
		auto c1 = block.get_component_array_mutable<C3>();

		for (int i = 0; i < block.last; i++)
		{
			nIterationsDecs++;
			compares2+=(int)DummyFunction(ap[i],ar[i],c1[i]);
			
			
		}
	});
	}
	double Decs_creation = tim.elapsed();

	int compares3 = 0;
	V_ECS.IterateBlocks(Cs.componentlist, [&](ArchetypeBlock & block) {
		auto ap = block.get_component_array_mutable<C1>();
		auto ar = block.get_component_array_mutable<C2>();
		auto c1 = block.get_component_array_mutable<C3>();

		for (int i = 0; i < block.last; i++)
		{
			//compares3++;
			compares3+=(int)DummyFunction(ap.Get(i),ar.Get(i),c1.Get(i));
		}
	}, true);
	tim.elapsed();
	compares3 = 0;
	V_ECS.IterateBlocks(Cs.componentlist, [&](ArchetypeBlock & block) {

		auto ap = block.get_component_array_mutable<C1>();
		auto ar = block.get_component_array_mutable<C2>();
		auto c1 = block.get_component_array_mutable<C3>();

		for (int i = 0; i < block.last; i++)
		{
			
			compares3+=(int)DummyFunction(ap.Get(i),ar.Get(i),c1.Get(i));
		}
	}, true);

	double Decs_parallel = tim.elapsed();

	cout << "Total Iterations: " << nIterationsEnTT << ":" << nIterationsDecs << endl;
	cout << "Dummy Numbers: " << compares << ":" << compares2 << endl;
	cout << "    Decs Parallel: " << Decs_parallel << "ms" << endl;
	Print_Comparaision(Entt_creation, Decs_creation);
}

int main()
{
	
		//Compare_Creation();

		//cout << "===========Comparing Decs against Entt ================" << endl << endl;
	//for (int i = 0; i < 1; i++)
	//{
		Compare_CreationDeletion();
		//Compare_ComponentAdd();
		//Compare_ComponentRemove();
	//}
		Compare_SimpleIteration();
		Compare_SimpleIteration_5Comps();
	for (int j = 1; j < 10; j++) {

	
		for (int i = 1; i < 3; i++)
		{
			Compare_Iteration_Pathological(j*100000, false);
		}
		for (int i = 1; i < 3; i++)
		{
			Compare_Iteration_Pathological(j*100000, true);
		}
	}
	
	char a;
	cin >> a;
}
#endif

#include <iostream>
#include <entt.hpp>
#include <K:\Programming\decs\decs\decs2.h>

#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch.hpp"

struct I1 {
	int val;
};
struct I2 {
	int val[2];
};
struct I3 {
	int val[3];
};
template<int N>
struct Comp128 {
	size_t a;
	size_t b;

	Comp128() {
		a = N;
		b = N * 2;
	}
};

struct alignas(32) IAlign {
	char val;
};

struct IConstr {
	int val{ 42 };
};
using namespace decs2;
void pathological_build(ECSWorld* world, int amount);
void pathological_build_entt(entt::registry& world, int amount);
const Metatype *I1type = get_metatype<I1>();

const Metatype *I2type = get_metatype<I2>();

const Metatype *I3type = get_metatype<I3>();

const Metatype *IAligntype = get_metatype<IAlign>();

const Metatype* IContype = get_metatype<IConstr>();

TEST_CASE("basic ecs features", "[bit-tree,!benchmark]") {

	ECSWorld world{};

	const Metatype* types[] = { I1type,I2type,I3type,IAligntype };

	Archetype* arch1 = find_or_create_archetype(&world, types, 1);
	Archetype* arch2 = find_or_create_archetype(&world, types, 2);
	Archetype* arch3 = find_or_create_archetype(&world, types, 3);	


	int nIterations = 1000;
	

	BENCHMARK_ADVANCED("std: create 1.000 sanitycheck")(Catch::Benchmark::Chronometer meter) {

		std::vector<uint64_t> test;

		meter.measure([&]() {
			for (int i = 0; i < nIterations; i++) {
				test.push_back(i);
			}
		});
	};
	BENCHMARK_ADVANCED("Decs: create 1.000 entities naked")(Catch::Benchmark::Chronometer meter) {
		std::vector<EntityID> idpool1K;
		idpool1K.reserve(nIterations);

		meter.measure([&]() {
			for (int i = 0; i < nIterations; i++) {
				idpool1K.push_back(
				allocate_entity(&world));
			}
		});			
	};
	BENCHMARK_ADVANCED("Entt: create 1.000 entities naked")(Catch::Benchmark::Chronometer meter) {
		
		std::vector<entt::registry*> registries;
		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(new entt::registry());
			registries[i]->create();
		}

		meter.measure([&](int num) {
			for (int i = 0; i < nIterations; i++) {
				registries[num]->create();
			}
			return registries[num]->alive();
		});
	};

	BENCHMARK_ADVANCED("Decs: create 1.000 entities with comps")(Catch::Benchmark::Chronometer meter) {
		std::vector<ECSWorld*> registries;
		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(new ECSWorld{});
		}

		meter.measure([&](int num) {
			auto test_reg = registries[num];
			for (int i = 0; i < nIterations; i++) {

				create_entity_with_archetype(arch3);
			}
			return registries[num]->live_entities;
			});
	};
	BENCHMARK_ADVANCED("Entt: create 1.000 entities with comps")(Catch::Benchmark::Chronometer meter) {

		std::vector<entt::registry*> registries;
		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(new entt::registry());
			registries[i]->create();
		}

		meter.measure([&](int num) {
			for (int i = 0; i < nIterations; i++) {
				auto id = registries[num]->create();
				registries[num]->assign<I1>(id);
				registries[num]->assign<I2>(id);
				registries[num]->assign<I3>(id);
			}
			return registries[num]->alive();
		});
	};

	nIterations = 100000;


	BENCHMARK_ADVANCED("std: create 100.000 sanitycheck")(Catch::Benchmark::Chronometer meter) {

		std::vector<std::vector<uint64_t>> registries;


		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(std::vector<uint64_t>{});
		}

		meter.measure([&](int num) {
			for (int i = 0; i < nIterations; i++) {
				registries[num].push_back(i);
			}
			return registries[num].size();
		});
	};
	BENCHMARK_ADVANCED("Decs: create 100.000 entities naked")(Catch::Benchmark::Chronometer meter) {
		
		std::vector<ECSWorld*> registries;
		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(new ECSWorld{});
			registries[i]->entities.reserve(100000);
			
		}

		meter.measure([&](int num) {
			auto test_reg = registries[num];
			for (int i = 0; i < nIterations; i++) {
				
				allocate_entity(test_reg);
			}
			return registries[num]->live_entities;
		});
	};
	BENCHMARK_ADVANCED("Entt: create 100.000 entities naked")(Catch::Benchmark::Chronometer meter) {

		std::vector<entt::registry*> registries;
		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(new entt::registry());
			registries[i]->create();
		}

		meter.measure([&](int num) {
			for (int i = 0; i < nIterations; i++) {
				registries[num]->create();
			}
			return registries[num]->alive();
		});
	};

	BENCHMARK_ADVANCED("Decs: create 100.000 entities with comps")(Catch::Benchmark::Chronometer meter) {

		std::vector<ECSWorld*> registries;
		std::vector<Archetype*> archetypes;
		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(new ECSWorld{});
			registries[i]->entities.reserve(100000);
			
			archetypes.push_back(find_or_create_archetype(registries[i], types, 3));
		}

		meter.measure([&](int num) {
			auto test_reg = registries[num];
			for (int i = 0; i < nIterations; i++) {

				create_entity_with_archetype(archetypes[num]);
			}
			return archetypes[num]->chunks.size();
		});
	};
	BENCHMARK_ADVANCED("Entt: create 100.000 entities with comps")(Catch::Benchmark::Chronometer meter) {

		std::vector<entt::registry*> registries;
		for (int i = 0; i < meter.runs(); i++) {
			registries.push_back(new entt::registry());
			registries[i]->create();
		}

		meter.measure([&](int num) {
			for (int i = 0; i < nIterations; i++) {
				auto id = registries[num]->create();
				registries[num]->assign<I1>(id);
				registries[num]->assign<I2>(id);
				registries[num]->assign<I3>(id);
			}
			return registries[num]->alive();
		});
	};
	nIterations = 100000;
	long int global_count = 0;
	long int iterationcount = 0;
	BENCHMARK_ADVANCED("Decs: basic query, 100k entities 1-3 comps")(Catch::Benchmark::Chronometer meter) {

		ECSWorld tstworld;
		Archetype* tstarch = find_or_create_archetype(&tstworld, types, 3);
		
		for (int i = 0; i < nIterations; i++) {
			
			auto eid = create_entity_with_archetype(tstarch);
			get_component<I1>(&tstworld, eid).val = i % 32;
		}		

		meter.measure([&](int num) {
			long int count = 0;

			iterate_entities<I1, I2>(&tstworld, [&](EntityID id, I1& c, I2&c2) {
				count += c.val;
			});

			if (num == 0) {
				global_count = count;
			}
			
			return count;
		});
	};
	long int accum = 0;
	BENCHMARK_ADVANCED("Entt:  basic query, 100k entities 1-3 comps")(Catch::Benchmark::Chronometer meter) {
		
		entt::registry reg;
		for (int i = 0; i < nIterations; i++) {
			
			auto et = reg.create();
			reg.assign<I1>(et, i % 32);
		}
		
		meter.measure([&](int num) {
			
			long int count = 0;
			reg.view<I1>().each( [&](auto eid,I1& cp) {
				count += cp.val;
			});			

			if (num == 0) {
				accum = count;
			}
			
			return count;
		});		
	};

	BENCHMARK("Decs:  pathological create 1000k"){//(Catch::Benchmark::Chronometer meter) {

		ECSWorld worldt;
		
		//meter.measure([&]() {

			
			pathological_build(&worldt, 1000000);
			int count = 0;;
			iterate_entities<Comp128<2>>(&worldt, [&](EntityID id, Comp128<2>& c) {
				count += c.a;
				});

			return worldt.archetypes.size() + count;
		//});
	};

	BENCHMARK_ADVANCED("Decs:  pathological iterate 1000k") (Catch::Benchmark::Chronometer meter) {

		ECSWorld worldt;
		pathological_build(&worldt, 1000000);
		meter.measure([&](int idx) {


			
			int count = 0;;
			iterate_entities<Comp128<4>, Comp128<5>>(&worldt, [&](EntityID id, auto& c, auto& cb) {
				count += (c.a + cb.b) * idx;
				});

			return worldt.archetypes.size() + count;
			});
	};

	BENCHMARK_ADVANCED("STL:  add 2 comps iterate 1000k :sanity check") (Catch::Benchmark::Chronometer meter) {

		std::vector<Comp128<4>> comp4;
		std::vector<Comp128<5>> comp5;
		comp4.reserve(1000000);
		comp5.reserve(1000000);
		

		std::mt19937 rng(0);
		std::uniform_int_distribution<int> uniform_dist(0, 1000);

		auto rngcheck = [&]() {return uniform_dist(rng) < 5; };

		for (int i = 0; i < 1000000/4; i++) {
			Comp128<4> a;
			a.a = uniform_dist(rng);
			a.b = uniform_dist(rng);
			comp4.push_back(a);

			Comp128<5> b;
			b.a = uniform_dist(rng);
			b.b = uniform_dist(rng);
			comp5.push_back(b);
		}
			
		meter.measure([&](int idx) {


			int count = 0;
			for (int i = 0; i < 1000000 / 4; i++) {
				
				count += (comp4[i].a + comp5[i].b)*idx;
			}

			return  count;
		});
	};


	BENCHMARK("Entt:  pathological create 1000k"){//(Catch::Benchmark::Chronometer meter) {

		entt::registry reg;

		//meter.measure([&]() {

			pathological_build_entt(reg, 1000000);

			int count = 0;;
			reg.view<Comp128<2>>().each([&](auto eid, auto& c) {
				count += c.a;
				});

			return  count;
		//});
	};
	BENCHMARK_ADVANCED("Entt:  pathological iterate 1000k") (Catch::Benchmark::Chronometer meter) {

		entt::registry reg;

		//meter.measure([&]() {
		auto grp = reg.group<Comp128<4>, Comp128<5> >();
		pathological_build_entt(reg, 1000000);
		meter.measure([&](int idx) {
			int count = 0;;
			grp.each([&](auto eid, auto& c, auto& cb) {
				count += c.a * idx + cb.a;
			});

			return count;			
		});
	};


	CHECK(accum == global_count);
};


void pathological_build(ECSWorld* world, int amount) {
	std::mt19937 rng(0);
	std::uniform_int_distribution<int> uniform_dist(0, 10);

	auto rngcheck = [&]() {return uniform_dist(rng) < 5; };

	//ECSWorld *world = new ECSWorld();
	const Metatype* types[] = { get_metatype<Comp128<1>>() };
	Archetype* arc1 = find_or_create_archetype(world, types, 1);

	std::vector<EntityID> ids;
	ids.reserve(amount);
	for (int i = 0; i < amount; i++) {
		EntityID id = create_entity_with_archetype(arc1);
		ids.push_back(id);
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<2>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<3>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<4>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<5>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<6>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<7>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<8>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<9>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<10>{});
		}
		if (rngcheck()) {
			add_component_to_entity(world, id, Comp128<11>{});
		}
	}
}
void pathological_build_entt(entt::registry& world, int amount) {

	std::mt19937 rng(0);
	std::uniform_int_distribution<int> uniform_dist(0, 10);

	auto rngcheck = [&]() {return uniform_dist(rng) < 5; };


	std::vector<entt::entity> ids;
	ids.reserve(amount);

	world.reserve(amount);
	for (int i = 0; i < amount; i++) {
		entt::entity id = world.create();
		ids.push_back(id);
		if (rngcheck()) {
			world.assign<Comp128<2>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<3>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<4>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<5>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<6>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<7>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<8>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<9>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<10>>(id);
		}
		if (rngcheck()) {
			world.assign<Comp128<11>>(id);
		}
	}
}
int add_comps(Comp128<4>&ca, Comp128<5>&cb) {
	return (ca.a + cb.b);
}
int main(int argc, char* argv[])
{
	Catch::Session session; // There must be exactly one instance

	
#if 0
	for (int i = 0; i < 1000; i++) {
		ECSWorld benchworld;
		{
			auto start = std::chrono::system_clock::now();



			pathological_build(&benchworld, 1000000);


			auto end = std::chrono::system_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

			std::cout << benchworld.archetypes.size() << std::endl;
			std::cout << "Decs:" << (double(elapsed.count())) / 1000.f << "ms" << '\n';



		}
	
		{
			//for (int j = 0; j < 1000; j++) {
			//
			//	auto start = std::chrono::system_clock::now();
			//
			//	int count = 0;;
			//	iterate_entities<Comp128<4>, Comp128<5>>(&benchworld,[&](EntityID id, auto& c, auto& cb) {
			//		count += add_comps(c,cb)+j;
			//		});
			//
			//	auto end = std::chrono::system_clock::now();
			//	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			//
			//	std::cout << benchworld.archetypes.size() << std::endl;
			//	std::cout << "Decs-iter:" << (double(elapsed.count())) / 1000.f << "ms" << count << '\n';
			//}
		}

		//{
		//	auto start = std::chrono::system_clock::now();
		//
		//
		//	entt::registry benchworld;
		//	auto grp = benchworld.group < Comp128<2>, Comp128<3>, Comp128<4>, Comp128<5>
		//		, Comp128<6>, Comp128<7>, Comp128<8>>();
		//	pathological_build_entt(benchworld, 1000000);
		//
		//
		//	auto end = std::chrono::system_clock::now();
		//	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		//
		//	std::cout << grp.size() << std::endl;
		//	std::cout << "Entt:" << (double(elapsed.count())) / 1000.f << "ms" << '\n';
		//}


	}
	return 0;
#else
	
	
	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0) // Indicates a command line error
		return returnCode;


	int numFailed = session.run();
	return numFailed;
#endif
}