#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std;

const int SIZE = 15; // 意★义★不★明
int board[SIZE][SIZE] = { 0 }; //我方1 对方-1 先手不定
int new_x, new_y; // 输出

/* Part 1 全局变量 */

const int counter = 60; // 在被模拟多少次后扩展

struct Coord {
	int x, y;
	Coord(int xx = -1, int yy = -1) { x = xx, y = yy; }
	Coord(Coord& tmp) { x = tmp.x, y = tmp.y; }
	Coord& operator = (const Coord& other) {
		return *this;
	}
};

vector<Coord> UntriedMoves; // 记录开始扩展时, 可用的点
vector<Coord> EmptyGrid;  // 用来记录没下过的点

int curBoard[SIZE][SIZE]; //一个棋盘的备份
int upperR[SIZE][SIZE], upperB[SIZE][SIZE], lowerR[SIZE][SIZE], lowerB[SIZE][SIZE]; // 计算潜力值的工具
vector<Coord> HighVal, MidVal, LowVal, MustDone; // 给所有可行的下一步棋分类 | （对后两者的存在的必要性保持怀疑）
/* 
必做：几乎一定会获胜或者几乎一定是最优的情况（以70%的概率执行这个动作）

高价值：自己潜力值最高和对手潜力值最高的1-3个点（在不执行必做的情况下以90%的概率执行这个动作）

中等价值：自己潜力值和对手潜力值较高的点（在不执行高价值的情况下以70%的概率执行这个动作）

低价值：自己潜力值和对手潜力值都较低的点（在不执行中价值的情况下以70%的概率执行这个动作）
*/

int fa[SIZE * SIZE] = { 0 }; // 并查集函数(其中 0,1,2,3分别代表  board[0][0-10] / board[10][0-10]/ board[0-10][0] / board[0-10][10])
int mycolor = 0; // -1 表示我的颜色是board[0][0-10]和board[10][0-10] / 1 表示我的颜色是board[0-10][0]和board[0-10][10]

// 目前位置还没有地方正式修改过mycolor

/* End Part1 */

