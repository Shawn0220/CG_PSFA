#define _CRT_SECURE_NO_WARNINGS
// PSFA.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// Polygon scanning and filling algorithm 
// 多边形扫描填充算法

#include <iostream>
#include<gl/glut.h>
#include"polygon.h"
#include <string>
#include <fstream>
#include"edge.h"

static const int screenwidth = 800;  //自定义窗口宽度
static const int screenheight = 600; //自定义窗口高度

int move_x, move_y; //鼠标当前坐标值，在鼠标移动动态画线时使用
bool select = false; //多边形封闭状态判断变量，当为true时即按下鼠标右键，将多边形封闭
bool flash = false;
bool Transform = false;
int angle = 0;
bool translate = false;
bool readtxt = false;
bool writetxt = false;
bool AET_printer = false;
int now_color = 1;
bool ALLOW_printAET = false;

vector<point> p; //多边形点集向量
vector<polygon> s; //多边形类向量，用来保存已经画完的多边形

//边表
Edge* ET[screenheight];
//活性边表
Edge* AET;

void init()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(0.0, screenwidth, 0.0, screenheight);
}

void sortAET(Edge *q)
{
	while (q->next)
	{
		Edge* p = q;
		while (p->next && p->next->next)
		{
			if (p->next->x > p->next->next->x)
			{
				int nowymax;
				float nowx;
				float nowdx;
				nowymax = p->next->ymax;
				nowx = p->next->x;
				nowdx = p->next->dx;
				p->next->ymax = p->next->next->ymax;
				p->next->x = p->next->next->x;
				p->next->dx = p->next->next->dx;
				p->next->next->ymax = nowymax;
				p->next->next->x = nowx;
				p->next->next->dx = nowdx;
			}
			else
			{
				p = p->next;
			}
		}
		q = q->next;
	}
}

