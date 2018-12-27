#include "decs.hpp"
#include "entt.hpp"
#include <iostream>
#include <chrono>
#include <random>

#include "ExampleComponents.h"

using namespace std;
BaseComponent::IDCounter BaseComponent::family_counter_ = 1;

struct timer final {
	timer() : start{ std::chrono::system_clock::now() } {}

	double elapsed() {
		auto now = std::chrono::system_clock::now();
		double time = std::chrono::duration<double>(now - start).count();
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
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();
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
	PosRot.AddComponent<Position>();
	PosRot.AddComponent<Rotation>();
	Archetype empty;
	Archetype PosOnly;
	PosOnly.AddComponent<Position>();
	//PosRot.AddComponent<BigComponent>();

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
		c.x = uniform_dist2(rng2);
		c.y = p;
	});
	p = 0;
	V_ECS.IterateBlocks(PosOnly.componentlist, empty.componentlist, [&](ArchetypeBlock & block) {

		auto ap = block.GetComponentArray<Position>();

		for (int i = 0; i < block.last; i++)
		{
			p++;
			ap.Get(i).x = uniform_dist1(rng1);
			ap.Get(i).y = p;
		}
	});



	timer tim;

	int compares = 0;
	Entt_ECS.view<Position>().each([&](auto entity, const auto &c) {

		const int x = c.x;
		const int y = c.y;
		compares += x % 20 + y;
	});
	double Entt_creation = tim.elapsed();
	int compares2 = 0;
	V_ECS.IterateBlocks(PosOnly.componentlist, empty.componentlist, [&](ArchetypeBlock & block) {

		auto ap = block.GetComponentArray<Position>();
		//auto ar = block.GetComponentArray<Rotation>();
		for (int i = 0; i < block.last; i++)


		{
			const int x = ap.Get(i).x;
			const int y = ap.Get(i).y;
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
	All.AddComponent<Position>();
	All.AddComponent<Rotation>();
	All.AddComponent<C1>();
	All.AddComponent<C2>();
	All.AddComponent<C2>();
	Archetype empty;
	Archetype PosOnly;
	PosOnly.AddComponent<Position>();
	//PosRot.AddComponent<BigComponent>();

	std::cout << "Iterating 1.000.000 entities 5 Component-------------------------------" << std::endl;

	//create entt entities
	for (std::uint64_t i = 0; i < 1000000L; i++) {
		auto e = Entt_ECS.create();
		Entt_ECS.assign<Position>(e);
		Entt_ECS.assign<Rotation>(e);
		Entt_ECS.assign<C1>(e);
		Entt_ECS.assign<C2>(e);
		Entt_ECS.assign<C3>(e);
		//Entt_ECS.assign<BigComponent>(e);
	}

	auto handles = V_ECS.CreateEntityBatched(All, 1000000L);
	auto view = Entt_ECS.persistent_view<Position, Rotation, C1, C2, C3>();
	//view.initialize();

	timer tim;

	int compares = 0;
	view.each([&](auto entity, auto &c, auto &c1, auto &c2, auto &c3, auto &c4) {
		c.x = c1.y * c2.z;
		compares+= c.x;
	});
	double Entt_creation = tim.elapsed();

	int compares2 = 0;
	V_ECS.IterateBlocks(All.componentlist, empty.componentlist, [&](ArchetypeBlock & block) {
		auto ap = block.GetComponentArray<Position>();
		auto ar = block.GetComponentArray<Rotation>();
		auto c1 = block.GetComponentArray<C1>();

		for (int i = 0; i < block.last; i++)
		{
			ap.Get(i).x = ar.Get(i).y * c1.Get(i).z;
			compares2+= ap.Get(i).x;
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
void Compare_Iteration_Pathological(uint64_t numEntities )
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(100000);
	entt::registry<> Entt_ECS;


	Archetype All;
	All.AddComponent<Position>();
	All.AddComponent<Rotation>();
	All.AddComponent<C1>();
	All.AddComponent<C2>();
	All.AddComponent<C2>();
	Archetype empty;
	Archetype PosOnly;
	PosOnly.AddComponent<Position>();
	//PosRot.AddComponent<BigComponent>();
	Archetype Cs;
	Cs.AddComponent<C1>();
	Cs.AddComponent<C2>();
	Cs.AddComponent<C3>();
	//Cs.AddComponent<comp<1>>();
	//Cs.AddComponent<comp<2>>();

	std::cout << "Iterating entities: pathological random case 10 Component find 5: Num Entities : " << numEntities <<"  -------------------------------" << std::endl;

	std::mt19937 rng(0);
	std::uniform_int_distribution<int> uniform_dist(1, 10);
	//create entt entities
	for (std::uint64_t i = 0; i < numEntities; i++) {
		auto e1 = Entt_ECS.create();
		auto e2 = V_ECS.CreateEntity(empty);

		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<Position>(e1);
			V_ECS.AddComponent<Position>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<Rotation>(e1);
			V_ECS.AddComponent<Rotation>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<C1>(e1);
			V_ECS.AddComponent<C1>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<C2>(e1);
			V_ECS.AddComponent<C2>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<C3>(e1);
			V_ECS.AddComponent<C3>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<1>>(e1);
			V_ECS.AddComponent<comp<1>>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<2>>(e1);
			V_ECS.AddComponent<comp<2>>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<3>>(e1);
			V_ECS.AddComponent<comp<3>>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<4>>(e1);
			V_ECS.AddComponent<comp<4>>(e2);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<5>>(e1);
			V_ECS.AddComponent<comp<5>>(e2);
		}
	}

	//auto handles = V_ECS.CreateEntityBatched(All, 1000000L);
	auto view = Entt_ECS.persistent_view<C1, C2, C3/*,,comp<1>,comp<2>*/>();
	//view.initialize();

	timer tim;

	long long compares = 0;
	

	int nIterationsEnTT = 0;
	view.each([&](auto entity, auto &c2, auto &c3, auto &c4)
	{//,auto &c5, auto &c6) {
		compares+=DummyFunction(c2,c3,c4);
		nIterationsEnTT++;
	});
	double Entt_creation = tim.elapsed();
	long long  compares2 = 0;
	int nIterationsDecs = 0;
	for (int i = 0; i < 1; i++)
	{
	V_ECS.IterateBlocks(Cs.componentlist, [&](ArchetypeBlock & block) {
		auto ap = block.GetComponentArray<C1>();
		auto ar = block.GetComponentArray<C2>();
		auto c1 = block.GetComponentArray<C3>();

		for (int i = 0; i < block.last; i++)
		{
			nIterationsDecs++;
			compares2+=DummyFunction(ap.Get(i),ar.Get(i),c1.Get(i));
			
			
		}
	});
	}
	double Decs_creation = tim.elapsed();

	int compares3 = 0;
	V_ECS.IterateBlocks(Cs.componentlist, [&](ArchetypeBlock & block) {
		auto ap = block.GetComponentArray<C1>();
		auto ar = block.GetComponentArray<C2>();
		auto c1 = block.GetComponentArray<C3>();

		for (int i = 0; i < block.last; i++)
		{
			//compares3++;
			compares3+=DummyFunction(ap.Get(i),ar.Get(i),c1.Get(i));
		}
	}, true);
	tim.elapsed();
	compares3 = 0;
	V_ECS.IterateBlocks(Cs.componentlist, [&](ArchetypeBlock & block) {

		auto ap = block.GetComponentArray<C1>();
		auto ar = block.GetComponentArray<C2>();
		auto c1 = block.GetComponentArray<C3>();

		for (int i = 0; i < block.last; i++)
		{
			
			compares3+=DummyFunction(ap.Get(i),ar.Get(i),c1.Get(i));
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
	for (int i = 0; i < 1; i++)
	{
		Compare_CreationDeletion();
		Compare_ComponentAdd();
		Compare_ComponentRemove();
	}
		Compare_SimpleIteration();
		Compare_SimpleIteration_5Comps();
	for (int i = 5; i < 20; i++)
	{
		Compare_Iteration_Pathological(i * 20000);
	}

	

	char a;
	cin >> a;
}