/* Part2 搜素树的节点 */

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

	TreeNode* Expand() {

		// 初始化

		TreeNode* ChosenChild = nullptr;
		MustDone.clear();
		HighVal.clear();
		MidVal.clear();
		LowVal.clear();

		for (int i = 0; i < UntriedMoves.size(); i++) {

			/* Simulate中的两个优化同样可以适用于Expand */

			Check1(move_y, move_x, -player); // 找出被捕获的点

			if (Check2(UntriedMoves[i].x, UntriedMoves[i].y) == 1) {
				UntriedMoves.erase(UntriedMoves.begin() + i); // 以后也不用考虑这个点了
				continue;
			}

			// 这里计算每个节点的效益 然后放入HighVal LowVal MustDone

			/* 向搜素树添加节点 */

			TreeNode* child = new TreeNode(move_x, move_y, -player, this); // 子节点玩家对当前玩家取反
			children.push_back(child);

			/* 根据不同概率 随机从 MustDone HighVal LowVal 中选一个扩展 */

		}
		return ChosenChild;
	}

	double Simulate() {

		// 直接使用 UntriedMoves 和 curBoard 即可
		// 把当前节点的状态作为起始状态
		int lastX = move_x, lastY = move_y, lastplayer = player;

		while (1) { // 直到分出胜负才停

			/* 判断胜负（判断的是（lastX，lastY）这一步） */
			if (TrytoMerge(lastX, lastY, lastplayer) != 0) {
				return lastplayer * player; // -1 * -1 从表示对手的节点出发且对手赢了 | 1 * 1 从表示我的节点出发且我赢了
			}

			/* 初始化 */
			MustDone.clear();
			HighVal.clear();
			MidVal.clear();
			LowVal.clear();

			/* 选出一个节点 */

			Check1(lastX, lastY, -lastplayer); // 找出被捕获的点

			for (int i = 0; i < UntriedMoves.size(); i++) {
				if (Check2(UntriedMoves[i].x, UntriedMoves[i].y) == 1) {
					UntriedMoves.erase(UntriedMoves.begin() + i); // 以后也不用考虑这个点了
					continue;
				}
			}

			// 没写好
			int curX, curY;

			/* 更新 */
			// 更新 UntriedMoves
			curBoard[][] = -lastplayer; // 更新 curBoard
			lastplayer = -lastplayer;
			lastX = curX;
			lastY = curY;

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

/* End Part2 */

/* Part3 函数 */

int stepX[6] =    {-1,-1, 0, 0, 1, 1};
int stepY[6] =    { 0, 1,-1, 1,-1, 0};
int bridgeX1[6] = {-1,-1,-1,-1, 0, 0};
int bridgeY1[6] = { 1, 0, 0, 1,-1, 1};
int bridgeX2[6] = { 0, 0, 1, 1, 1, 1};
int bridgeY2[6] = {-1, 1,-1, 0, 0,-1};

void Check1(int lastX, int lastY, int curPl) { // 被捕获的位置 模拟时优化1
	int captureX, captureY, b1X, b1Y, b2X, b2Y;
	for (int i = 0; i < 6; i++) {
		captureX = lastX + stepX[i], captureY = lastY + stepX[i];
		b1X = lastX + bridgeX1[i], b1Y = lastY + bridgeY1[i];
		b2X = lastX + bridgeX2[i], b2Y = lastY + bridgeY2[i];

		if (b1X < 0 || b2X < 0 || b1X > 10 || b2X > 10 || b1Y < 0 || b2Y < 0 || b1Y > 10 || b2Y > 10 || captureX < 0 || captureY <0 || captureX > 10 || captureY > 10) continue; // 超出范围一概不
		if (curBoard[captureX][captureY] == 0 && curBoard[b1X][b1Y] == curPl && curBoard[b2X][b2Y] == curPl) {
			MustDone.push_back(Coord(captureX, captureY));
		}
	}
	return;
}

bool Check2(int curX, int curY) { // 返回1说明（curX, curY）是无效位置
	int ctr1 = 0, ctr2 = 0;
	for (int j = 0; j <= 12; j++) {
		int i = j % 6;
		int tmpX = curX + stepX[i], tmpY = curY + stepY[i];
		if (curBoard[tmpX][tmpY] == 1) {
			ctr1++;
			ctr2 = 0;
			if (ctr1 == 4) {
				return 1;
			}
			else if (ctr1 == 3) {
				int tx = curX + stepX[(i + 2) % 6], ty = curY + stepY[(i + 2) % 6];
				if (curBoard[tx][ty] == -1) return 1;
			}
			else if (ctr1 == 2) {
				int tx1 = curX + stepX[(i + 2) % 6], ty1 = curY + stepY[(i + 2) % 6];
				int tx2 = curX + stepX[(i + 3) % 6], ty2 = curY + stepY[(i + 3) % 6];
				if (curBoard[tx1][ty1] == -1 && curBoard[tx2][ty2] == -1) return 1;
			}
		}
		if (curBoard[tmpX][tmpY] == -1) {
			ctr2++;
			ctr1 = 0;
			if (ctr2 == 4) {
				return 1;
			}
			else if (ctr2 == 3) {
				int tx = curX + stepX[(i + 2) % 6], ty = curY + stepY[(i + 2) % 6];
				if (curBoard[tx][ty] == 1) return 1;
			}
			else if (ctr2 == 2) {
				int tx1 = curX + stepX[(i + 2) % 6], ty1 = curY + stepY[(i + 2) % 6];
				int tx2 = curX + stepX[(i + 3) % 6], ty2 = curY + stepY[(i + 3) % 6];
				if (curBoard[tx1][ty1] == 1 && curBoard[tx2][ty2] == 1) return 1;
			}
		}
	}
}

void Calc_Potential() { // 计算双威胁值 用到CurBoard
	// 初始化
	for (int i = 0; i <= 10; i++)
		for (int j = 0; j <= 10; j++)
			lowerB[i][j] = lowerR[i][j] = upperB[i][j] = upperR[i][j] = 1e9;
}

int get_fa(int x) {
	if (fa[x] == x) return x;
	fa[x] = get_fa(fa[x]);
	return fa[x];
}

int TrytoMerge(int x, int y,int curPl) { // 尝试把（x,y）与其相邻点合并 | 返回1表示游戏结束
	vector<int> fathers;
	int flag1 = 0, flag2 = 0; // 是否接触到下半边 是否接触到上半边

	// fa中 0,1,2,3分别代表  board[0][0-10] / board[10][0-10]/ board[0-10][0] / board[0-10][10]
	// mycolor  -1 表示我的颜色是board[0][0 - 10]和board[10][0 - 10] / 1 表示我的颜色是board[0 - 10][0]和board[0 - 10][10]
	// （x,y）判断是否到达自己的边缘 如果到达则记录
	if (x == 0 && ((mycolor == 1 && curPl == 1) || (mycolor == -1 && curPl == -1))) {
		fa[x * 10 + y + 4] = 0;
		flag1 = 1;
	}
	if (x == 10 && ((mycolor == 1 && curPl == 1) || (mycolor == -1 && curPl == -1))) {
		fa[x * 10 + y + 4] = 1;
		flag2 = 1;
	}
	if (y == 0 && ((mycolor == 1 && curPl == -1) || (mycolor == -1 && curPl == 1))) {
		fa[x * 10 + y + 4] = 2;
		flag1 = 1;
	}
	if (y == 10 && ((mycolor == 1 && curPl == -1) || (mycolor == -1 && curPl == 1))) {
		fa[x * 10 + y + 4] = 3;
		flag2 = 1;
	}

	//查询所有与（x,y）相邻的位置
	for (int i = 0; i < 6; i++) {
		int tmpX = x + stepX[i], tmpY = y + stepY[i];

		if (tmpX < 0 || tmpY < 0 || tmpX > 10 || tmpY > 10 ) continue; // 排除不合法位置
		if (curBoard[tmpX][tmpY] != curPl)continue; 

		if (get_fa(tmpX * 10 + tmpY + 4) == 0 || get_fa(tmpX * 10 + tmpY + 4) == 2) flag1 = 1; // 判断是否有到边上的点
		if (get_fa(tmpX * 10 + tmpY + 4) == 1 || get_fa(tmpX * 10 + tmpY + 4) == 3) flag2 = 1;

		if (flag1 == 1 && flag2 == 1) { // 如果此时上下已经连通 说明已经胜利 直接返回
			return 1;
		}

		if (curBoard[tmpX][tmpY] == curPl) { // 把与（x,y）相邻的点都与（x,y）合并 让（x,y）作并查集的父节点
			fa[get_fa(tmpX * 10 + tmpY + 4)] = x * 10 + y + 4;
		}
	}
	return 0;
}

void Init() { 
	// EmptyGrid 把所有空节点加入
	// fa 初始化所有已经在棋盘上的点
	for (int i = 0; i <= 10; i++)
		for (int j = 0; j <= 10; j++)
			if (board[i][j] == 0) {
				EmptyGrid.push_back(Coord(i, j));
			}
			else {
				TrytoMerge(i,j,board[i][j]); 
			}
	return;
}

void MCTS() {
	int limit = 1000;

	TreeNode* root = new TreeNode(-1, -1, -1, nullptr); // 初始化根节点（根节点的玩家记为对方）

	for (int i = 0; i < limit; i++) { // 姑且先循环固定的次数

		/* 预处理 */
		TreeNode* node = root;
		// UntriedMoves 和 curBoard: 用来记录待扩展节点所对应的状态 随Select的进行而更新 并在Simulate时使用
		UntriedMoves = EmptyGrid;
		memcpy(curBoard, board, sizeof(board));
		for (int i = 0; i < 10 * 10; i++) { // 初始化并查集
			fa[i + 4] = i + 4;
		}

		/* Select部分 */
		int alreadyWin = 0;
		while (!node->children.empty()) {
			node = node->Select();
			// 更新 UntriedMoves （能简化吗？）
			for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end(); iter++) {
				if (iter->x == node->move_x && iter->y == node->move_y) {
					UntriedMoves.erase(iter);
					break;
				}
			}
			curBoard[node->move_x][node->move_y] = node->player; // 更新curBoard
			if (TrytoMerge(node->move_x, node->move_y, node->player) != 0) {
				node->BackPropagate(1);
				alreadyWin = 1;
				break;
			}
		}

		if (alreadyWin == 1) continue;

		if (node == nullptr) { // 某次Select返回的是nullptr 理论上来说不可能
			return;
		}

		/* Expand 部分 */
		if (node->n >= counter) {
			// 只有一个节点被模拟了足够的次数时, 才进行扩展
			node = node->Expand();
		}


		if (node == nullptr) {
			// 这个叶节点扩展不出来一个节点, 也就是说, 该节点表示的状态是一个摆满棋子的棋盘
			return;
		}

		/* Simulate 部分 */
		// memcpy(curBoard, board, sizeof(board)); 没必要还原吧
		double result = node->Simulate();

		/* BackPropagate 部分 */
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
	else {
		Coord fallback = EmptyGrid[rand() % EmptyGrid.size()];
		new_x = fallback.x;
		new_y = fallback.y;
	}

	return;
}

/* End Part3 */

int main() {
	int x, y, n;
	//恢复目前的棋盘信息
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
		cin >> x >> y; if (x != -1) board[x][y] = -1;	//对方
		cin >> x >> y; if (x != -1) board[x][y] = 1;	//我方
	}
	cin >> x >> y;
	if (x != -1) board[x][y] = -1;	//对方

	//此时board[][]里存储的就是当前棋盘的所有棋子信息 且轮到我方下棋

	/************************************************************************************/
	/***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/

	if (n == 1 && x == -1) { // 我方先手 下固定位置
		cout << 1 << ' ' << 2 << endl;
		return 0;
	}

	srand(time(0)); // 生成随机数种子

	Init(); // 初始化EmptyGrod

	MCTS(); // 蒙特卡洛树搜索

	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果
	cout << new_x << ' ' << new_y << endl;
	return 0;

}
