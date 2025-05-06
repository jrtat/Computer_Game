#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>

using namespace std;

/* Part 1 全局变量 */

// 输出
int new_x, new_y;

// 扩展时的参数
const int counter = 30; // 在被模拟多少次后扩展

// 计时
auto start = chrono::steady_clock::now(), stops = chrono::steady_clock::now();// 计时

// 记录棋盘信息
const int SIZE = 15;
int board[SIZE][SIZE] = { 0 }; //我方1 对方-1 先手不定
struct Coord {
	int x, y;
	Coord(int xx = -1, int yy = -1) { x = xx, y = yy; }
	Coord(const Coord& tmp) { x = tmp.x, y = tmp.y; }
	Coord& operator = (const Coord& other) {
		return *this;
	}
};

vector<Coord> UntriedMoves; // 用来记录没下过的点
int curBoard[SIZE][SIZE]; //一个棋盘的备份
int mycolor = 0; // mycolor  -1 表示我的颜色是board[0][0 - 10]和board[10][0 - 10](红色) / 1 表示我的颜色是board[0 - 10][0]和board[0 - 10][10](蓝色)

// 并查集
int fa[SIZE * SIZE + 4] = { 0 }; // 并查集函数(其中 0,1,2,3分别代表  board[0][0-10] / board[10][0-10]/ board[0-10][0] / board[0-10][10])

// 计算潜力值和机动性的
struct Cmp {
	int pot, act, x, y;
	Cmp(int v1 = 0, int v2 = 0, int t1 = 0, int t2 = 0) {
		pot = v1, act = v2, x = t1, y = t2;
	}
	bool operator < (const Cmp& tmp)const {
		if (tmp.pot == pot) return tmp.act < act;
		return tmp.pot < pot;
	}
};
queue<Coord> q; // 存放当前已被更新, 但没拿来更新其相邻位置的位置
queue<int> fx, fy;
priority_queue<int, vector<int>, greater<int> >Adj[SIZE][SIZE]; // 存放每个点被其他点更新时, 所对应的双距离, 取次小值作为最终双距离
priority_queue<Cmp> Redpl, Bluepl;

int tempBoard[SIZE][SIZE] = { 0 }; //白1,黑-1
int BlueSide1[SIZE][SIZE] = { 0 }, BlueSide2[SIZE][SIZE] = { 0 }, RedSide1[SIZE][SIZE] = { 0 }, RedSide2[SIZE][SIZE] = { 0 }; //双距离
int BlueAct1[SIZE][SIZE] = { 0 }, BlueAct2[SIZE][SIZE] = { 0 }, RedAct1[SIZE][SIZE] = { 0 }, RedAct2[SIZE][SIZE] = { 0 }; //机动性
int vis[SIZE][SIZE] = { 0 }, FloodVis[SIZE][SIZE] = { 0 };

// 选下一步棋
vector<Coord> MustDone, HighVal, MidVal, LowVal; // 给所有可行的下一步棋分类 | （对后两者的存在的必要性保持怀疑）

/*
必做：几乎一定会获胜或者几乎一定是最优的情况（以70%的概率执行这个动作）

高价值：自己潜力值最高和对手潜力值最高的1-3个点（在不执行必做的情况下以90%的概率执行这个动作）

中等价值：自己潜力值和对手潜力值较高的点（在不执行高价值的情况下以70%的概率执行这个动作）

低价值：自己潜力值和对手潜力值都较低的点（在不执行中价值的情况下以70%的概率执行这个动作） // 不存在
*/

/* End Part1 */

/* End Part2 */

/* Part2 函数声明 */
int stepX[6] = { -1,-1, 0, 1, 1 , 0};
int stepY[6] = {  0, 1, 1, 0,-1 ,-1};
int bridgeX1[6] = { -1,-1,-1, 0, 0,-1};
int bridgeY1[6] = {  1, 0, 1, 1,-1, 0};
int bridgeX2[6] = {  0, 0, 1, 1, 1, 1};
int bridgeY2[6] = { -1, 1, 0,-1, 0,-1};

bool jdg1(int x, int y);

bool jdg2(int x, int y);

bool jdg3(int x, int y);

bool jdg4(int x, int y);

void Flood(int x, int y, int s, int mark);

void UpdateAdj(int tx, int ty, int mark);

void Capture(int lastX, int lastY, int curPl);

bool Invalid(int curX, int curY);

void ChoosePos(int* cx, int* cy);

void Calc_Potential();

int get_fa(int x);

int TrytoMerge(int x, int y, int curPl);

