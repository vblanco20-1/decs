#include "decs.hpp"

#include "ExampleComponents.h"


BaseComponent::IDCounter BaseComponent::family_counter_ = 1;

using namespace std;
//using namespace decs;

struct ScopeTimer {
	ScopeTimer(const char* ptext)
	{
		clock = chrono::high_resolution_clock::now();
		text = ptext;
	}

	~ScopeTimer() {
		auto end = chrono::steady_clock::now();
		//Blocks.ValidateAll();

		auto diff = end - clock;
		double miliseconds = chrono::duration <double, milli>(diff).count();

		printf(text,miliseconds);
		printf("\n");
	}
	std::chrono::time_point<chrono::high_resolution_clock> clock;
	const char* text;
};

#define SCOPE_BENCH( text ) auto timer = ScopeTimer(text);


int main()
{
	// Get current flag  
	//int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	//
	//// Turn on leak-checking bit.  
	//tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	//
	//// Turn off CRT block checking bit.  
	//tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
	//
	//// Set flag to the new value.  
	//_CrtSetDbgFlag(tmpFlag);

	for (int r = 0; r < 100; r++)
	{


		Archetype PosRot;
		PosRot.add_component<Position>();
		PosRot.add_component<Rotation>();
		PosRot.add_component<BigComponent>();


		ECSWorld Blocks;
		//Blocks.Blocks.reserve(50000);
		Blocks.BlockStorage.reserve(100);
		//Blocks.CreateBlock(PosRot);


		//ArchetypeBlock block(PosRot);

		std::vector<Position> Positions1;
		std::vector<Rotation> Rotations1;
		std::vector<EntityHandle> Handles;
		Archetype PR;
		PR.add_component<Position>();
		PR.add_component<Rotation>();

		ComponentList search = PR.componentlist;
		ComponentList empty;
		ComponentList noBig;
		//Blocks.CreateBlock(PR);

		noBig.metatypes.push_back(Metatype::BuildMetatype<BigComponent>());

		

		//for (int i = 0; i < 1000000; i++)
		//{
		//	/*Handles.push_back( Blocks.CreateEntity(PR);// / / );
		//}
		{
			SCOPE_BENCH("Creating 1 million entities: %f ms")
			Handles = Blocks.CreateEntityBatched(PR, 1000000);

		}
		{
			SCOPE_BENCH("Adding component to 333.000 entities: %f ms")

			for (int i = 0; i < Handles.size(); i++)
			{
				if (i % 3 == 0)
				{
					Blocks.AddComponent<Acceleration>(Handles[i]);//, BigComponent());
				}
				//if (i % 5 == 0)			
			}
		}

	

		
		{
			SCOPE_BENCH("Removing component to 1 million entities: %f ms");

				for (int i = 0; i < Handles.size(); i++)
				{

					auto eh = Handles[i];
					Blocks.RemoveComponent<Position>(eh);//, BigComponent());
				}
		}
		
		{
			SCOPE_BENCH("Adding component to 1 million entities: %f ms");
				for (int i = 0; i < Handles.size(); i++)
				{
					Blocks.AddComponent<Position>(Handles[i]);

				}
		}
		

		{
			SCOPE_BENCH("Iterating 1 million entities: %f ms");
			int niter = 0;
			//while (true)
			{
				Blocks.IterateBlocks(search, empty, [&](ArchetypeBlock & block) {

					//auto ap = block.GetComponentArray<Position>();
					//	auto ar = block.GetComponentArray<Rotation>();
					for (int i = 0; i < block.last; i++)
					{
						niter++;
						//Position p;
						//p.x = i;
						//p.y = i % 2;
						//p.z = p.x + p.y;
						//Rotation r;
						//r.x = p.y;
						//r.y = p.z;
						//r.z = p.x;
						//
						////Positions1.push_back(p);
						////Rotations1.push_back(r);
						//
						//Position &p1 = ap.Get(i);// = p;
						//Rotation &r1 = ar.Get(i);// = r;
						//p1 = p;
						//r1 = r;
					}
				});
			}
			




			cout << niter << " blocks" << endl;
		}
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

		
		{
			SCOPE_BENCH("Iterating 1 million entities - Component doesnt exist: %f ms");

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
		}

		cout << endl << endl << endl;


	}
	return 0;

	
}


