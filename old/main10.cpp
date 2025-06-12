#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>
#include <iomanip>

using namespace std;

/* Part 1 全局变量 */

// 输出
int new_x = -150, new_y = -150;

// 扩展时的参数
	// 在被模拟多少次后扩展
	// ucb公式中的参数c 越大越侧重广度
	// 扩展多少节点 容忍多少潜力值的差距
int counter = 50;
double uct = 0.2;
int ExpNum = 15, Tolerant = 2;

// 记录棋盘信息
const int SIZE = 13;
int board[SIZE][SIZE] = { 0 }; //我方1 对方-1 先手不定
struct Coord {
	int x, y;
	Coord(int xx = -1, int yy = -1) { x = xx, y = yy; }
	Coord(const Coord& tmp) { x = tmp.x, y = tmp.y; }
	Coord& operator = (const Coord& other) {
		x = other.x;
		y = other.y;
		return *this;
	}
};

vector<Coord> UntriedMoves; // 用来记录没下过的点
int curBoard[SIZE][SIZE]; // 一个棋盘的备份
int mycolor = 0; // mycolor  -1 表示我的颜色是board[0][0 - 10]和board[10][0 - 10](红色) / 1 表示我的颜色是board[0 - 10][0]和board[0 - 10][10](蓝色)

// 并查集
int fa[250] = { 0 }; // 并查集函数(其中 0,1,2,3分别代表  board[0][0-10] / board[10][0-10]/ board[0-10][0] / board[0-10][10])

// 计算潜力值
struct Cmp {
	int pot, x, y;
	Cmp(int v = 0, int t1 = 0, int t2 = 0) {
		pot = v, x = t1, y = t2;
	}
	bool operator < (const Cmp& tmp)const {
		return tmp.pot < pot;
	}
};
queue<Coord> q; // 存放当前已被更新, 但没拿来更新其相邻位置的位置
queue<int> fx, fy;
priority_queue<int, vector<int>, greater<int> >Adj[SIZE][SIZE]; // 存放每个点被其他点更新时, 所对应的双距离, 取次小值作为最终双距离
priority_queue<Cmp> Redpl, Bluepl;

int tempBoard[SIZE][SIZE] = { 0 }; //白1,黑-1
int BlueSide1[SIZE][SIZE] = { 0 }, BlueSide2[SIZE][SIZE] = { 0 }, RedSide1[SIZE][SIZE] = { 0 }, RedSide2[SIZE][SIZE] = { 0 }; //双距离
int vis[SIZE][SIZE] = { 0 }, FloodVis[SIZE][SIZE] = { 0 };

// 扩展时优化
vector<Coord> Choose;
queue<Coord> upperRed, lowerRed, upperBlue, lowerBlue;

// 第一个是要下的位置，第2-3个是可以是自己的位置
int uRedX1[5] = {-1, 0,-2,-1,-2}; // x = 2
int uRedY1[5] = {-1,-1,-1, 0, 0};
int uRedX2[5] = {-1, 0,-2,-1,-2}; 
int uRedY2[5] = { 2, 1, 3, 1, 2};
int lRedX1[5] = { 1, 0, 2, 1, 2}; // x = 8
int lRedY1[5] = { 1, 1, 1, 0, 0}; 
int lRedX2[5] = { 1, 0, 2, 1, 2};
int lRedY2[5] = {-2,-1,-3,-1,-2};
int uBlueX1[5] = {-1,-1,-1, 0, 0}; // y = 2
int uBlueY1[5] = {-1,-2, 0,-1,-2};
int uBlueX2[5] = { 2, 3, 1, 1, 2};
int uBlueY2[5] = {-1,-2, 0,-1,-2};
int lBlueX1[5] = {-2,-3,-1,-1,-2}; // y = 8
int lBlueY1[5] = { 1, 2, 0, 1, 2};
int lBlueX2[5] = { 1, 1, 1, 0, 0};
int lBlueY2[5] = { 1, 0, 2, 1, 2};
/* End Part1 */