void MCTS(int lstX, int lstY);

/* End Part2 */

/* Part3 搜素树的节点 */

class TreeNode {

private:

	double val;// 当前节点的评价值


public:

	int player;// 这一步的玩家(-1,1)
	int n;// 当前节点被更新的次数
	int move_x, move_y;// 这一步的动作
	vector<TreeNode*> children;// 这个节点的儿子
	TreeNode* father;// 这个节点的监护人

	TreeNode(int pos_x = -1, int pos_y = -1, int pl = 0, TreeNode* fa = nullptr) {
		val = 0;
		n = 0;
		move_x = pos_x;
		move_y = pos_y;
		player = pl;
		father = fa;
	}

	TreeNode* Select() {
		TreeNode* best = nullptr;
		double score = -1e9;
		for (auto child : children) {
			double ucb = child->val * 1.0 / child->n + 0.5 * sqrt(2 * log(n) * 1.0 / child->n); //ucb
			if (ucb > score) {
				score = ucb;
				best = child;
			}

		}
		return best;
	}

	TreeNode* Expand() { // 5.6 这一部分

		/* 初始化 把Simulate中的策略也应用在Expand中 */
		TreeNode* ChosenChild = nullptr;
		MustDone.clear();
		HighVal.clear();
		MidVal.clear();
		// LowVal.clear();

		for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end();) {
			if (Invalid(iter->x, iter->y) == 1) {
				cout << "I:" << iter->x << " " << iter->y << endl;
				curBoard[iter->x][iter->y] = 1; // 给谁都无所谓, 干脆给我
				iter = UntriedMoves.erase(iter); // 以后也不用考虑这个点了
			}
			else  iter++;
		}

		Capture(move_y, move_x, -player); // 找出被捕获的点

			/* 向搜素树添加节点 */
		for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end(); iter++) {
			TreeNode* child = new TreeNode(iter->x, iter->y, -player, this); // 子节点玩家对当前玩家取反
			children.push_back(child);
		}
		/* 根据不同概率 随机从 MustDone HighVal LowVal 中选一个给ChosenChild赋值 */

		/* 通过策略选择扩展的点 */
		int nextx, nexty;
		ChoosePos(&nextx, &nexty);
		for (int i = 0; i < children.size(); i++) {
			if (children[i]->move_x == nextx && children[i]->move_y == nexty) {
				ChosenChild = children[i];
				break;
			}
		}
		return ChosenChild;
	}

	double Simulate() {

		// 直接使用 UntriedMoves 和 curBoard 即可
		// 把当前节点的状态作为起始状态
		int lastX = move_x, lastY = move_y, lastplayer = player;

		while (1) { // 直到分出胜负才停

			cout <<"Last"<<":"<<lastplayer <<" " << lastX << " " << lastY << endl;

			/* 判断胜负（判断的是（lastX，lastY）这一步） */
			if (TrytoMerge(lastX, lastY, lastplayer) != 0) {
				return lastplayer * player; // -1 * -1 从表示对手的节点出发且对手赢了 | 1 * 1 从表示我的节点出发且我赢了
			}

			/* 初始化 */
			MustDone.clear();
			HighVal.clear();
			MidVal.clear();
			// LowVal.clear();

			Capture(lastX, lastY, -lastplayer); // 找出被捕获的点

			for (int i = 0; i < MustDone.size();i++) {
				cout <<"Must:" << MustDone[i].x << " " << MustDone[i].y << endl;
			}

			for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end();) {
				if (Invalid(iter->x, iter->y) == 1) {
					cout << "I:" << iter->x << " " << iter->y << endl;
					curBoard[iter->x][iter->y] = 1; // 给谁都无所谓, 干脆给我
					iter = UntriedMoves.erase(iter); // 以后也不用考虑这个点了
				}
				else  iter++;
			}

			/* 根据不同概率 随机从 MustDone HighVal LowVal 中选一个给curX 和 curY 赋值 */

			int curX, curY;
			ChoosePos(&curX, &curY);

			/* 更新 */
			for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end(); iter++) { //更新 UntriedMoves
				if (iter->x == curX && iter->y == curY) {
					UntriedMoves.erase(iter);
					break;
				}
			}
			curBoard[curX][curY] = -lastplayer; // 更新 curBoard
			lastplayer = -lastplayer, lastX = curX, lastY = curY; // 更新lastplayer lastX lastY

		}

	}

	void BackPropagate(double re) { // 更新搜素树中的值
		TreeNode* node = this;
		while (node != nullptr) {
			node->n += 1;
			node->val += re;
			re = -re;
			node = node->father;
		}
	}

};

