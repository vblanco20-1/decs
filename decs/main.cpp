#include "decs.hpp"


BaseComponent::IDCounter BaseComponent::family_counter_ = 1;

using namespace std;
//using namespace decs;




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
		//	/*Handles.push_back( Blocks.CreateEntity(PR);// / / );
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


