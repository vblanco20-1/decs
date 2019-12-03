# decs

High performance data oriented Entity/Component/System library.

Inspired by the way the unity ECS handles memory, this ECS stores entities in contiguous structure of arrays blocks per entity signature. 


# Design
This ECS uses a memory model similar to the model Unity engine uses in their new ECS. Its based around storing entities grouped by their "archetype".
The archetype of a entity is just what components does it have. 
Each archetype has a list of blocks, each of them with entities inside. 
The blocks are perfect structure of arrays for the components of those entities.
Finding a given component list to iterate on searches the registed archetypes to find the blocks that hold the entities for that query. This is the main difference comparing it to other designs, and allows it to do pure high performance data oriented design while iterating. 

# Features
* Designed around parallel execution. Easy paralelized iteration with C++ 17 parallel algorithms.
* Guaranteed "perfect" memory model. Component data is stored in perfectly contiguous structure of arrays fashion, allowing SIMD or memcopying of component data.
* Component alignement, constructor and destructors are respected. 
* Block iteration allows branchless execution and data-oriented existence processing.
* Doesnt need components to be registered, and allows cross-dll matching as long as typeid hashes are equal.
* Supports negative and positive querying of components.
* Its possible to iterate from multiple threads at once. 


# Performance Characteristics
* Slower than usual entity creation and destruction. Only depends on the number of components you have and the overhead of finding a block to slot the entity in. 
* Adding and Removing components is slower than other ECS, as it requires a full copy of all the component data from one block to another. Its slower the more components you have on the entity that is getting modified.
* The number of entities does not affect iteration speed. Only number of total archetypes does.
* Memory usage depends on number of 16kb blocks allocated, at least 1 per archetype. 

# Caveats
* This project is a heavy WIP and under constant changes. Use EnTT if you want a stable ECS.

# Todo
* Implement component locking through atomics to assert if unsafe operations are used.
* Implement block-level data for shared data beetween groups of entities.
* Implement a way to detect modifications, or callbacks.