/* End Part3 */

/* Part4 主函数 */

int main() {

	start = chrono::steady_clock::now(); // 获取开始时间

	int x, y, n;
	//恢复目前的棋盘信息
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
		cin >> x >> y; if (x != -1) board[x][y] = -1;	//对方
		if (i == 0) {
			if (x == -1) {
				mycolor = -1;  // 我先手, 我是红色
			}
			else mycolor = 1; // 对方先手, 我是蓝色
		}
		cin >> x >> y; if (x != -1) board[x][y] = 1;	//我方
	}
	cin >> x >> y;
	if (x != -1) board[x][y] = -1;	//对方

	//此时board[][]里存储的就是当前棋盘的所有棋子信息 且轮到我方下棋

	//注意, 此时board[i][j] = 1表示此处是我方, 并非指此处是某个颜色

	/************************************************************************************/
	/***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/

	if (n == 1) { // 我方先手 下固定位置
		if (x == -1) {
			cout << 1 << ' ' << 2 << endl;
			return 0;
		}
		else {
			mycolor = 1; // 对方先手, 我是蓝色
		}
	}

	/* 初始化 */

	srand(time(0)); // 生成随机数种子

	/* 蒙特卡洛 */

	MCTS(x,y); // 蒙特卡洛树搜索

	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果0
	cout << new_x << ' ' << new_y << endl;

	return 0;

}

/* End Part4 */

/* Part5 函数实现 */

// 判定x,y是否越界 越界返回 0
bool jdg1(int x, int y) {
	return x > 0 && (x <= 11 + 1) && y > 0 && y <= 11; // 对于白色的第一条边ptt，X<=5+1是边界 
}

bool jdg2(int x, int y) {
	return x >= 0 && x <= 11 && y > 0 && y <= 11;
}

bool jdg3(int x, int y) {
	return x > 0 && x <= 11 && y > 0 && (y <= 11 + 1);
}

bool jdg4(int x, int y) {
	return x > 0 && x <= 11 && y >= 0 && y <= 11;
}


void Flood(int x, int y, int s, int mark) {
	FloodVis[x][y] = 1;
	for (int i = 0; i < 6; i++) {
		int tx = x + stepX[i], ty = y + stepY[i];
		if (mark == 1) if ((!jdg1(tx, ty)) || FloodVis[tx][ty] || (tempBoard[tx][ty] == -1))continue;
		if (mark == 2) if ((!jdg2(tx, ty)) || FloodVis[tx][ty] || (tempBoard[tx][ty] == -1))continue;
		if (mark == 3) if ((!jdg3(tx, ty)) || FloodVis[tx][ty] || (tempBoard[tx][ty] == 1))continue;
		if (mark == 4) if ((!jdg4(tx, ty)) || FloodVis[tx][ty] || (tempBoard[tx][ty] == 1))continue;
		FloodVis[tx][ty] = 1;
		if ((tempBoard[tx][ty] == 1 && mark <= 2) || (tempBoard[tx][ty] == -1 && mark > 2)) {
			Flood(tx, ty, s, mark);
			continue; // 个人觉得这个加不加不太影响实际结果
		}
		else if (tempBoard[tx][ty] == 0) {
			fx.push(tx);
			fy.push(ty);
		}
	}
}

void UpdateAdj(int tx, int ty, int mark) {
	vis[tx][ty] = 1;
	q.push({ tx,ty });
	int tmp = Adj[tx][ty].top();
	Adj[tx][ty].pop();
	if (mark == 1)RedSide1[tx][ty] = Adj[tx][ty].top() + 1; // 用次大值更新
	if (mark == 2)RedSide2[tx][ty] = Adj[tx][ty].top() + 1;
	if (mark == 3)BlueSide1[tx][ty] = Adj[tx][ty].top() + 1;
	if (mark == 4)BlueSide2[tx][ty] = Adj[tx][ty].top() + 1;
	Adj[tx][ty].push(tmp); // 保留最大值不被删去
}