/* Part2 函数声明 */
int stepX[6] = { -1,-1, 0, 1, 1 , 0 };
int stepY[6] = { 0, 1, 1, 0,-1 ,-1 };
int bridgeX1[6] = { -1,-1,-1, 0, 0,-1 };
int bridgeY1[6] = { 1, 0, 1, 1,-1, 0 };
int bridgeX2[6] = { 0, 0, 1, 1, 1, 1 };
int bridgeY2[6] = { -1, 1, 0,-1, 0,-1 };

bool jdg1(int x, int y);

bool jdg2(int x, int y);

bool jdg3(int x, int y);

bool jdg4(int x, int y);

void Flood(int x, int y, int s, int mark);

void UpdateAdj(int tx, int ty, int mark);

void Capture(int lastX, int lastY, int curPl);

bool Invalid(int curX, int curY);

void judge(int x, int y, int curPl);

void Edge(int curPl);

void Calc_Potential();

int get_fa(int x);

int TrytoMerge(int x, int y, int curPl);

void MCTS(int lstX, int lstY);

/* End Part2 */

/* Part3 搜素树的节点 */

class TreeNode {

public:

	double val;// 当前节点的评价值
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
			double ucb = child->val * 1.0 / child->n + uct * sqrt(2 * log(n) * 1.0 / child->n); //ucb
			if (ucb > score) {
				score = ucb;
				best = child;
			}

		}
		//cout <<"Bestval:" << best->move_x<< " "<<best->move_y << endl;
		return best;
	}

	TreeNode* Expand() {

		/* 初始化 */
		int update[SIZE][SIZE] = { 0 };
		TreeNode* ChosenChild = nullptr;
		Choose.clear();
		/* 向搜素树添加节点 */

		// 找出无效点

		/*
		for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end();) {
			if (Invalid(iter->x, iter->y) == 1) {
				// 测试
				// curBoard[iter->x][iter->y] = 0;
				// 从UntriedMove中删除, 保证该点不被Expand
				iter = UntriedMoves.erase(iter);
			}
			else  iter++;
		}
		*/

		/* 找出被捕获的点 */

		Capture(move_y, move_x, -player);

		/* 计算并找出潜力值较高的几个点 */

		Calc_Potential();

		/* 只考虑潜力值较高和被捕获的点 */

		// 选一个点用作扩展
		int c = (rand() % Choose.size());
		auto iter = Choose.begin() + c;
		// 防止该坐标被重复扩展
		update[iter->x][iter->y] = 1;
		// 将其加入当前节点的Children
		ChosenChild = new TreeNode(iter->x, iter->y, -player, this); // 子节点玩家对当前玩家取反
		children.push_back(ChosenChild);
		// 从UntriedMove中删点（这个被提前到这里, 是因为这里有iter）
		for (auto it = UntriedMoves.begin(); it != UntriedMoves.end(); it++) {
			if (it->x == iter->x && it->y == iter->y) {
				UntriedMoves.erase(it);
				break;
			}
		}

		for (auto iter = Choose.begin(); iter != Choose.end(); iter++) {
			if (update[iter->x][iter->y] >= 1) {
				update[iter->x][iter->y]++;
				continue; // 防止同一个坐标被当作两个点
			}
			update[iter->x][iter->y] = 1;
			TreeNode* child = new TreeNode(iter->x, iter->y, -player, this); // 子节点玩家对当前玩家取反
			children.push_back(child);
		}

		// cout << "Expand:" << move_x << " " << move_y << "  Size:" << children.size() << endl;

		return ChosenChild;
	}

	double Simulate() {

		// 直接使用 UntriedMoves 和 curBoard 即可
		// 把当前节点的状态作为起始状态
		int lastX = move_x, lastY = move_y, lastplayer = player;

		while (1) {

			/* 判断胜负 */
			if (TrytoMerge(lastX, lastY, lastplayer) != 0) {
				return lastplayer * player; // 相同为1, 相异为-1
			}

			/* 更新棋盘状态 */



		    
			// 纯随机

			int c = (rand() % UntriedMoves.size());
			auto iter = UntriedMoves.begin() + c;
			// 更新 lastX， lastY
			lastX = iter->x, lastY = iter->y;
			// cout << "go:" << lastX << " " << lastY << endl;
			// 更新curboard
			curBoard[iter->x][iter->y] = -lastplayer;
			// 从UntriedMove中删点
			UntriedMoves.erase(iter);
			// 更新lastplayer
			lastplayer = -lastplayer;

		}

	}

	void BackPropagate(double re) { // 更新搜素树中的值
		TreeNode* node = this;
		while (node != nullptr) {
			node->n += 1;
			if (re == 1) node->val += re;
			re = -re;
			node = node->father;
		}
	}

};

