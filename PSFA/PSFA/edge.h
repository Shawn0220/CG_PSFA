#ifndef DATA_H_1
#define DATA_H_1
#include<vector>
using namespace std;

class Edge
{
public:
	int ymax;
	float x;
	float dx;
	Edge* next;
};
#endif