// 被捕获的位置 负责模拟时优化
void Capture(int lastX, int lastY, int curPl) { // 传入上一步棋的状态, 判断上一步棋是否导致一个点被捕获

	int captureX, captureY, b1X, b1Y, b2X, b2Y;
	for (int i = 0; i < 6; i++) {
		captureX = lastX + stepX[i], captureY = lastY + stepY[i];
		b1X = lastX + bridgeX1[i], b1Y = lastY + bridgeY1[i];
		b2X = lastX + bridgeX2[i], b2Y = lastY + bridgeY2[i];

		if (b1X < 0 || b2X < 0 || b1X > 10 || b2X > 10 || b1Y < 0 || b2Y < 0 || b1Y > 10 || b2Y > 10 || captureX < 0 || captureY < 0 || captureX > 10 || captureY > 10) continue; // 超出范围一概不
		if (curBoard[captureX][captureY] == 0 && curBoard[b1X][b1Y] == curPl && curBoard[b2X][b2Y] == curPl) {
			MustDone.push_back(Coord(captureX, captureY));
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

void ChoosePos(int* cx, int* cy) { // 选择下一步的走法
	
	Calc_Potential(); // 调用函数计算潜力值和机动性

	if (!MustDone.empty()) {
		if (rand() % 10 >= 7) {  // 70% 的概率返回Mustdone中的内容
			int r = (rand() % (MustDone.size()));
			*cx = MustDone[r].x, *cy = MustDone[r].y;
			return;
		}
	}

	if (!HighVal.empty()) {
		if (rand() % 10 >= 8) {  // 80% 的概率返回HighVal中的内容
			int r = (rand() % (HighVal.size() - 1));
			*cx = HighVal[r].x, *cy = HighVal[r].y;
			return;
		}
	}

	if (!MidVal.empty()) {// 80% 的概率返回HighVal中的内容
		int r = (rand() % (MidVal.size() - 1));
		*cx = MidVal[r].x, * cy = MidVal[r].y;
		return;
	}

	*cx = -1, * cy = -1;
	return;
}

void Calc_Potential() { // 计算双威胁值 用到CurBoard

	/* 初始化 */
	memset(BlueSide1, 0x3f, sizeof(BlueSide1));
	memset(BlueSide2, 0x3f, sizeof(BlueSide2));
	memset(RedSide1, 0x3f, sizeof(RedSide1));
	memset(RedSide2, 0x3f, sizeof(RedSide2));
	memset(BlueSide1, 0x3f, sizeof(BlueAct1));
	memset(BlueSide2, 0x3f, sizeof(BlueAct2));
	memset(RedSide1, 0x3f, sizeof(RedAct1));
	memset(RedSide2, 0x3f, sizeof(RedAct2));


	// 一个新棋盘 保证 红色为1 蓝色为-1 并且下标范围为1-11, 而 0 和 12 行 是边的范围
	for (int i = 0; i <= 10; i++) {
		for (int j = 0; j <= 10; j++) {
			if (mycolor == 1) {
				tempBoard[i + 1][j + 1] = curBoard[i][j];
			}
			else tempBoard[i + 1][j + 1] = -curBoard[i][j];
		}
	}

	/* 测试 */

	tempBoard[2][3] = -1;
	tempBoard[2][4] = -1;
	// tempBoard[3][4] = -1;
	// tempBoard[3][11] = -1;

	/* Part1 预处理上红边 */

	// 初始化vis 初始化对边
	memset(vis, 0, sizeof(vis));
	for (int i = 1; i <= 11; i++) {
		tempBoard[11 + 1][i] = 1;
	}

	// 预处理首行
	for (int i = 1; i <= 11; i++) {
		vis[1][i] = 1;
		if (tempBoard[1][i] == 0) { RedSide1[1][i] = 1; RedAct1[1][i] = 2; } //初始化双距离和机动性 //
		if (tempBoard[1][i] == 1)RedSide1[1][i] = 0;
		if (tempBoard[1][i] != -1) {
			// 更新与之相邻的位置
			for (int j = 0; j < 6; j++) {
				int tx = 1 + stepX[j], ty = i + stepY[j];
				if ((!jdg1(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == -1))continue; // 排除越界 对方棋子 已被更新 三种情况
				Adj[tx][ty].push(RedSide1[1][i]); // 更新 Adj[tx][ty]
				if (tempBoard[tx][ty] == 1) { // 如果更新到同色棋子, 通过一次搜素找到一团同色棋子
					memset(FloodVis, 0, sizeof(FloodVis));
					for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
						int ttx = 1 + stepX[k], tty = i + stepY[k];
						if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
					}
					Flood(tx, ty, RedSide1[1][i], 1);
					while (!fx.empty()) {
						int ti = fx.front(), tj = fy.front();
						fx.pop(), fy.pop();
						Adj[ti][tj].push(RedSide1[1][i]);
						if (!vis[ti][tj] && tempBoard[ti][tj] == 0 && Adj[ti][tj].size() >= 2) { // 更新双距离数组
							UpdateAdj(ti, tj, 1);
						}
					}
					continue;
				}
				if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) { // 更新双距离数组
					UpdateAdj(tx, ty, 1);
				}
			}
		}
	}
	while (!q.empty()) {
		int x = q.front().x, y = q.front().y;
		q.pop();
		for (int j = 0; j < 6; j++) {
			int tx = x + stepX[j], ty = y + stepY[j];
			if ((!jdg1(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == -1))continue; // 排除越界 对方棋子 已被更新 三种情况
			Adj[tx][ty].push(RedSide1[x][y]);
			if (tempBoard[tx][ty] == 1) {
				memset(FloodVis, 0, sizeof(FloodVis));
				for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
					int ttx = x + stepX[k], tty = y + stepY[k];
					if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
				}
				Flood(tx, ty, RedSide1[x][y], 1);
				while (!fx.empty()) {
					int ti = fx.front(), tj = fy.front();
					fx.pop(), fy.pop();
					Adj[ti][tj].push(RedSide1[x][y]);
					if (!vis[ti][tj] && Adj[ti][tj].size() >= 2) { // 更新双距离数组
						UpdateAdj(ti, tj, 1);
					}
				}
				continue;
			}
			if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) {// 更新双距离数组
				UpdateAdj(tx, ty, 1);
			}
		}
	}

	// 算机动性 顺带清空Adj
	for (int i = 1; i <= 11; i++) {
		for (int j = 1; j <= 11; j++) {
			if (tempBoard[i][j] == 0) { // 算机动性
				while (!Adj[i][j].empty() && Adj[i][j].top() + 1 <= RedSide1[i][j]) {
					RedAct1[i][j] += 1;
					Adj[i][j].pop();
				}
			}
			while (!Adj[i][j].empty()) Adj[i][j].pop(); // 清空Adj
		}
	}

	// 恢复对边
	for (int i = 1; i <= 11; i++) {
		tempBoard[11 + 1][i] = 0;
	}

	// Part2 预处理下红边

	// 初始化vis 初始化对边
	memset(vis, 0, sizeof(vis));
	for (int i = 1; i <= 11; i++) {
		tempBoard[0][i] = 1;
	}

	// 预处理首行
	for (int i = 1; i <= 11; i++) {
		vis[11][i] = 1; //
		if (tempBoard[11][i] == 0) { RedSide2[11][i] = 1;  RedAct2[11][i] = 2; } //初始化双距离和机动性 //
		if (tempBoard[11][i] == 1)RedSide2[11][i] = 0; //
		if (tempBoard[11][i] != -1) { //
			// 更新与之相邻的位置
			for (int j = 0; j < 6; j++) {
				int tx = 11 + stepX[j], ty = i + stepY[j]; // 
				if ((!jdg2(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == -1))continue; // 排除越界 对方棋子 已被更新 三种情况 //
				Adj[tx][ty].push(RedSide2[11][i]); // 更新 Adj[tx][ty] //
				if (tempBoard[tx][ty] == 1) { // 如果更新到同色棋子, 通过一次搜素找到一团同色棋子 //
					memset(FloodVis, 0, sizeof(FloodVis));
					for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
						int ttx = 11 + stepX[k], tty = i + stepY[k];
						if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
					}
					Flood(tx, ty, RedSide2[11][i], 2); //
					while (!fx.empty()) {
						int ti = fx.front(), tj = fy.front();
						fx.pop(), fy.pop();
						Adj[ti][tj].push(RedSide2[11][i]); // 
						if (!vis[ti][tj] && tempBoard[ti][tj] == 0 && Adj[ti][tj].size() >= 2) { // 更新双距离数组
							UpdateAdj(ti, tj, 2); // 
						}
					}
					continue;
				}
				if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) { // 更新双距离数组
					UpdateAdj(tx, ty, 2); // 
				}
			}
		}
	}
	while (!q.empty()) {
		int x = q.front().x, y = q.front().y;
		q.pop();
		for (int j = 0; j < 6; j++) {
			int tx = x + stepX[j], ty = y + stepY[j];
			if ((!jdg2(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == -1))continue; // 排除越界 对方棋子 已被更新 三种情况 //
			Adj[tx][ty].push(RedSide2[x][y]); // 
			if (tempBoard[tx][ty] == 1) { //
				memset(FloodVis, 0, sizeof(FloodVis));
				for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
					int ttx = x + stepX[k], tty = y + stepY[k];
					if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
				}
				Flood(tx, ty, RedSide2[x][y], 2); //
				while (!fx.empty()) {
					int ti = fx.front(), tj = fy.front();
					fx.pop(), fy.pop();
					Adj[ti][tj].push(RedSide2[x][y]); //
					if (!vis[ti][tj] && Adj[ti][tj].size() >= 2) { // 更新双距离数组
						UpdateAdj(ti, tj, 2); //
					}
				}
				continue;
			}
			if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) {// 更新双距离数组
				UpdateAdj(tx, ty, 2); //
			}
		}
	}

	// 算机动性 顺带清空Adj
	for (int i = 1; i <= 11; i++) {
		for (int j = 1; j <= 11; j++) {
			if (tempBoard[i][j] == 0) { // 算机动性
				while (!Adj[i][j].empty() && Adj[i][j].top() + 1 <= RedSide2[i][j]) { //
					RedAct2[i][j] += 1; //
					Adj[i][j].pop();
				}
			}
			while (!Adj[i][j].empty()) Adj[i][j].pop(); // 清空Adj
		}
	}

	// 恢复对边
	for (int i = 1; i <= 11; i++) {
		tempBoard[0][i] = 0;
	}

	// Part3 预处理上蓝边

	// 初始化vis 初始化对边
	memset(vis, 0, sizeof(vis));
	for (int i = 1; i <= 11; i++) {
		tempBoard[i][11 + 1] = -1;
	}

	// 预处理首行
	for (int i = 1; i <= 11; i++) {
		vis[i][1] = 1; // vis[i][1]
		if (tempBoard[i][1] == 0) { BlueSide1[i][1] = 1;  BlueAct1[i][1] = 2; } //初始化双距离和机动性 //
		if (tempBoard[i][1] == -1) BlueSide1[i][1] = 0; //
		if (tempBoard[i][1] != 1) { //
			// 更新与之相邻的位置
			for (int j = 0; j < 6; j++) {
				int tx = i + stepX[j], ty = 1 + stepY[j];
				if ((!jdg3(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == 1))continue; // 排除越界 对方棋子 已被更新 三种情况 //
				Adj[tx][ty].push(BlueSide1[i][1]); // 更新 Adj[tx][ty] //
				if (tempBoard[tx][ty] == -1) { // 如果更新到同色棋子, 通过一次搜素找到一团同色棋子 //
					memset(FloodVis, 0, sizeof(FloodVis));
					for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
						int ttx = i + stepX[k], tty = 1 + stepY[k];
						if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
					}
					Flood(tx, ty, BlueSide1[i][1], 3); //
					while (!fx.empty()) {
						int ti = fx.front(), tj = fy.front();
						fx.pop(), fy.pop();
						Adj[ti][tj].push(BlueSide1[i][1]); // 
						if (!vis[ti][tj] && tempBoard[ti][tj] == 0 && Adj[ti][tj].size() >= 2) { // 更新双距离数组
							UpdateAdj(ti, tj, 3); // 
						}
					}
					continue;
				}
				if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) { // 更新双距离数组
					UpdateAdj(tx, ty, 3); //
				}
			}
		}
	}
	while (!q.empty()) {
		int x = q.front().x, y = q.front().y;
		q.pop();
		for (int j = 0; j < 6; j++) {
			int tx = x + stepX[j], ty = y + stepY[j];
			if ((!jdg3(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == 1))continue; // 排除越界 对方棋子 已被更新 三种情况 //
			Adj[tx][ty].push(BlueSide1[x][y]); // 
			if (tempBoard[tx][ty] == -1) { //
				memset(FloodVis, 0, sizeof(FloodVis));
				for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
					int ttx = x + stepX[k], tty = y + stepY[k];
					if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
				}
				Flood(tx, ty, BlueSide1[x][y], 3); //
				while (!fx.empty()) {
					int ti = fx.front(), tj = fy.front();
					fx.pop(), fy.pop();
					Adj[ti][tj].push(BlueSide1[x][y]); //
					if (!vis[ti][tj] && Adj[ti][tj].size() >= 2) { // 更新双距离数组
						UpdateAdj(ti, tj, 3); //
					}
				}
				continue;
			}
			if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) {// 更新双距离数组
				UpdateAdj(tx, ty, 3); //
			}
		}
	}

	// 算机动性 顺带清空Adj
	for (int i = 1; i <= 11; i++) {
		for (int j = 1; j <= 11; j++) {
			if (tempBoard[i][j] == 0) { // 算机动性
				while (!Adj[i][j].empty() && Adj[i][j].top() + 1 <= BlueSide1[i][j]) { //
					BlueAct1[i][j] += 1; //
					Adj[i][j].pop();
				}
			}
			while (!Adj[i][j].empty()) Adj[i][j].pop(); // 清空Adj
		}
	}

	// 恢复对边
	for (int i = 1; i <= 11; i++) {
		tempBoard[i][11 + 1] = 0;
	}

	// Part4 预处理下蓝边

	// 初始化vis 初始化对边
	memset(vis, 0, sizeof(vis));
	for (int i = 1; i <= 11; i++) {
		tempBoard[i][0] = -1;
	}

	// 预处理首行
	for (int i = 1; i <= 11; i++) {
		vis[i][11] = 1; // vis[i][1]
		if (tempBoard[i][11] == 0) { BlueSide2[i][11] = 1;  BlueAct2[i][11] = 2; } //初始化双距离和机动性 //
		if (tempBoard[i][11] == -1) BlueSide2[i][11] = 0; //
		if (tempBoard[i][11] != 1) { //
			// 更新与之相邻的位置
			for (int j = 0; j < 6; j++) {
				int tx = i + stepX[j], ty = 11 + stepY[j];
				if ((!jdg4(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == 1))continue; // 排除越界 对方棋子 已被更新 三种情况 //
				Adj[tx][ty].push(BlueSide2[i][11]); // 更新 Adj[tx][ty] //
				if (tempBoard[tx][ty] == -1) { // 如果更新到同色棋子, 通过一次搜素找到一团同色棋子 //
					memset(FloodVis, 0, sizeof(FloodVis));
					for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
						int ttx = i + stepX[k], tty = 11 + stepY[k];
						if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
					}
					Flood(tx, ty, BlueSide2[i][11], 4); //
					while (!fx.empty()) {
						int ti = fx.front(), tj = fy.front();
						fx.pop(), fy.pop();
						Adj[ti][tj].push(BlueSide2[i][11]); // 
						if (!vis[ti][tj] && tempBoard[ti][tj] == 0 && Adj[ti][tj].size() >= 2) { // 更新双距离数组
							UpdateAdj(ti, tj, 4); // 
						}
					}
					continue;
				}
				if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) { // 更新双距离数组
					UpdateAdj(tx, ty, 4); //
				}
			}
		}
	}
	while (!q.empty()) {
		int x = q.front().x, y = q.front().y;
		q.pop();
		for (int j = 0; j < 6; j++) {
			int tx = x + stepX[j], ty = y + stepY[j];
			if ((!jdg4(tx, ty)) || vis[tx][ty] || (tempBoard[tx][ty] == 1))continue; // 排除越界 对方棋子 已被更新 三种情况 //
			Adj[tx][ty].push(BlueSide2[x][y]); // 
			if (tempBoard[tx][ty] == -1) { //
				memset(FloodVis, 0, sizeof(FloodVis));
				for (int k = 0; k < 6; k++) { // 防止同一棋子被重复计数
					int ttx = x + stepX[k], tty = y + stepY[k];
					if (tempBoard[ttx][tty] == 0) FloodVis[ttx][tty] = 1;
				}
				Flood(tx, ty, BlueSide2[x][y], 4); //
				while (!fx.empty()) {
					int ti = fx.front(), tj = fy.front();
					fx.pop(), fy.pop();
					Adj[ti][tj].push(BlueSide2[x][y]); //
					if (!vis[ti][tj] && Adj[ti][tj].size() >= 2) { // 更新双距离数组
						UpdateAdj(ti, tj, 4); //
					}
				}
				continue;
			}
			if (!vis[tx][ty] && tempBoard[tx][ty] == 0 && Adj[tx][ty].size() >= 2) {// 更新双距离数组
				UpdateAdj(tx, ty, 4); //
			}
		}
	}

	// 算机动性 顺带清空Adj
	for (int i = 1; i <= 11; i++) {
		for (int j = 1; j <= 11; j++) {
			if (tempBoard[i][j] == 0) { // 算机动性
				while (!Adj[i][j].empty() && Adj[i][j].top() + 1 <= BlueSide2[i][j]) { //
					BlueAct2[i][j] += 1; //
					Adj[i][j].pop();
				}
			}
			while (!Adj[i][j].empty()) Adj[i][j].pop(); // 清空Adj
		}
	}

	// 恢复对边
	for (int i = 1; i <= 11; i++) {
		tempBoard[i][0] = 0;
	}

	for (int i = 0; i < UntriedMoves.size(); i++) {
		int tx = UntriedMoves[i].x, ty = UntriedMoves[i].y;
		Redpl.push(Cmp(RedSide1[tx][ty] + RedSide2[tx][ty], min(RedAct1[tx][ty], RedAct2[tx][ty]), tx, ty));
		Bluepl.push(Cmp(BlueSide1[tx][ty] + BlueSide2[tx][ty], min(BlueAct1[tx][ty], BlueAct2[tx][ty]), tx, ty));
	}

	/* 更新 HighVal 和 MidVal */

	//记得恢复原来的坐标(0-10)
	for (int i = 1; i <= 2; i++) { // 每个队列选两个
		if (!Redpl.empty()) {
			HighVal.push_back(Coord(Redpl.top().x - 1, Redpl.top().y - 1));
			Redpl.pop();
		}
		if (!Bluepl.empty()) {
			HighVal.push_back(Coord(Bluepl.top().x - 1, Bluepl.top().y - 1));
			Bluepl.pop();
		}

	}
	while (!Redpl.empty()) {
		MidVal.push_back(Coord(Redpl.top().x - 1, Redpl.top().y - 1));
		Redpl.pop();
	}
	while (!Bluepl.empty()) {
		MidVal.push_back(Coord(Bluepl.top().x - 1, Bluepl.top().y - 1));
		Bluepl.pop();
	}

}

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

