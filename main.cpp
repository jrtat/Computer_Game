#include <iostream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std;

const int SIZE = 15; // 意★义★不★明
int board[SIZE][SIZE] = { 0 };//本方1，对方-1，空白0

int new_x, new_y; // 下一步棋下哪

struct Coord {
	int x, y;
	Coord(int xx = -1, int yy = -1) {
		x = xx, y = yy;
	}
};

vector<Coord> EmptyGrid;  // 用来记录没下过的点

void Init() {

	for (int i = 0; i <= 10; i++) { // 初始化 EmptyGrid 把所有空节点加入
		for (int j = 0; j <= 10; j++) {
			if (board[i][j] == 0) {
				EmptyGrid.push_back({ i,j }); // 奇怪的写法
			}
		}
	}

}

/*

shuffle

*/

class TreeNode {

private:

	double val;						// 当前节点的评价值
	int player;						// 这一步的玩家(-1,1)

public:

	int n;							// 当前节点被更新的次数
	int move_x, move_y;				// 这一步的动作
	vector<TreeNode*> children;		// 这个节点的儿子
	TreeNode* father;				// 这个节点的监护人
	vector<Coord>UntriedMoves;

	TreeNode(int pos_x = -1, int pos_y = -1, int pl = 0, TreeNode* fa = nullptr) {
		val = 0;
		n = 0;
		move_x = pos_x;
		move_y = pos_y;
		player = pl;
		father = fa;
		if(father == nullptr){
			for(auto coord:EmptyGrid){
				UntriedMoves.push_back(coord);
			}
		}
	}

	TreeNode* Select() {
		TreeNode* best = nullptr;
		double score = -1e9;
		for (auto child : children) {
			double ucb = child->val*1.0 / child->n + sqrt(2*log(n)*1.0 / child->n);
			if (ucb > score) {
				score = ucb;
				best = child;
			}
		}
		return best;
	}
	
	TreeNode* Expand() { // Expand 被 Select 调用 
		if(UntriedMoves.empty())return nullptr;

		int t = rand() % UntriedMoves.size();
		Coord move = UntriedMoves[t];
		UntriedMoves.erase(UntriedMoves.begin() + t);

		TreeNode* child = new TreeNode(move.x,move.y,-player,this);
		child->UntriedMoves=this->UntriedMoves;
		children.push_back(child);

		return child;
	}

	double Simulate(){
		int tempBoard[SIZE][SIZE];
		memcpy(tempBoard, board, sizeof(board));
		vector<Coord>tempEmpty = EmptyGrid;

		if(move_x != -1 && move_y != -1){
			tempBoard[move_x][move_y] = player;
			tempEmpty.erase(remove_if(tempEmpty.begin(), tempEmpty.end(),
				[&](const Coord& re){return re.x==move_x&&re.y==move_y;}), tempEmpty.end());
		}

		int currPlayer = -player;
		while(!tempEmpty.empty()){
			int r = rand()%tempEmpty.size();
			Coord move = tempEmpty[r];
			tempBoard[move.x][move.y] = currPlayer;
			tempEmpty.erase(tempEmpty.begin()+r);
			currPlayer = -currPlayer;
		}

		return CheckWin(tempBoard) * player; //令其始终表示对当前节点而言的优劣，便于Select判断
	}

	void BackPropagate(double re){
		TreeNode* node = this;
		while(node != nullptr){
			node->n += 1;
			node->val += re;
			re = -re;
			node = node->father;
		}
	}

};

TreeNode* MCTS(int limit,int player){
	TreeNode* root = new TreeNode(-1,-1,player,nullptr);
	for(int i = 0; i < limit; i++){
		TreeNode* node = root;
		while(!node->UntriedMoves.empty() && !node->children.empty()){
			node = node->Select();
		}

		TreeNode* expand = nullptr;
		if(!node->UntriedMoves.empty()){
			expand = node->Expand();
		} else {
			expand = node;
		}

		double result = expand->Simulate();

		expand->BackPropagate(result);
	}

	TreeNode* BestChild = nullptr;
	int MaxVisit = -1;
	for(auto child : root->children){
		if(child->n > MaxVisit){
			MaxVisit = child->n;
			BestChild = child;
		}
	}
	if(BestChild){
		new_x = BestChild->move_x;
		new_y = BestChild->move_y;
	} else {
		Coord fallback = EmptyGrid[rand()%EmptyGrid.size()];
		new_x = fallback.x;
		new_y = fallback.y;
	}
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

	srand(time(0));
	Init();
	
	TreeNode* root = MCTS(1000,1);


	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果
	cout << new_x << ' ' << new_y << endl;
	return 0;
}
