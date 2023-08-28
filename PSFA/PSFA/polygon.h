#ifndef DATA_H_
#define DATA_H_
#include<vector>
using namespace std;
class point //点类，存储了一个点的两坐标值
{
public:
	int x;
	int y;
};

class polygon			//多边形类，存了一个多边形
{
public:
	vector<point> p; //多边形的顶点
	int color;
};

#endif