void MCTS(int lstX, int lstY) {

	TreeNode* root = new TreeNode(lstX, lstY, -1, nullptr); // 初始化根节点（根节点的玩家记为对方）

	int ctr = 0; // 删

	while (1) {

		ctr++;

		cout << ctr << endl;

		/* 计时 */
		stops = chrono::steady_clock::now();

		if (chrono::duration_cast<chrono::milliseconds>(stops - start).count() >= 99999) { // 提前1ms结束
			return;
		}

		/* 预处理 */
		// curBoard: 用来记录待扩展节点所对应的状态 随Select的进行而更新 并在Simulate时使用
		TreeNode* node = root;
		memcpy(curBoard, board, sizeof(board));
		
		// 初始化并查集
		for (int i = 0; i < 11 * 11 + 4; i++) fa[i] = i;

		for (int i = 0; i <= 10; i++) {
			for (int j = 0; j <= 10; j++) {
				// cout << mycolor << " " << i << " " << j <<" " << board[i][j] << endl;
				if (board[i][j] != 0) TrytoMerge(i, j, board[i][j]);
			}
		}

		/*
		for (int ii = 0; ii <= 10; ii++) {
			for (int jj = 0; jj <= 10; jj++) {
				cout << fa[ii * 11 + jj + 4]<<" ";
			}
			cout << endl;
		}
		*/

		/* Select部分 */

		cout << "Selection"<<endl;

		int alreadyWin = 0;
		while (!node->children.empty()) {
			node = node->Select();
			// 更新curBoard
			curBoard[node->move_x][node->move_y] = node->player;
			// 更新并查集(顺带判断胜负)
			if (TrytoMerge(node->move_x, node->move_y, node->player) != 0) {
				node->BackPropagate(1);
				alreadyWin = 1;
				break;
			}
		}

		// 这种情况是: 还没到叶节点就已经赢了
		if (alreadyWin == 1) continue;

		if (node == nullptr) { // 某次Select返回的是nullptr 理论上来说不可能
			return;
		}

		// cout << node->move_x << " " << node->move_y << endl;

		/* Expand 部分 */

		cout << "Expand" << endl;

		// 初始化UntriedMoves (没错 我把UnusedGrid删了 主要是感觉没啥用)
		for (int i = 0; i <= 10; i++)
			for (int j = 0; j <= 10; j++)
				if (curBoard[i][j] == 0) UntriedMoves.push_back(Coord(i, j));

		if (node->n >= counter) {
			// 只有一个节点被模拟了足够的次数时, 才进行扩展
			node = node->Expand();
		}

		if (node == nullptr) { // 这个叶节点扩展不出来一个节点, 也就是说, 该节点表示的状态是一个摆满棋子的棋盘
			return;
		}

		/* Simulate 部分 */

		cout << "Simulate" << endl;

		double result = node->Simulate();

		/* BackPropagate 部分 */

		cout << "BP" << endl;

		node->BackPropagate(result);
	}

	/* 选出根节点的最优子节点并更新new_x, new_y */

	TreeNode* BestChild = nullptr;
	int MaxVisit = -1;
	for (auto child : root->children) {
		if (child->n > MaxVisit) {
			MaxVisit = child->n;
			BestChild = child;
		}
	}
	if (BestChild) {
		new_x = BestChild->move_x;
		new_y = BestChild->move_y;
	}
	else { // 什么都没找到 ?
		return;
	}

	return;
}

/* End Part5 */
