# decs
Prototype data-oriented prototype ECS. Not fit for any real use.

Inspired by the way the unity ECS handles memory, this ECS stores entities in contiguous structure of arrays blocks per entity signature. 

The amount of components it can use is pretty much infinite, due to a lack of a bitset.

The block approach makes iteration really efficient when you have many entities of the same signature. 

It would not scale well if you have many singular entities with different signatures.

The iteration is block based, with each block having one contiguous array per component type, this allows for customized matching for optional components or negative queries. 
Becouse the arrays are allways contiguous (guaranteed) everything can be vectorized and you can do branchless iteration.

Current speed is not very good due to suboptimal algorythms done for simplicity.
