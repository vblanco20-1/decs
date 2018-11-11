
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

	Entt_ECS.each( [&](auto e) {
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




void Compare_ComponentAdd()
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(100);
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
		V_ECS.AddComponent<Position>(e);
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
	Entt_ECS.view<Position>().each([&](auto entity,const auto &c) {

		const int x = c.x;
		const int y = c.y;
		compares+= x % 20 + y;
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

	timer tim;

	int compares = 0;
	Entt_ECS.view<Position,Rotation,C1,C2,C3>().each([&](auto entity, auto c, auto c1, auto c2, auto c3, auto c4 ) {
		compares++;
	});
	double Entt_creation = tim.elapsed();

	int compares2 = 0;
	V_ECS.IterateBlocks(All.componentlist, empty.componentlist, [&](ArchetypeBlock & block) {

		//auto ap = block.GetComponentArray<Position>();
		//	auto ar = block.GetComponentArray<Rotation>();
		for (int i = 0; i < block.last; i++)
		{
			compares2++;
		}
	});
	double Decs_creation = tim.elapsed();

	cout << "Total Iterations: " << compares << ":" << compares2 << endl;
	Print_Comparaision(Entt_creation, Decs_creation);
}

void Compare_Iteration_Pathological()
{
	ECSWorld V_ECS;
	V_ECS.BlockStorage.reserve(10000);
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

	std::cout << "Iterating entities: pathological random case 10 Component find 3-------------------------------" << std::endl;

	std::mt19937 rng(0);
	std::uniform_int_distribution<int> uniform_dist(1, 10);
	//create entt entities
	for (std::uint64_t i = 0; i < 1000000L; i++) {
		auto e = Entt_ECS.create();
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<Position>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<Rotation>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			C1 dummy1;
			dummy1.x = 1.0f;
			dummy1.y = 2.0f;
			dummy1.z = 0.5f;
			Entt_ECS.accommodate<C1>(e, dummy1);
		}
		if (uniform_dist(rng) < 4)
		{
			C2 dummy1;
			dummy1.x = 1.0f;
			dummy1.y = 2.0f;
			dummy1.z = 0.5f;
			Entt_ECS.accommodate<C2>(e, dummy1);
		}
		if (uniform_dist(rng) < 4)
		{
			C3 dummy1;
			dummy1.x = 1.0f;
			dummy1.y = 2.0f;
			dummy1.z = 0.5f;
			Entt_ECS.accommodate<C3>(e, dummy1);
		}	
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<1>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<2>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<3>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<4>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			Entt_ECS.assign<comp<5>>(e);
		}
	}
	std::mt19937 rng2(0);
	std::uniform_int_distribution<int> uniform_dist2(1, 10);
	for (std::uint64_t i = 0; i < 1000000L; i++) {
		auto e = V_ECS.CreateEntity(empty);

		if (uniform_dist2(rng2) < 4)
		{
			V_ECS.AddComponent<Position>(e);
		}
		if (uniform_dist2(rng2) < 4)
		{
			V_ECS.AddComponent<Rotation>(e);
		}
		if (uniform_dist2(rng2) < 4)
		{

			C1 dummy1;
			dummy1.x = 1.0f;
			dummy1.y = 2.0f;
			dummy1.z = 0.5f;
			V_ECS.accomodate<C1>(e, dummy1);
		}
		if (uniform_dist2(rng2) < 4)
		{
			C2 dummy1;
			dummy1.x = 1.0f;
			dummy1.y = 2.0f;
			dummy1.z = 0.5f;
			V_ECS.accomodate<C2>(e, dummy1);
		}
		if (uniform_dist2(rng2) < 4)
		{
			C3 dummy1;
			dummy1.x = 1.0f;
			dummy1.y = 2.0f;
			dummy1.z = 0.5f;
			V_ECS.accomodate<C3>(e, dummy1);
		}

		if (uniform_dist(rng) < 4)
		{
			V_ECS.AddComponent<comp<1>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			V_ECS.AddComponent<comp<2>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			V_ECS.AddComponent<comp<3>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			V_ECS.AddComponent<comp<4>>(e);
		}
		if (uniform_dist(rng) < 4)
		{
			V_ECS.AddComponent<comp<5>>(e);
		}

	}
	//auto handles = V_ECS.CreateEntityBatched(All, 1000000L);
	for (int i = 0; i < 1; i++)
	{
		timer tim;

		int compares = 0;
		Entt_ECS.view<C1, C2, C3>().each([&](auto entity, auto c2, auto c3, auto c4) {
			compares += uniform_dist(rng) +c2.x +c3.z+c4.y;
		});
		double Entt_creation = tim.elapsed();
		int compares2 = 0;
		V_ECS.IterateBlocks(Cs.componentlist, [&](ArchetypeBlock & block) {

			auto ap = block.GetComponentArray<C1>();
			auto ar = block.GetComponentArray<C2>();
			auto a3 = block.GetComponentArray<C3>();
			for (int i = 0; i < block.last; i++)
			{
				compares2 += uniform_dist2(rng2) + ap.Get(i).x + ar.Get(i).z + a3.Get(i).y;
			}
		});

		double Decs_creation = tim.elapsed();

		cout << "Sum: " << compares << ":" << compares2 << endl;		
		Print_Comparaision(Entt_creation, Decs_creation);
	}

}

int main()
{
	for (int i = 0; i < 100; i++)
	{
		//Compare_Creation();
	
	cout << "===========Comparing Decs against Entt ================" << endl << endl;
	Compare_CreationDeletion();
	Compare_ComponentAdd();
	Compare_ComponentRemove();
	Compare_SimpleIteration();
	Compare_SimpleIteration_5Comps();
	Compare_Iteration_Pathological();
	}

	char a;
	cin >> a;
}