#ifndef DATA_H_
#define DATA_H_
#include<vector>
using namespace std;
class point //���࣬�洢��һ�����������ֵ
{
public:
	int x;
	int y;
};

class polygon			//������࣬����һ�������
{
public:
	vector<point> p; //����εĶ���
	int color;
};

#endif