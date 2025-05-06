#include <iostream>
#include <iomanip>
using namespace std;

int fa[200] = { 0 }, mycolor = -1; // 红先

int stepX[6] = { -1,-1, 0, 0, 1, 1 };
int stepY[6] = { 0, 1,-1, 1,-1, 0 };

int curBoard[12][12] = { 0 };

int get_fa(int x) {
	if (fa[x] == x) return x;
	return fa[x] = get_fa(fa[x]);
}

int TrytoMerge(int x, int y, int curPl) { // 尝试把（x,y）与其相邻点合并 | 返回1表示游戏结束

	int flag1 = 0, flag2 = 0; // 是否接触到下半边 是否接触到上半边
	// fa中 0,1,2,3分别代表  board[0][0-10] / board[10][0-10]/ board[0-10][0] / board[0-10][10] 四个边
	// mycolor  -1 表示我的颜色是board[0][0 - 10]和board[10][0 - 10](红色) / 1 表示我的颜色是board[0 - 10][0]和board[0 - 10][10](蓝色)

	int curfa = get_fa(x * 11 + y + 4);

	if (curfa == 0 || curfa == 2) flag1 = 1;
	if (curfa == 1 || curfa == 3) flag2 = 1;

	// 判断是否到达自己的边缘 如果到达则记录
	if (x == 0 && ((mycolor == -1 && curPl == 1) || (mycolor == 1 && curPl == -1))) { // 0 边
		flag1 = 1;
		if (flag2 == 1) {
			return 1;
		}
		else fa[curfa] = 0;
	}
	if (x == 10 && ((mycolor == -1 && curPl == 1) || (mycolor == 1 && curPl == -1))) { // 1 边
		flag2 = 1;
		if (flag1 == 1) {
			return 1;
		}
		else fa[curfa] = 1;
	}
	if (y == 0 && ((mycolor == -1 && curPl == -1) || (mycolor == 1 && curPl == 1))) { // 2 边
		flag1 = 1;
		if (flag2 == 1) {
			return 1;
		}
		else fa[curfa] = 2;
	}
	if (y == 10 && ((mycolor == -1 && curPl == -1) || (mycolor == 1 && curPl == 1))) { // 3 边
		flag2 = 1;
		if (flag1 == 1) {
			return 1;
		}
		else fa[curfa] = 3;
	}
	// 更新 curfa
	curfa = get_fa(x * 11 + y + 4);
	for (int i = 0; i < 6; i++) {
		int tmpX = x + stepX[i], tmpY = y + stepY[i];
		if (tmpX < 0 || tmpY < 0 || tmpX > 10 || tmpY > 10) continue; // 排除不合法位置
		if (curBoard[tmpX][tmpY] != curPl) continue; // 排除对手棋子 和 空位置

		int tmpfa = get_fa(tmpX * 11 + tmpY + 4);
		if (tmpfa == 0 || tmpfa == 2) { flag1 = 1; } // 判断是否有到边上的点
		if (tmpfa == 1 || tmpfa == 3) { flag2 = 1; }

		if (flag1 == 1 && flag2 == 1) { // 如果此时上下已经连通 说明已经胜利 直接返回
			return 1;
		}

		if (tmpfa <= 3) { // 合并, 要保证0-3作为集合的代表
			fa[curfa] = tmpfa;
		}
		else fa[tmpfa] = curfa;
	}
	return 0;
}

int main() {
	for (int i = 0; i < 11 * 11 + 4; i++)fa[i] = i;
	while (1) {
		int x, y;
		cin >> x >> y;
		curBoard[x][y] = -1;
		cout<<TrytoMerge(x,y,-1)<<endl;
		for (int i = 0; i < 11; i++) {
			for (int k = 0; k < i; k++) {
				cout << "  ";
			}
			for (int j = 0; j < 11; j++) {
				cout <<setw(5)<< fa[get_fa(i * 11 + j + 4)];
			}
			cout << endl;
		}

	}
}

/*

1 2
1 4
0 6
1 5
1 3

4 5
5 5
6 5

10 5
9 5
8 5
7 5

2 5
3 5

*/

/*

4 3
5 4
4 4

4 1
4 2
4 0

6 5
6 6
6 7
6 8
6 9
6 10
5 5

*/
