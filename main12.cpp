#define _CRT_SECURE_NO_WARNINGS
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
int counter = 30, launch = 100;
double uct = 0.3;
int time_ms = 970;

// 记录棋盘信息
int chessround; // 本程序开始时，处于第几轮
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

// 扩展时优化
int ctr = 0; // 记录MCTS次数
vector<Coord> Choose;
queue<Coord> upperRed, lowerRed, upperBlue, lowerBlue;

// 计时
std::chrono::steady_clock::time_point start, stops;

/* End Part1 */

/* Part2 函数声明 */
// 第一个是要下的位置，第2-3个是可以是自己的位置
int uRedX1[5] = { -1, 0,-2,-1,-2 }; // x = 2
int uRedY1[5] = { -1,-1,-1, 0, 0 };
int uRedX2[5] = { -1, 0,-2,-1,-2 };
int uRedY2[5] = { 2, 1, 3, 1, 2 };
int lRedX1[5] = { 1, 0, 2, 1, 2 }; // x = 8
int lRedY1[5] = { 1, 1, 1, 0, 0 };
int lRedX2[5] = { 1, 0, 2, 1, 2 };
int lRedY2[5] = { -2,-1,-3,-1,-2 };
int uBlueX1[5] = { -1,-1,-1, 0, 0 }; // y = 2
int uBlueY1[5] = { -1,-2, 0,-1,-2 };
int uBlueX2[5] = { 2, 3, 1, 1, 2 };
int uBlueY2[5] = { -1,-2, 0,-1,-2 };
int lBlueX1[5] = { -2,-3,-1,-1,-2 }; // y = 8
int lBlueY1[5] = { 1, 2, 0, 1, 2 };
int lBlueX2[5] = { 1, 1, 1, 0, 0 };
int lBlueY2[5] = { 1, 0, 2, 1, 2 };


int stepX[6] = { -1,-1, 0, 1, 1 , 0 };
int stepY[6] = { 0, 1, 1, 0,-1 ,-1 };
int bridgeX1[6] = { -1,-1,-1, 0, 0,-1 };
int bridgeY1[6] = { 1, 0, 1, 1,-1, 0 };
int bridgeX2[6] = { 0, 0, 1, 1, 1, 1 };
int bridgeY2[6] = { -1, 1, 0,-1, 0,-1 };


int BX[6] = { -1,-2,-1, 1, 2, 1 };
int BY[6] = { -1, 1, 2, 1,-1,-2 };
int ex1[6] = { 0,-1,-1, 0, 1, 1 };
int ey1[6] = { -1, 0, 1, 1, 0,-1 };
int ex2[6] = { -1,-1, 0, 1, 1, 0 };
int ey2[6] = { 0, 1, 1, 0,-1,-1 };

int jud1(int x, int y, int curPl);

int jud2(int x, int y, int curPl);

int jud3(int x, int y, int curPl);

int jud4(int x, int y, int curPl);

void init_node(int* n, int* val, int mine, int yours);

void init_connect(int* n, int* val, int mine, int yours);

void init_edge(int* n, int* val, int x, int y, int curPl);

void Capture(int lastX, int lastY, int curPl);

bool Invalid(int curX, int curY);

void Edge(int curPl);

