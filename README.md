# decs

Prototype data-oriented ECS. Not ready for real use.

Inspired by the way the unity ECS handles memory, this ECS stores entities in contiguous structure of arrays blocks per entity signature. 


# Design
This ECS uses a memory model similar to the model Unity engine uses in their new ECS. Its based around storing entities grouped by their "archetype".
The archetype of a entity is just what components does it have. 
Each archetype has a list of blocks, each of them with entities inside. 
The blocks are perfect structure of arrays for the components of those entities.
Finding a given component list to iterate on searches the registed archetypes to find the blocks that hold the entities for that query. This is the main difference comparing it to other designs, and allows it to do pure high performance data oriented design while iterating. 

# Features
* Designed around parallel execution. Features paralelized iteration with C++ 17 parallel algorithms.
* Guaranteed "perfect" memory model. Component data is stored in perfectly contiguous structure of arrays fashion, allowing SIMD or memcopying of component data. 
* Block iteration allows branchless execution and data-oriented existence processing.
* Components can be added or removed while iterating safely without double-execution.
* Batched entity creation, to create many entities with a specific signature in an optimal way.
* Doesnt need components to be registered, and allows cross-dll matching as long as typeid hashes are equal.
* Supports negative and positive querying of components.

# Performance Characteristics
* Slower than usual entity creation and destruction. Only depends on the number of components you have and the overhead of finding a block to slot the entity in. Batched creation is much faster but still slower than other ECS.
* Adding and Removing components is slower than other ECS, as it requires a full copy of all the component data from one block to another. Its slower the more components you have on the entity that is getting modified.
* To find a signature match, it iterates all the blocks. At the moment is O(N) where N is the number of total live archetypes. Unnafected by number of entities. Has room for improvement.
* The number of entities does not affect iteration speed. Only number of total archetypes does.
* Memory usage depends on number of blocks and their components. At the moment its much higher than it should be due to wasted space.

# Caveats
* This project is a heavy WIP and not intended for use yet. Use EnTT if you want a usable ECS.
* Current code is not clean and the ECS has poor safety, so its very easy to use it wrong and cause havok due to raw memory control going wrong.
* Iteration APIs are verbose and easy to get wrong.
* Several parts of the code are not optimized and will use more performance and memory than it should.

# Todo
* Clean the entire API
* Improve multithreading support by allowing multiple iterations at the same time.
* Implement component locking through atomics to assert if unsafe operations are used.
* Implement block-level data for shared data beetween groups of entities.
* Implement tags (zero byte components) that dont allocate any memory when used.
* Implement a way to detect modifications, or callbacks.
* Implement safe high-level iterators, to wrap raw accessing of block memory.