// 多边形填充算法
void mypaint(vector<point> mypoints, int color)
{
	int max_Y = 0;
	for (int i = 0; i < mypoints.size(); i++)
	{
		if (mypoints[i].y > max_Y)
		{
			max_Y = mypoints[i].y;
		}
	}
	// 初始化 ET
	Edge* ET[screenheight];
	for (int i = 0; i < max_Y; i++)
	{
		ET[i] = new Edge();
		ET[i]->next = nullptr;
	}
	// 初始化 AET
	AET = new Edge();
	AET->next = nullptr;

	//glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);


	glBegin(GL_POINTS);

	// 建立边表ET 存储处理过非极值点的边
	for (int i = 0; i < mypoints.size(); i++)
	{
		// 当前边的前一条边的起点
		int x0 = mypoints[(i - 1 + mypoints.size()) % mypoints.size()].x;
		int y0 = mypoints[(i - 1 + mypoints.size()) % mypoints.size()].y;

		// 当前边的起点 前一条边的终点
		int x1 = mypoints[i].x;
		int y1 = mypoints[i].y;

		// 当前边的终点 后一条边的起点
		int x2 = mypoints[(i + 1) % mypoints.size()].x;
		int y2 = mypoints[(i + 1) % mypoints.size()].y;

		// 当前边的后一条边的终点
		int x3 = mypoints[(i + 2) % mypoints.size()].x;
		int y3 = mypoints[(i + 2) % mypoints.size()].y;

		// 当前边是水平线，不用填充，进入下一条边
		if (y1 == y2)
			continue;

		//分别计算下端点y坐标、上端点y坐标、下端点x坐标和斜率倒数
		int ymin = y1 < y2 ? y1 : y2;
		int ymax = y1 > y2 ? y1 : y2;
		float x = y1 < y2 ? x1 : x2;
		float dx = (x1 - x2) * 1.0f / (y1 - y2);

		int nowy;
		if (y1 < y2)
		{
			nowy = y1;
			ymax = y2;
		}
		else
		{
			nowy = y2;
			ymax = y1;
		}
		

		//奇点特殊处理，若点2->1->0的y坐标单调递减则y1为奇点，若点1->2->3的y坐标单调递减则y2为奇点
		if (((y0 < y1) && (y1 < y2)) || ((y2 < y1) && (y3 < y2)))
		{
			nowy++;
			x += dx;
		}

		// 将该边插入边表ET
		Edge* p = new Edge();
		p->ymax = ymax;
		p->x = x;
		p->dx = dx;
		p->next = ET[nowy]->next;
		ET[nowy]->next = p;
	}

	//扫描线从下往上扫描，每轮扫描 y 加 1
	for (int i = 0; i < max_Y; i++)
	{
		//取出ET中当前扫描行的所有边并按x的递增顺序（若x相等则按dx的递增顺序）插入AET
		while (ET[i]->next)
		{
			//取出ET中当前扫描行表头位置的边
			Edge* pInsert = ET[i]->next;
			Edge* p = AET;
			//在AET中搜索合适的插入位置
			while (p->next)
			{
				if (pInsert->x > p->next->x)
				{
					p = p->next;
					continue;
				}
				if (pInsert->x == p->next->x && pInsert->dx > p->next->dx) // 等于的时候按斜率排
				{
					p = p->next;
					continue;
				}
				//找到位置
				break;
			}
			//将pInsert从ET中删除，并插入AET的当前位置
			ET[i]->next = pInsert->next;
			pInsert->next = p->next;
			p->next = pInsert;
		}

		// AET中的边两两配对并填色
		Edge* p = AET;
		while (p->next && p->next->next)
		{
			for (int x = p->next->x; x < p->next->next->x; x++)
			{
				switch (color)
				{
				case 1:
					glColor3f(1.0, 0.0, 0.0);
					break;
				case 2:
					glColor3f(1.0, 1.0, 0.0);
					break;
				case 3:
					glColor3f(0.0, 1.0, 1.0);
					break;
				case 4:
					glColor3f(0.0, 1.0, 0.0);
					break;
				case 5:
					glColor3f(0.0, 0.0, 1.0);
					break;
				}
				//随机闪烁颜色
				if (flash)
					glColor3f(rand() % 2, rand() % 2, rand() % 2);
				glVertex2i(x, i);
			}
			// 打印活性边表信息
			if (ALLOW_printAET)
				if (AET_printer)
				{
					cout << i << endl;
					cout << p->next->dx << "  |  " << p->next->x << "  |  " << p->next->ymax << endl;
					cout << p->next->next->dx << "  |  " << p->next->next->x << "  |  " << p->next->next->ymax << endl;
				}
			p = p->next->next; // 跳到间隔一个的交点
		}

		//删除AET中满足y=ymax的边
		p = AET;
		while (p->next)
		{
			if (p->next->ymax == i)
			{
				Edge* pDelete = p->next;
				p->next = pDelete->next;
				pDelete->next = nullptr;
				delete pDelete;
			}
			else
			{
				p = p->next;
			}
		}

		// 更新AET中边的x值
		p = AET;
		while (p->next)
		{
			p->next->x +=  p->next->dx;
			p = p->next;
		}

		// 重新排序AET中的边，按从小到大
		Edge* q = AET;
		sortAET(q);
	}

	glEnd();
}