void judge(int x, int y, int curPl);

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

	TreeNode(int pos_x = -1, int pos_y = -1, int pl = 0, TreeNode* fa = nullptr, int nn = 0, int vv = 0) {
		val = vv;
		n = nn;
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

		/* 尝试给每个树节点一个初始权值 */
		Choose.clear();
		Capture(move_x, move_y, -player);

		for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end(); iter++) {

			if (curBoard[iter->x][iter->y] != 0) { // 恢复二者之间的一致对应
				iter = UntriedMoves.erase(iter);
				if (iter == UntriedMoves.end())break;
			}

			int mine = 0, yours = 0, init_n = 0, init_val = 0, mine_c = 0, yours_c = 0;
			vector<int> mine_rec, yours_rec;

			for (int i = 0; i < Choose.size(); i++) { // 捕获点
				if (iter->x == Choose[i].x && iter->y == Choose[i].y) {
					init_n = 8, init_val = 10;
				}
			}

			for (int i = 0; i < 6; i++) { // 形成桥
				int tx = iter->x + BX[i], ty = iter->y + BY[i];
				int tx1 = iter->x + ex1[i], ty1 = iter->y + ey1[i];
				int tx2 = iter->x + ex2[i], ty2 = iter->y + ey2[i];
				if (tx < 0 || tx1 < 0 || tx2 < 0 || ty < 0 || ty1 < 0 || ty2 < 0 || tx > 10 || tx1 > 10 || tx2 > 10 || ty > 10 || ty1 > 10 || ty2 > 10) continue;
				if (curBoard[tx1][ty1] == 0 && curBoard[tx2][ty2] == 0) {
					if (curBoard[tx][ty] == -player) mine++;
					if (curBoard[tx][ty] == player) yours++;
				}
			}

			if (chessround > launch) {
				for (int i = 0; i < 6; i++) { // 连接点
					int tx = iter->x + stepX[i], ty = iter->y + stepY[i];
					if (curBoard[tx][ty] == -player) {
						int mark = 1, tmpfa = get_fa(tx * 11 + ty);
						for (int j = 0; j < mine_rec.size(); j++) {
							if (mine_rec[j] == tmpfa) {
								mark = 0;
								break;
							}
						}
						if (mark == 1) mine_rec.push_back(tmpfa);
					}
					if (curBoard[tx][ty] == player) {
						int mark = 1, tmpfa = get_fa(tx * 11 + ty);
						for (int j = 0; j < yours_rec.size(); j++) {
							if (yours_rec[j] == tmpfa) {
								mark = 0;
								break;
							}
						}
						if (mark == 1) yours_rec.push_back(tmpfa);
					}
				}
			}

			// 采用什么策略更好？
			init_node(&init_n, &init_val, mine, yours);
			init_connect(&init_n, &init_val, mine_rec.size(), yours_rec.size());
			init_edge(&init_n, &init_val, iter->x, iter->y, -player);

			TreeNode* child = new TreeNode(iter->x, iter->y, -player, this, init_n, init_val); // 子节点玩家对当前玩家取反
			children.push_back(child);

		}

		/* 随机选点扩展 */

		TreeNode* ChosenChild = children[rand() % children.size()];

		return ChosenChild;

	}

	double Simulate() {

		// 直接使用 UntriedMoves 和 curBoard 即可
		// 把当前节点的状态作为起始状态
		int lastX = move_x, lastY = move_y, lastplayer = player;

		while (1) { // 尝试极简

			/* 判断胜负 */

			if (TrytoMerge(lastX, lastY, lastplayer) != 0) {
				return lastplayer * player; // 相同为1, 相异为-1
			}

			/* 模拟时优化 */

			Choose.clear();

			Capture(lastX, lastY, -lastplayer); // 捕获的点
			int cx = -100, cy = -100;
			if (!Choose.empty()) {
				int c = (rand() % Choose.size());
				cx = Choose[c].x, cy = Choose[c].y;
			}
			else {
				Edge(-lastplayer);
				if (!Choose.empty()) {
					int c = (rand() % Choose.size());
					cx = Choose[c].x, cy = Choose[c].y;
				}
			}

			/* 判断能否使用优化 */

			if (cx != -100) { // 可以优化

				// cout << 'y' << endl;
				auto iter = UntriedMoves.begin();
				int l = 0, r = UntriedMoves.size() - 1;
				while (1) {
					// 
					int mid = (l + r) / 2;
					// cout << UntriedMoves.begin()->x * 11 + UntriedMoves.begin()->y << " " << (UntriedMoves.end()-1)->x * 11 + (UntriedMoves.end()-1)->y << endl;
					// cout << UntriedMoves[mid].x << " " << UntriedMoves[mid].y << "    c: " << cx << " " << cy << " curboard: " <<curBoard[cx][cy]<< endl;
					if (UntriedMoves[mid].x * 11 + UntriedMoves[mid].y == cx * 11 + cy) {
						UntriedMoves.erase(iter + mid);
						break;
					}
					else if (UntriedMoves[mid].x * 11 + UntriedMoves[mid].y < cx * 11 + cy) {
						l = mid + 1;
					}
					else {
						r = mid - 1;
					}
				}

			}

			else {
				int c = (rand() % UntriedMoves.size());
				while (1) {
					auto iter = UntriedMoves.begin() + c;
					if (curBoard[iter->x][iter->y] != 0) { // 同步Invalid的修改（这种同步本身是异步的）
						iter = UntriedMoves.erase(iter);
						c = (rand() % UntriedMoves.size());
					}
					else {
						cx = iter->x, cy = iter->y;
						UntriedMoves.erase(iter);
						break;
					}
				}
			}

			/* 更新参数 */

			// 更新 lastX / lastY, curboard, lastplayer
			lastX = cx, lastY = cy;
			curBoard[cx][cy] = -lastplayer;
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

	start = chrono::steady_clock::now(); // 获取开始时间

	int x, y;
	//恢复目前的棋盘信息
	scanf("%d", &chessround);
	for (int i = 0; i < chessround - 1; i++) {
		scanf("%d %d", &x, &y);
		if (x != -1) board[x][y] = -1;	//对方
		if (i == 0) {
			if (x == -1) {
				mycolor = -1;  // 我先手, 我是红色
			}
			else mycolor = 1; // 对方先手, 我是蓝色
		}
		if (x != -1) judge(x, y, -1); // 边缘特判

		scanf("%d %d", &x, &y);
		if (x != -1) board[x][y] = 1;	//我方
		if (x != -1) judge(x, y, 1); // 边缘特判

	}
	scanf("%d %d", &x, &y);

	if (x != -1) board[x][y] = -1;	//对方

	if (chessround == 1) { // 我方先手 下固定位置
		if (x == -1) {
			printf("1 2 \n");
			return 0;
		}
		else {
			//printf("7 3 \n");
			//return 0;
			mycolor = 1;
		}
	}

	if (x != -1) judge(x, y, -1); // 边缘特判
	//此时board[][]里存储的就是当前棋盘的所有棋子信息 且轮到我方下棋

	//注意, 此时board[i][j] = 1表示此处是我方, 并非指此处是某个颜色

	/************************************************************************************/
	/***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/



	/* 初始化 */

	srand(time(0)); // 生成随机数种子

	/* 蒙特卡洛 */

	MCTS(x, y); // 蒙特卡洛树搜索

	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果0
	printf("%d %d \n", new_x, new_y);
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

	TreeNode* root = new TreeNode(lstX, lstY, -1, nullptr); // 初始化根节点（根节点的玩家记为对方）

	while (1) {

		ctr++;
		// if (ctr >= 1000) { ctr = 0; printf("1\n"); }

		stops = chrono::steady_clock::now();
		if (chrono::duration_cast<chrono::milliseconds>(stops - start).count() >= time_ms) { // 提前1ms结束
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

int jud1(int x, int y, int curPl) { // 上红
	Coord tar;
	int mark = 1;
	tar.x = x + uRedX1[0], tar.y = y + uRedY1[0];
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + uRedX1[i], tmpY = y + uRedY1[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
	}
	if (mark == 1) {
		Choose.push_back(tar); // 注意，这里是把答案放到choose里了，某些用不到choose的地方也不能忘了这点
		return 1; // 找到了立刻返回
	}
	tar.x = x + uRedX2[0], tar.y = y + uRedY2[0], mark = 1;
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + uRedX2[i], tmpY = y + uRedY2[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
	}
	if (mark == 1) {
		Choose.push_back(tar); // 找到了立刻返回
		return 1;
	}
	return 0;
}

int jud2(int x, int y, int curPl) { // 下红
	Coord tar;
	int mark = 1;
	tar.x = x + lRedX1[0], tar.y = y + lRedY1[0];
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + lRedX1[i], tmpY = y + lRedY1[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
	}
	if (mark == 1) {
		Choose.push_back(tar); // 找到了立刻返回
		return 1;
	}
	tar.x = x + lRedX2[0], tar.y = y + lRedY2[0], mark = 1;
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + lRedX2[i], tmpY = y + lRedY2[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
	}
	if (mark == 1) {
		Choose.push_back(tar); // 找到了立刻返回
		return 1;
	}
	return 0;
}

int jud3(int x, int y, int curPl) { // 上蓝
	Coord tar;
	int mark = 1;
	tar.x = x + uBlueX1[0], tar.y = y + uBlueY1[0];
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + uBlueX1[i], tmpY = y + uBlueY1[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
	}
	if (mark == 1) {
		Choose.push_back(tar); // 找到了立刻返回
		return 1;
	}
	tar.x = x + uBlueX2[0], tar.y = y + uBlueY2[0], mark = 1;
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + uBlueX2[i], tmpY = y + uBlueY2[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
	}
	if (mark == 1) {
		Choose.push_back(tar); // 找到了立刻返回
		return 1;
	}
	return 0;
}

int jud4(int x, int y, int curPl) { // 下蓝
	Coord tar;
	int mark = 1;
	tar.x = x + lBlueX1[0], tar.y = y + lBlueY1[0];
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + lBlueX1[i], tmpY = y + lBlueY1[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; } // 越界特判
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0; // 这两个点只要不是对面的点就行
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0; // 这两个点必须是空白的
	}
	if (mark == 1) {
		Choose.push_back(tar); // 找到了立刻返回
		return 1;
	}
	tar.x = x + lBlueX2[0], tar.y = y + lBlueY2[0], mark = 1;
	if (tar.x < 0 || tar.x > 10 || tar.y < 0 || tar.y > 10) mark = 0;
	else if (curBoard[tar.x][tar.y] != 0) mark = 0;
	for (int i = 1; i <= 4; i++) {
		int tmpX = x + lBlueX2[i], tmpY = y + lBlueY2[i];
		if (tmpX < 0 || tmpX > 10 || tmpY < 0 || tmpY > 10) { mark = 0; break; }
		if (i <= 2 && curBoard[tmpX][tmpY] == -curPl) mark = 0;
		if (i > 2 && curBoard[tmpX][tmpY] != 0) mark = 0;
	}
	if (mark == 1) {
		Choose.push_back(tar); // 找到了立刻返回
		return 1;
	}
	return 0;
}

void Edge(int curPl) {
	// Red Blue
	if ((curPl == 1 && mycolor == -1) || (curPl == -1 && mycolor == 1)) {
		while (1) {
			if (!upperRed.empty()) {
				int x = upperRed.front().x, y = upperRed.front().y; // mark = 1 表示符合判断
				upperRed.pop(); // 选出一个边缘第三行的点
				if (jud1(x, y, curPl))return;
			}
			if (!lowerRed.empty()) {
				int x = lowerRed.front().x, y = lowerRed.front().y, mark = 1; // mark = 1 表示符合判断
				lowerRed.pop(); // 选出一个边缘第三行的点
				if (jud2(x, y, curPl))return;
			}
			if (upperRed.empty() && lowerRed.empty()) break;
		}
	}
	if ((curPl == 1 && mycolor == 1) || (curPl == -1 && mycolor == -1)) {
		while (1) {
			if (!upperBlue.empty()) {
				int x = upperBlue.front().x, y = upperBlue.front().y, mark = 1; // mark = 1 表示符合判断
				upperBlue.pop(); // 选出一个边缘第三行的点
				if (jud3(x, y, curPl))return;
			}
			if (!lowerBlue.empty()) {
				int x = lowerBlue.front().x, y = lowerBlue.front().y, mark = 1; // mark = 1 表示符合判断
				lowerBlue.pop(); // 选出一个边缘第三行的点
				if (jud4(x, y, curPl))return;
			}
			if (upperBlue.empty() && lowerBlue.empty()) break;
		}
	}
}

void init_node(int* n, int* val, int mine, int yours) {
	if (*n != 0) return; // 被捕获的点另作处理
	*n = 0, * val = 0;
	if (mine != 0) {
		*n = 18;
		*val = 14 + (mine - 1) * 2;
	}
	if (yours != 0) {
		if (*n == 0) {
			*n = 18;
			*val = 8 + (yours - 1) * 2;
		}
		else {
			*n += 3;
			*val += 3;
		}
	}
}

void init_connect(int* n, int* val, int mine, int yours) {
	
	if (*n != 0) return; // 被捕获的点和桥优先级更高
	*n = 0, * val = 0;
	if (mine >= 2) {
		*n = 12;
		*val = 5;
	}
	/* 这个动作不够优
	if (yours >= 2) {
		if (*n == 0) {
			*n = 16;
			*val = 7;
		}
		else {
			*n += 2;
			*val += 2;
		}
	}
	*/

}

void init_edge(int* n, int* val, int x, int y, int curPl) { // 这个判定应该不涉及阻拦吧

	if (curPl * mycolor == -1) { // 红边
		if ((x == 2 && jud1(x, y, curPl)) || (x == 8 && jud2(x, y, curPl))) {
			if (*n != 0) { // 奖励分较高
				*val += 6;
				*n += 2;
			}
			else { // 基础分较低
				*val = 3;
				*n = 7; 
			}
		}
	}
	else if(curPl * mycolor == 1){ // 蓝边
		if ((y == 2 && jud3(x, y, curPl)) || (y == 8 && jud4(x, y, curPl))) {
			if (*n != 0) { // 奖励分较高
				*val += 6;
				*n += 2;
			}
			else { // 基础分较低
				*val = 3;
				*n = 7;
			}
		}
	}
	return;
}


// 被捕获的位置 负责模拟时优化
// 更新后能够捕获边缘的点
// 注意，现在的capture因为调用了invalid，导致了curboard和untriedmove之间的不一致
void Capture(int lastX, int lastY, int curPl) { // 传入上一步棋的状态, 判断上一步棋是否导致一个点被捕获

	// cout << lastX << " " << lastY << " " << curPl << endl;

	int captureX, captureY, b1X, b1Y, b2X, b2Y;
	for (int i = 0; i < 6; i++) {
		captureX = lastX + stepX[i], captureY = lastY + stepY[i];
		b1X = lastX + bridgeX1[i], b1Y = lastY + bridgeY1[i];
		b2X = lastX + bridgeX2[i], b2Y = lastY + bridgeY2[i];
		if (captureX < 0 || captureY < 0 || captureX > 10 || captureY > 10) continue; // CaptureX ， CaptureY 不能超出范围
		if (curBoard[captureX][captureY] != 0)continue; // 越界不考虑

		/* 无效点判定 */
		// 一般无效点都有一个生成的过程，每次遍历棋盘太慢了，不如将其内置于Capture之中，仅测试某一步是否形成无效点
		int val = Invalid(captureX, captureY); // 反正捕获点也是绕一周
		if (val != 0) { // 肯定是不下这里了
			curBoard[captureX][captureY] = val; //这里产生了不一致，要在调用之外消去
			// cout << captureX << " " << captureY << endl;
			continue;
		}

		/* 捕获点判定 */
		int ctrRed = 0, ctrBlue = 0;
		if (b1X < 0 || b2X < 0 || b1X > 10 || b2X > 10) ctrRed += 1; // 有红边参与
		if (b1Y < 0 || b2Y < 0 || b1Y > 10 || b2Y > 10) ctrBlue += 1; // 有蓝边参与
		if (b1X >= 0 && b1Y >= 0 && b1X <= 10 && b1Y <= 10) {
			if (curBoard[b1X][b1Y] == -mycolor)  ctrRed += 1; // mycolor -1 时表示我方是红色 1 时表示我是蓝色
			if (curBoard[b1X][b1Y] == mycolor)  ctrBlue += 1;
		}
		if (b2X >= 0 && b2Y >= 0 && b2X <= 10 && b2Y <= 10) {
			if (curBoard[b1X][b1Y] == -mycolor)  ctrRed += 1;
			if (curBoard[b2X][b2Y] == mycolor)  ctrBlue += 1;
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