/* End Part3 */

/* Part4 主函数 */

int main() {

	// start = chrono::steady_clock::now(); // 获取开始时间

	int x, y, n;
	//恢复目前的棋盘信息
	// scanf("%d", &n);
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
		// scanf("%d%d", &x, &y);
		cin >> x >> y;
		if (x != -1) board[x][y] = -1;	//对方
		if (i == 0) {
			if (x == -1) {
				mycolor = -1;  // 我先手, 我是红色
			}
			else mycolor = 1; // 对方先手, 我是蓝色
		}

		if (x != -1) judge(x, y, -1); // 边缘特判

		//scanf("%d%d", &x, &y);
		cin >> x >> y;
		if (x != -1) board[x][y] = 1;	//我方

		if (x != -1) judge(x, y, 1); // 边缘特判

	}
	//scanf("%d%d", &x, &y);
	cin >> x >> y;

	if (x != -1) board[x][y] = -1;	//对方

	if (x != -1) judge(x, y, -1); // 边缘特判

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

	MCTS(x, y); // 蒙特卡洛树搜索

	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果0
	cout << new_x << ' ' << new_y << endl;

	return 0;

}

/* End Part4 */

/* Part5 函数实现 */

int get_fa(int x) {
	if (fa[x] == x) return x;
	return fa[x] = get_fa(fa[x]);
}

int TrytoMerge(int x, int y, int curPl) { // 尝试把（x,y）与其相邻点合并 | 返回1表示游戏结束

	if (x == 0 && ((mycolor == -1 && curPl == 1) || (mycolor == 1 && curPl == -1))) { // 0 边
		fa[get_fa(200)] = get_fa(x * 11 + y);
	}
	if (x == 10 && ((mycolor == -1 && curPl == 1) || (mycolor == 1 && curPl == -1))) { // 1 边
		fa[get_fa(201)] = get_fa(x * 11 + y);
	}
	if (y == 0 && ((mycolor == -1 && curPl == -1) || (mycolor == 1 && curPl == 1))) { // 2 边
		fa[get_fa(202)] = get_fa(x * 11 + y);
	}
	if (y == 10 && ((mycolor == -1 && curPl == -1) || (mycolor == 1 && curPl == 1))) { // 3 边
		fa[get_fa(203)] = get_fa(x * 11 + y);
	}

	for (int i = 0; i < 6; i++) {
		int tmpX = x + stepX[i], tmpY = y + stepY[i];
		if (tmpX < 0 || tmpY < 0 || tmpX > 10 || tmpY > 10) continue; // 排除不合法位置
		if (curBoard[tmpX][tmpY] != curPl) continue; // 排除对手棋子 和 空位置
		fa[get_fa(tmpX * 11 + tmpY)] = get_fa(x * 11 + y);
	}

	if (get_fa(200) == get_fa(201) || get_fa(202) == get_fa(203)) {
		return 1;
	}

	return 0;
}