void savepolygon()
{
	ofstream fout("C:\\Users\\52954\\Desktop\\output_polygon.txt");     //创建一个data.txt的文件
	fout << s.size() << endl; // 多边形个数
	for (int i = 0; i < s.size(); i++) {
		fout << s[i].color << endl; //多边形颜色
		fout << s[i].p.size() << endl;
		for (int j = 0; j < s[i].p.size(); j++) {
			fout << s[i].p[j].x << " " << screenheight - s[i].p[j].y << endl; //多边形点
		}
	}
	fout.close();                  //关闭文件
	writetxt = !writetxt;
}
// C:\\Users\\52954\\Desktop\\output_polygon.txt
void my_readtxt()
{
	int pcnt = 0;
	glutPostRedisplay(); //重绘窗口
	FILE* fp;//文件指针
	//char filename[100] = "C:\\Users\\52954\\Desktop\\my_polygon.txt";
	char filename[100] = "C:\\Users\\52954\\Desktop\\output_polygon.txt";
	if ((fopen(filename, "r")) == NULL)
	{   /*二进制只读打开文件*/
		printf("fail to open file\n");
		exit(1);
	}
	fp = fopen(filename, "r");
	fscanf_s(fp, "%d", &(pcnt));
	for (int j = 0; j < pcnt; j++)
	{
		int lines;
		fscanf_s(fp, "%d", &(now_color));
		fscanf_s(fp, "%d", &(lines));
		for (int i = 0; i < lines; i++)
		{
			int xx, yy;
			fscanf_s(fp, "%d", &(xx));//输出数据到数组
			fscanf_s(fp, "%d", &(yy));
			point fv;
			fv.x = xx;
			fv.y = screenheight - yy;
			p.push_back(fv); //将点信息存入多边形点集向量p中
			glutPostRedisplay(); //重绘窗口
		}

		polygon sq;
		int i;
		//将封闭了的多边形保存到多边形类中
		for (i = 0; i < p.size(); i++)
			sq.p.push_back(p[i]);
		sq.color = now_color;
		s.push_back(sq); //将绘成的多边形存入多边形类向量中
		mypaint(sq.p, sq.color); //给当前画完的多边形填色
		p.clear();
	}

	fclose(fp);/*关闭文件*/
	//s.clear();
	readtxt = !readtxt;
}

void lineSegment()
{

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);   //设定颜色，既是线段颜色也是填充色

	int i, j;                 //两个循环控制变量，在下面的向量循环和数组循环中将被多次调用。
	if (!s.empty())            //看多边形类向量是否为空，即判断除了当前正在画的多边形是否还有曾经已经画好的多边形
	{
		for (i = 0; i < s.size(); i++)   //对多边形类向量循环，该向量中的每个元素代表一个多边形
		{
			int k = s[i].p.size();  //将一个多边形的点的个数，后面划线会用到
//			s[i].line(); //生成多边形的边
			for (j = 0; j < s[i].p.size(); j++) //画多边形
			{
				glBegin(GL_LINES); //将当前的点与后一个点连线
				glVertex2i(s[i].p[j].x, s[i].p[j].y);	//确定前一个点
				glVertex2i(s[i].p[(j + 1) % k].x, s[i].p[(j + 1) % k].y);//确定后一个点，通过取模操作来避免越界问题，该思路来源于循环队列.
				glEnd();
			}
			if (Transform)
			{
				// 旋转
				glRotatef(angle, 0, 0, -1);
				if (translate)
					glTranslated(60, 60, 0);
				//	printf("%d",rotate0);
				Transform = !Transform;
			}

			mypaint(s[i].p, s[i].color);  //为当前的多边形填充颜色
//			s[i].edge.clear(); //清空当前多边形的边向量
			glColor3f(0.0, 0.0, 0.0);
		}
	}
	i = 0;
	j = p.size() - 1;
	while (i < j)           //循环画图，画当前正在画的多边形
	{
		glBegin(GL_LINES); //将已经确定的点连接起来
		glVertex2i(p[i].x, p[i].y);
		glVertex2i(p[i + 1].x, p[i + 1].y);
		glEnd();
		i++;
	}
	if (!p.empty())//画当前点到鼠标点的线段（变化）
	{
		//	int i = p.size() - 1; //将确定的最后一个点与当前鼠标所在位置连线，即动态画线
		glBegin(GL_LINES);
		glVertex2i(p[j].x, p[j].y);
		glVertex2i(move_x, move_y);
		glEnd();
	}

	// readtxt = true;
	// 读多边形文件
	if (readtxt)
		my_readtxt();

	// 保存多边形
	if (writetxt)
		savepolygon();

	if (select) //判断右键是否被点下
	{
		select = false; //将状态值置为假
		if (!p.empty())
		{
			glBegin(GL_LINES); //自动将当前点和第一点连接起来封闭多边形
			glVertex2i(p[j].x, p[j].y);
			glVertex2i(p[0].x, p[0].y);
			glEnd();
			polygon sq;
			//将封闭了的多边形保存到多边形类中
			for (i = 0; i < p.size(); i++)
				sq.p.push_back(p[i]);
			sq.color = now_color;
			s.push_back(sq); //将绘成的多边形存入多边形类向量中
			mypaint(sq.p, sq.color); //给当前画完的多边形填色
		}
		p.clear();
	}

	glFlush();
}

