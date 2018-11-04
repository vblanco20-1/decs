#pragma once


struct Position : public Component<Position> {

	Position() = default;
	Position(const Position & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};

struct C1 : public Component<C1> {

	C1() = default;
	C1(const C1 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
struct C2 : public Component<C2> {

	C2() = default;
	C2(const C2 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
struct C3 : public Component<C3> {

	C3() = default;
	C3(const C3 & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;
};
//};
struct Rotation : public Component<Rotation> {
	Rotation() = default;
	Rotation(const Rotation & other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}
	float x, y, z;

};
struct Speed : public Component<Speed> {
	Speed() = default;

	float x;

};
struct Acceleration : public Component<Acceleration> {
	Acceleration() = default;

	float x;

};
struct BigComponent : public Component<BigComponent> {

	//static constexpr ComponentGUID GUID = 324132;
	int data[10000];
};