void MCTS(int lstX, int lstY) {

	auto start = chrono::steady_clock::now(); // 获取开始时间

	TreeNode* root = new TreeNode(lstX, lstY, -1, nullptr); // 初始化根节点（根节点的玩家记为对方）

	int ctr = 0;

	while (1) {

		ctr++;

		//中途改参数
		if (ctr > 5) {
			ExpNum = 30, Tolerant = 3;
		}

		if (ctr == 1000) {
			printf("1");
			ctr = 0;
		}
		/* 计时 */
		auto stops = chrono::steady_clock::now();

		if (chrono::duration_cast<chrono::milliseconds>(stops - start).count() >= 825) { // 提前1ms结束
			break;
		}

		/* 预处理 */

		// 一个标记
		int alreadyWin = 0;

		// curBoard: 用来记录待扩展节点所对应的状态 随Select的进行而更新 并在Simulate时使用
		for (int i = 0; i < 11; i++) {
			for (int j = 0; j < 11; j++) {
				curBoard[i][j] = board[i][j];
			}
		}

		// 初始化并查集
		for (int i = 0; i < 11 * 11; i++) fa[i] = i;
		for (int i = 200; i < 204; i++) fa[i] = i;

		for (int i = 0; i < 11; i++) {
			for (int j = 0; j < 11; j++) {
				// cout << mycolor << " " << i << " " << j <<" " << curBoard[i][j] << endl;
				if (curBoard[i][j] != 0) TrytoMerge(i, j, curBoard[i][j]);
			}
		}


		//	memcpy(curBoard, board, sizeof(board));

		/* Select部分 */

		// cout << "Selection"<<endl;

		TreeNode* node = root;

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
			// 这里不用更新untriedmoves因为untriedmoves还没初始化
		}

		// 这种情况是: 还没模拟就已经赢了
		if (alreadyWin == 1) {
			// cout << " Win After Select" << endl;
			continue;
		}

		if (node == nullptr) { // 某次Select返回的是nullptr 理论上来说不可能
			cout << 2333 << endl;
			return;
		}

		/* Expand 部分 */

		// 初始化UntriedMoves 
		UntriedMoves.clear();
		for (int i = 0; i <= 10; i++)
			for (int j = 0; j <= 10; j++)
				if (curBoard[i][j] == 0) UntriedMoves.push_back(Coord(i, j));

		if (node->n >= counter) {
			// 只有一个节点被模拟了足够的次数时, 才进行扩展
			node = node->Expand();
			// cout << "Expand Node:" << node->move_x <<" " << node->move_y << endl;
			if (node == nullptr) { // 这个叶节点扩展不出来一个节点, 也就是说, 该节点表示的状态是一个摆满棋子的棋盘
				cout << 23333 << endl;
				return;
			}

			// 更新这个点的curboard （这俩被拖到这里是因为要判断胜负）
			curBoard[node->move_x][node->move_y] = node->player;
			// 更新并查集
			if (TrytoMerge(node->move_x, node->move_y, node->player) != 0) {
				node->BackPropagate(1);
				alreadyWin = 1;
			}
		}

		// 这种情况是: 还没模拟就已经赢了
		if (alreadyWin == 1) {
			// cout << " Win After Expand" << endl;
			continue;
		}
		/* Simulate 部分 */

		// cout << "Simulate" << endl;

		double result = node->Simulate();

		/* BackPropagate 部分 */

		// cout << "BP" << endl;

		node->BackPropagate(result);

	}

	TreeNode* BestChild = nullptr;
	double MaxRate = -1919810.0;
	for (int i = 0; i < root->children.size(); i++) {
		if (double(double(root->children[i]->val) / double(root->children[i]->n)) > MaxRate) {
			MaxRate = double(double(root->children[i]->val) / double(root->children[i]->n));
			BestChild = root->children[i];
		}
	}
	if (BestChild != nullptr) {
		new_x = BestChild->move_x;
		new_y = BestChild->move_y;
	}
	else { // 什么都没找到 ?
		cout << 233 << endl;
		return;
	}

	return;
}

