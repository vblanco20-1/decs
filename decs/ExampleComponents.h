#pragma once


struct Position {

	Position() = default;
	Position(const Position & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};

struct C1{

	C1() = default;
	C1(const C1 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
template<std::size_t>
struct comp { int x; };


struct C2 {

	C2() = default;
	C2(const C2 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
struct C3{

	C3() = default;
	C3(const C3 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
//};
struct Rotation  {
	Rotation() = default;
	Rotation(const Rotation & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;

};
struct Speed  {
	Speed() = default;

	float x;

};
struct Acceleration   {
	Acceleration() = default;

	float x;

};
struct BigComponent {

	//static constexpr ComponentGUID GUID = 324132;
	int data[100];
};