void myMouse(int button, int state, int x, int y) //鼠标点击事件响应函数
{
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)//当鼠标左键被点击
	{
		point v;  //申请一个点类变量，点类为自定义类，在zl.h中定义
		v.x = x; //将点击处的点坐标，即x和y的值存入v中
		v.y = screenheight - y;
		cout << "该点坐标  x:" << v.x << ",  y:" << v.y << '\n';
		p.push_back(v); //将点信息存入多边形点集向量p中
		glutPostRedisplay(); //重绘窗口
	}

	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) //当鼠标右键被点击
	{
		select = true;
		AET_printer = true;
		Transform = true;
		glutPostRedisplay();
	}
}

void myPassiveMotion(int x, int y) //鼠标移动事件响应函数
{
	move_x = x; //保存当前鼠标所在的坐标的值
	move_y = screenheight - y;
	AET_printer = false;
	glutPostRedisplay();
}

void mymenuOption(GLint option)
{
	switch (option)
	{
	case 1:// 撤销上一步
		if (!p.empty())
			p.pop_back();
		break;
	case 2:// 重画目前在绘制的多边形
		p.clear();
		break;
	case 3:// 删除上一个已经绘制好的多边形
		if (!s.empty())
		s.pop_back();
		break;
	case 4:// 清空画布
		p.clear();
		s.clear();
		glutPostRedisplay();
		break;
	case 5:
		if (angle == 0)
			angle = 10;
		else
			angle = 0;
		break;
	case 6:
		translate = !translate;
		break;
	case 7:
		readtxt = !readtxt;
		break;
	case 8:
		now_color = (now_color + 1) % 5;
		break;
	case 9:
		writetxt = !writetxt;
		break;
	case 10:
		ALLOW_printAET = !ALLOW_printAET;
		break;
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(50, 100);
	glutInitWindowSize(screenwidth, screenheight);

	glutCreateWindow("多边形扫描填充算法");
	init();
	glutMouseFunc(myMouse); //鼠标点击消息监控，即监控鼠标是否被点击，若被点击就调用myMouse函数
	glutDisplayFunc(lineSegment);

	glutCreateMenu(mymenuOption);
	glutAddMenuEntry("Undo last point", 1);
	glutAddMenuEntry("Shut down current drawing", 2);
	glutAddMenuEntry("Undo last polygon", 3);
	glutAddMenuEntry("Clear board", 4);
	//glutAddMenuEntry("polygon flash", 5);
	//glutAddMenuEntry("Rotate Clockwise or Counterclockwise Exchange", 6);
	glutAddMenuEntry("Rotate switch", 5);
	glutAddMenuEntry("Translate switch", 6);
	glutAddMenuEntry("color shift", 8);
	glutAddMenuEntry("Read polygon-file", 7);
	glutAddMenuEntry("Save polygon into txt", 9);
	glutAddMenuEntry("Print AET switch", 10);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);
	glutPassiveMotionFunc(myPassiveMotion); //鼠标移动消息监控，即监控鼠标是否移动，若移动就调用myPassiveMotion函数
	glutMainLoop();

	return 0;
}