void judge(int x, int y, int curPl) {
	if (((mycolor == 1 && curPl == -1) || (mycolor == -1 && curPl == 1)) && x == 2) { upperRed.push(Coord(x, y)); return; };
	if (((mycolor == 1 && curPl == -1) || (mycolor == -1 && curPl == 1)) && x == 8) { lowerRed.push(Coord(x, y)); return; };
	if (((mycolor == -1 && curPl == -1) || (mycolor == 1 && curPl == 1)) && y == 2) { upperBlue.push(Coord(x, y)); return; };
	if (((mycolor == -1 && curPl == -1) || (mycolor == 1 && curPl == 1)) && y == 8) { lowerBlue.push(Coord(x, y)); return; };
}

void Edge(int curPl) {
	Coord tar;
	// Red Blue
	if ((curPl == 1 && mycolor == -1) || (curPl == -1 && mycolor == 1)) {
		while (1) { 
			if(!upperRed.empty()){
				int x = upperRed.front().x, y = upperRed.front().y, mark = 1; // mark = 1 表示符合判断
				upperRed.pop(); // 选出一个边缘第三行的点
				tar.x = x + uRedX1[0], tar.y = y + uRedY1[0];
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + uRedX1[i], tmpY = y + uRedY1[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
				tar.x = x + uRedX2[0], tar.y = y + uRedY2[0], mark = 1;
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + uRedX2[i], tmpY = y + uRedY2[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
			}
			if (!lowerRed.empty()) {
				int x = lowerRed.front().x, y = lowerRed.front().y, mark = 1; // mark = 1 表示符合判断
				lowerRed.pop(); // 选出一个边缘第三行的点
				tar.x = x + lRedX1[0], tar.y = y + lRedY1[0];
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + lRedX1[i], tmpY = y + lRedY1[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
				tar.x = x + lRedX2[0], tar.y = y + lRedY2[0], mark = 1;
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + lRedX2[i], tmpY = y + lRedY2[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
			}
		}
	}
	if ((curPl == 1 && mycolor == 1) || (curPl == -1 && mycolor == -1)) {
		while (1) {
			if (!upperBlue.empty()) {
				int x = upperBlue.front().x, y = upperBlue.front().y, mark = 1; // mark = 1 表示符合判断
				upperBlue.pop(); // 选出一个边缘第三行的点
				tar.x = x + uBlueX1[0], tar.y = y + uBlueY1[0];
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + uBlueX1[i], tmpY = y + uBlueY1[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
				tar.x = x + uBlueX2[0], tar.y = y + uBlueY2[0], mark = 1;
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + uBlueX2[i], tmpY = y + uBlueY2[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
			}
			if (!lowerBlue.empty()) {
				int x = lowerBlue.front().x, y = lowerBlue.front().y, mark = 1; // mark = 1 表示符合判断
				lowerBlue.pop(); // 选出一个边缘第三行的点
				tar.x = x + lBlueX1[0], tar.y = y + lBlueY1[0];
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + lBlueX1[i], tmpY = y + lBlueY1[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
				tar.x = x + lBlueX2[0], tar.y = y + lBlueY2[0], mark = 1;
				if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
				for (int i = 1; i <= 4; i++) {
					int tmpX = x + lBlueX2[i], tmpY = y + lBlueY2[i];
					if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
					if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
					if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
				}
				if (mark == 1) {
					Choose.push_back(tar); // 找到了立刻返回
					return;
				}
			}
		}
	}
}

// 被捕获的位置 负责模拟时优化
// 更新后能够捕获边缘的点
void Capture(int lastX, int lastY, int curPl) { // 传入上一步棋的状态, 判断上一步棋是否导致一个点被捕获

	int captureX, captureY, b1X, b1Y, b2X, b2Y;
	for (int i = 0; i < 6; i++) {
		captureX = lastX + stepX[i], captureY = lastY + stepY[i];
		b1X = lastX + bridgeX1[i], b1Y = lastY + bridgeY1[i];
		b2X = lastX + bridgeX2[i], b2Y = lastY + bridgeY2[i];
		if (captureX < 0 || captureY < 0 || captureX > 10 || captureY > 10) continue; // CaptureX ， CaptureY 不能超出范围
		int ctrRed = 0, ctrBlue = 0;
		if (b1X < 0 || b2X < 0 || b1X > 10 || b2X > 10) ctrRed += 1; // 有红边参与
		if (b1Y < 0 || b2Y < 0 || b1Y > 10 || b2Y > 10) ctrBlue += 1; // 有蓝边参与
		if (b1X >= 0 && b1Y >= 0 && b1X <= 10 && b1Y <= 10) {
			if (curBoard[b1X][b1Y] == -mycolor)  ctrRed += 1; // mycolor -1 时表示我方是红色
			else ctrBlue += 1;
		}
		if (b2X >= 0 && b2Y >= 0 && b2X <= 10 && b2Y <= 10) {
			if (curBoard[b1X][b1Y] == -mycolor)  ctrRed += 1;
			else ctrBlue += 1;
		}
		if (curPl == 1 && ((mycolor == -1 && ctrRed == 2) || (mycolor == 1 && ctrBlue == 2))) Choose.push_back(Coord(captureX, captureY));
		if (curPl == -1 && ((mycolor == 1 && ctrRed == 2) || (mycolor == -1 && ctrBlue == 2))) Choose.push_back(Coord(captureX, captureY));
	}
	return;
}

// 返回1 或 -1说明（curX, curY）是无效位置（可以直接用返回值填充）
// 已加入边缘特判
bool Invalid(int curX, int curY) {
	int ctr1 = 0, ctr2 = 0;
	for (int j = 0; j <= 12; j++) {
		int i = j % 6;
		int tmpX = curX + stepX[i], tmpY = curY + stepY[i];
		// 更新
		if (tmpX < 0 || tmpX > 10) {
			if (mycolor == -1) { // 如果我是红色
				ctr1++;
				ctr2 = 0;
			}
			else {
				ctr2++;
				ctr1 = 0;
			}
		}
		else if (tmpY < 0 || tmpY > 10) {
			if (mycolor == 1) { // 如果我是蓝色
				ctr1++;
				ctr2 = 0;
			}
			else {
				ctr2++;
				ctr1 = 0;
			}
		}
		else if (curBoard[tmpX][tmpY] == 0) {
			ctr1 = 0, ctr2 = 0;
		}
		else if (curBoard[tmpX][tmpY] == 1) {
			ctr1++;
			ctr2 = 0;
		}
		else if (curBoard[tmpX][tmpY] == -1) {
			ctr2++;
			ctr1 = 0;
		}

		// 判定1
		if (ctr1 == 4) {
			return 1;
		}
		else if (ctr1 == 3) {
			int tx = curX + stepX[(i + 2) % 6], ty = curY + stepY[(i + 2) % 6];
			if ((mycolor == -1 && (ty < 0 || ty > 10)) || (mycolor == 1 && (tx < 0 || tx > 10))) return 1; //边缘特判 且成功
			else if (tx < 0 || tx > 10 || ty < 0 || ty > 10) return 0; // 边缘特判 失败
			if (curBoard[tx][ty] == -1) return 1; //正常
		}
		else if (ctr1 == 2) {
			int tx1 = curX + stepX[(i + 2) % 6], ty1 = curY + stepY[(i + 2) % 6];
			int tx2 = curX + stepX[(i + 3) % 6], ty2 = curY + stepY[(i + 3) % 6];
			int ctr = 0;
			if ((mycolor == -1 && (ty1 < 0 || ty1 > 10)) || (mycolor == 1 && (tx1 < 0 || tx1 > 10))) ctr++;
			else if (tx1 < 0 || tx1 > 10 || ty1 < 0 || ty1 > 10) return 0; // 边缘特判 失败
			else if (curBoard[tx1][ty1] == -1) ctr++;
			if ((mycolor == -1 && (ty2 < 0 || ty2 > 10)) || (mycolor == 1 && (tx2 < 0 || tx2 > 10))) ctr++; //边缘特判 且成功
			else if (tx2 < 0 || tx2 > 10 || ty2 < 0 || ty2 > 10) return 0; // 边缘特判 失败
			else if (curBoard[tx2][ty2] == -1) ctr++;
			if (ctr == 2)return 1;
			return 0;
		}
		// 判定2
		if (ctr2 == 4) {
			return 1;
		}
		else if (ctr2 == 3) {
			int tx = curX + stepX[(i + 2) % 6], ty = curY + stepY[(i + 2) % 6];
			if (curBoard[tx][ty] == 1) return -1;
		}
		else if (ctr2 == 2) {
			int tx1 = curX + stepX[(i + 2) % 6], ty1 = curY + stepY[(i + 2) % 6];
			int tx2 = curX + stepX[(i + 3) % 6], ty2 = curY + stepY[(i + 3) % 6];
			int ctr = 0;
			if ((mycolor == 1 && (ty1 < 0 || ty1 > 10)) || (mycolor == -1 && (tx1 < 0 || tx1 > 10))) ctr++;
			else if (tx1 < 0 || tx1 > 10 || ty1 < 0 || ty1 > 10) return 0; // 边缘特判 失败
			else if (curBoard[tx1][ty1] == 1) ctr++;
			if ((mycolor == 1 && (ty2 < 0 || ty2 > 10)) || (mycolor == -1 && (tx2 < 0 || tx2 > 10))) ctr++; //边缘特判 且成功
			else if (tx2 < 0 || tx2 > 10 || ty2 < 0 || ty2 > 10) return 0; // 边缘特判 失败
			else if (curBoard[tx2][ty2] == 1) ctr++;
			if (ctr == 2) return -1;
		}
	}
	return 0; // 返回 1 或 -1 都是无效点 而返回 0 不是无效点
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

void Calc_Potential() { // 计算双威胁值 用到CurBoard

	/* 初始化 */
	memset(BlueSide1, 0x3f, sizeof(BlueSide1));
	memset(BlueSide2, 0x3f, sizeof(BlueSide2));
	memset(RedSide1, 0x3f, sizeof(RedSide1));
	memset(RedSide2, 0x3f, sizeof(RedSide2));

	// 一个新棋盘 保证 红色为1 蓝色为-1 并且下标范围为1-11, 而 0 和 12 行 是边的范围
	for (int i = 0; i <= 10; i++) {
		for (int j = 0; j <= 10; j++) {
			if (mycolor == -1) {
				tempBoard[i + 1][j + 1] = curBoard[i][j];
			}
			else tempBoard[i + 1][j + 1] = -curBoard[i][j];
		}
	}

	/* 测试 */

	/* Part1 预处理上红边 */

	// 初始化vis 初始化对边
	memset(vis, 0, sizeof(vis));
	for (int i = 1; i <= 11; i++) {
		tempBoard[11 + 1][i] = 1;
	}

	// 预处理首行
	for (int i = 1; i <= 11; i++) {
		vis[1][i] = 1;
		if (tempBoard[1][i] == 0) { RedSide1[1][i] = 1; } //初始化双距离和机动性 //
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

	// 清空Adj
	for (int i = 1; i <= 11; i++) {
		for (int j = 1; j <= 11; j++) {
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
		if (tempBoard[11][i] == 0) { RedSide2[11][i] = 1; } //初始化双距离和机动性 //
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
		if (tempBoard[i][1] == 0) { BlueSide1[i][1] = 1; } //初始化双距离和机动性 //
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

	// 清空Adj
	for (int i = 1; i <= 11; i++) {
		for (int j = 1; j <= 11; j++) {
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
		if (tempBoard[i][11] == 0) { BlueSide2[i][11] = 1; } //初始化双距离和机动性 //
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

	// 清空Adj
	for (int i = 1; i <= 11; i++) {
		for (int j = 1; j <= 11; j++) {
			while (!Adj[i][j].empty()) Adj[i][j].pop(); // 清空Adj
		}
	}

	// 恢复对边
	for (int i = 1; i <= 11; i++) {
		tempBoard[i][0] = 0;
	}

	for (int i = 0; i < UntriedMoves.size(); i++) {
		int tx = UntriedMoves[i].x + 1, ty = UntriedMoves[i].y + 1;
		Redpl.push(Cmp(RedSide1[tx][ty] + RedSide2[tx][ty], tx, ty));
		Bluepl.push(Cmp(BlueSide1[tx][ty] + BlueSide2[tx][ty], tx, ty));
	}
	/*
	cout << "mycolor: " << mycolor <<endl;
	cout << "Red1" << endl;
	for (int i = 1; i <= 11; i++) {
		for (int k = 1; k < i; k++) {
			cout << "  ";
		}
		for (int j = 1; j <= 11; j++) {
			cout << setw(5) << RedSide1[i][j];
		}
		cout << endl;
	}
	cout << "Red2" << endl;
	for (int i = 1; i <= 11; i++) {
		for (int k = 1; k < i; k++) {
			cout << "  ";
		}
		for (int j = 1; j <= 11; j++) {
			cout << setw(5) << RedSide2[i][j];
		}
		cout << endl;
	}
	cout << "Blue1" << endl;
	for (int i = 1; i <= 11; i++) {
		for (int k = 1; k < i; k++) {
			cout << "  ";
		}
		for (int j = 1; j <= 11; j++) {
			cout << setw(5) << BlueSide1[i][j];
		}
		cout << endl;
	}
	cout << "Blue2" << endl;
	for (int i = 1; i <= 11; i++) {
		for (int k = 1; k < i; k++) {
			cout << "  ";
		}
		for (int j = 1; j <= 11; j++) {
			cout << setw(5) << BlueSide2[i][j];
		}
		cout << endl;
	}
	*/

	/* 更新 Choose */

	int val1 = -1, val2 = -1;
	for (int i = 1; (!Redpl.empty()) && i <= ExpNum; i++) {
		if (val1 == -1) {
			Choose.push_back(Coord(Redpl.top().x - 1, Redpl.top().y - 1)); //记得恢复原来的坐标(0-10)
			Redpl.pop();
		}
		else if (Redpl.top().pot <= val1 + Tolerant) {
			Choose.push_back(Coord(Redpl.top().x - 1, Redpl.top().y - 1)); //记得恢复原来的坐标(0-10)
			Redpl.pop();
		}
		else break;
	}
	for (int i = 1; (!Bluepl.empty()) && i <= ExpNum; i++) {
		if (val2 == -1) {
			Choose.push_back(Coord(Bluepl.top().x - 1, Bluepl.top().y - 1));
			Bluepl.pop();
		}
		else if (Bluepl.top().pot <= val1 + Tolerant) {
			Choose.push_back(Coord(Bluepl.top().x - 1, Bluepl.top().y - 1));
			Bluepl.pop();
		}
		else break;
	}

	/* 清空Redpl 和 Bluepl */
	while (!Redpl.empty()) {
		Redpl.pop();
	}
	while (!Bluepl.empty()) {
		Bluepl.pop();
	}

}


/* End Part5 */



	/* Simulate 中的检查
			for (int ii = 0; ii < 11; ii++) {
				for (int k = 0; k < ii; k++) {
					cout << "  ";
				}
				for (int jj = 0; jj < 11; jj++) {
					cout << setw(5) << get_fa(ii * 11 + jj);
				}
				cout << endl;
			}

			cout << "fa[200]=" << get_fa(200) / 11 << " " << get_fa(200) % 11 << endl;
			cout << "fa[201]=" << get_fa(201) / 11 << " " << get_fa(201) % 11 << endl;
			cout << "fa[202]=" << get_fa(202) / 11 << " " << get_fa(202) % 11 << endl;
			cout << "fa[203]=" << get_fa(203) / 11 << " " << get_fa(203) % 11 << endl;
			for (int i = 0; i < 11; i++) {
				for (int k = 0; k < i; k++) {
					cout << "  ";
				}
				for (int j = 0; j < 11; j++) {
					cout << setw(5) << curBoard[i][j];
				}
				cout << endl;
			}
			*/

