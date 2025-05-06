#include <iostream>
#include <iomanip>
using namespace std;

int fa[200] = { 0 }, mycolor = 1; // 红先

int stepX[6] = { -1,-1, 0, 1, 1 , 0 };
int stepY[6] = { 0, 1, 1, 0,-1 ,-1 };
int bridgeX1[6] = { -1,-1,-1, 0, 0,-1 };
int bridgeY1[6] = { 1, 0, 1, 1,-1, 0 };
int bridgeX2[6] = { 0, 0, 1, 1, 1, 1 };
int bridgeY2[6] = { -1, 1, 0,-1, 0,-1 };
int curBoard[12][12] = { 0 };

// 被捕获的位置 负责模拟时优化
void Capture(int lastX, int lastY, int curPl) { // 传入上一步棋的状态, 判断上一步棋是否导致一个点被捕获

	int captureX, captureY, b1X, b1Y, b2X, b2Y;
	for (int i = 0; i < 6; i++) {
		captureX = lastX + stepX[i], captureY = lastY + stepY[i];
		b1X = lastX + bridgeX1[i], b1Y = lastY + bridgeY1[i];
		b2X = lastX + bridgeX2[i], b2Y = lastY + bridgeY2[i];

		if (b1X < 0 || b2X < 0 || b1X > 10 || b2X > 10 || b1Y < 0 || b2Y < 0 || b1Y > 10 || b2Y > 10 || captureX < 0 || captureY < 0 || captureX > 10 || captureY > 10) continue; // 超出范围一概不
		if (curBoard[captureX][captureY] == 0 && curBoard[b1X][b1Y] == curPl && curBoard[b2X][b2Y] == curPl) {
			cout <<"Capture:" << captureX << " " << captureY << endl;
		}
	}
	return;
}

bool Invalid(int curX, int curY) { // 返回1说明（curX, curY）是无效位置
	int ctr1 = 0, ctr2 = 0;
	for (int j = 0; j <= 12; j++) {
		int i = j % 6;
		int tmpX = curX + stepX[i], tmpY = curY + stepY[i];
		if (tmpX < 0 || tmpY < 0 || tmpX > 10 || tmpY > 10) return 0; // 没想好边缘怎么处理
		// 更新
		if (curBoard[tmpX][tmpY] == 0) {
			ctr1 = 0, ctr2 = 0;
		}
		if (curBoard[tmpX][tmpY] == 1) {
			ctr1++;
			ctr2 = 0;
		}
		if (curBoard[tmpX][tmpY] == -1) {
			ctr2++;
			ctr1 = 0;
		}
		// 判定1
		if (ctr1 == 4) {
			return 1;
		}
		else if (ctr1 == 3) {
			int tx = curX + stepX[(i + 2) % 6], ty = curY + stepY[(i + 2) % 6];
			if (curBoard[tx][ty] == 1) return 1;
		}
		else if (ctr1 == 2) {
			int tx1 = curX + stepX[(i + 2) % 6], ty1 = curY + stepY[(i + 2) % 6];
			int tx2 = curX + stepX[(i + 3) % 6], ty2 = curY + stepY[(i + 3) % 6];
			if (curBoard[tx1][ty1] == 1 && curBoard[tx2][ty2] == 1) return 1;
		}
		// 判定2
		if (ctr2 == 4) {
			return 1;
		}
		else if (ctr2 == 3) {
			int tx = curX + stepX[(i + 2) % 6], ty = curY + stepY[(i + 2) % 6];
			if (curBoard[tx][ty] == -1) return 1;
		}
		else if (ctr2 == 2) {
			int tx1 = curX + stepX[(i + 2) % 6], ty1 = curY + stepY[(i + 2) % 6];
			int tx2 = curX + stepX[(i + 3) % 6], ty2 = curY + stepY[(i + 3) % 6];
			if (curBoard[tx1][ty1] == -1 && curBoard[tx2][ty2] == -1) return 1;
		}
	}
	return 0;
}

int main() {
	curBoard[1][2] = 1;
	curBoard[1][3] = 1;
	curBoard[2][3] = 1;
	curBoard[3][2] = 1;
	curBoard[4][3] = 1;
	curBoard[3][4] = 1;
	curBoard[4][5] = -1;
	curBoard[5][5] = -1;
	curBoard[5][6] = -1;
	curBoard[3][7] = -1;
	cout << Invalid(2,2) << endl;
	cout << Invalid(3,3) << endl;
	cout << Invalid(0,3) << endl;
	cout << Invalid(1,4) << endl;
	cout << Invalid(4,6) << endl;
	cout << Invalid(4,4) << endl;
	curBoard[4][2] = -1;
	Capture(4, 2, 1);
	curBoard[4][6] = 1;
	Capture(4, 6, -1);
}

