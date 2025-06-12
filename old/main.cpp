#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std;

const int SIZE = 15; // 意★义★不★明
int board[SIZE][SIZE] = { 0 }; //本方1，对方-1，空白0

int new_x, new_y; // 输出

// 下面是自己定义的变量

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

void Init() {

	// 初始化 EmptyGrid 把所有空节点加入
	for (int i = 0; i <= 10; i++) { 
		for (int j = 0; j <= 10; j++) {
			if (board[i][j] == 0) {
				EmptyGrid.push_back({ i,j }); // 奇怪的语法
			}
		}
	}

}

/*

shuffle

*/

class TreeNode {

private:

	double val;// 当前节点的评价值
	int player;// 这一步的玩家(-1,1)

public:

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
		TreeNode* ChosenChild = nullptr; 
		while (1) {
			if (UntriedMoves.empty())break; // 终止条件 说不定可以在此进行剪枝？
			int t = rand() % UntriedMoves.size(); 
			Coord move = UntriedMoves[t];
			UntriedMoves.erase(UntriedMoves.begin() + t); // 纯随机扩展
			
			TreeNode* child = new TreeNode(move.x, move.y, -player, this);
			children.push_back(child);

			bool Liu_Shun_zui_shuai = 1;
			if (Liu_Shun_zui_shuai == 1) { // 采取某种策略 从扩展的新节点中选出一个
				ChosenChild = child;
			}
		}
		return ChosenChild;
	}

	double Simulate() {
		int tempBoard[SIZE][SIZE];
		memcpy(tempBoard, board, sizeof(board));
		vector<Coord>tempEmpty = EmptyGrid;

		if (move_x != -1 && move_y != -1) {
			tempBoard[move_x][move_y] = player;
			tempEmpty.erase(remove_if(tempEmpty.begin(), tempEmpty.end(),
				[&](const Coord& re) {return re.x == move_x && re.y == move_y; }), tempEmpty.end());
		}

		int currPlayer = -player;
		while (!tempEmpty.empty()) {
			int r = rand() % tempEmpty.size();
			Coord move = tempEmpty[r];
			tempBoard[move.x][move.y] = currPlayer;
			tempEmpty.erase(tempEmpty.begin() + r);
			currPlayer = -currPlayer;
		}

		return CheckWin(tempBoard) * player; //令其始终表示对当前节点而言的优劣，便于Select判断
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

void MCTS() {
	int limit = 1000;

	TreeNode* root = new TreeNode(-1, -1, -1, nullptr); // 初始化根节点

	for (int i = 0; i < limit; i++) { // 姑且先循环固定的次数
		TreeNode* node = root; 
		UntriedMoves = EmptyGrid; // 用来记录待扩展节点所对应的状态

		// 测试一下Coord赋值的重载对不对
		cout << UntriedMoves.size() << endl;
		//

		while (!node->children.empty()) {
		
			node = node->Select();
			for (auto iter = UntriedMoves.begin(); iter != UntriedMoves.end(); iter++) { // 手动更新状态
				if (iter->x == node->move_x && iter->y == node->move_y) {
					UntriedMoves.erase(iter);
					break;
				}
			}

		}

		if (node == nullptr) { 
			// 某次Select返回的是nullptr 理论上来说不可能
			return;
		}

		if (node->n >= counter) {
			// 只有一个节点被模拟了足够的次数时, 才进行扩展
			node = node->Expand();
		}
		
		
		if (node == nullptr) { 
			// 这个叶节点扩展不出来一个节点, 也就是说, 该节点表示的状态是一个摆满棋子的棋盘
			return;
		}

		double result = node->Simulate();

		node->BackPropagate(result);
	}

	// 选出根节点的最优子节点并更新new_x, new_y

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

int main()
{
	int x, y, n;
	//恢复目前的棋盘信息
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
		cin >> x >> y; if (x != -1) board[x][y] = -1;	//对方
		cin >> x >> y; if (x != -1) board[x][y] = 1;	//我方
	}
	cin >> x >> y;
	if (x != -1) board[x][y] = -1;	//对方

	//此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋

	/************************************************************************************/
	/***********在下面填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/

	srand(time(0)); // 生成随机数种子
	
	Init(); // 初始化EmptyGrod

	MCTS(); // 蒙特卡洛树搜索

	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果
	cout << new_x << ' ' << new_y << endl;
	return 0;
}
