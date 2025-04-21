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

struct Coord{
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
				EmptyGrid.push_back({i,j}); // 奇怪的写法
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
	int n;							// 当前节点被更新的次数
	int player;						// 这一步的玩家
	int move_x, move_y;				// 这一步的动作
	vector<TreeNode*> children;		// 这个节点的儿子
	TreeNode* father;				// 这个节点的监护人
public:
	TreeNode(int pos_x = -1, int pos_y = -1, int pl = 0) {
		val = 0;
		n = 0;
		move_x = pos_x;
		move_y = pos_y;
		player = pl;
		father = NULL;
	}
	TreeNode* Select() {

	}

	TreeNode* Expand() { // Expand 被 Select 调用 
		

	}

};

TreeNode* MTCL_Root = NULL;

/*


*/

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






	/***********在上方填充你的代码，决策结果（本方将落子的位置）存入new_x和new_y中****************/
	/************************************************************************************/

	// 向平台输出决策结果
	cout << new_x << ' ' << new_y << endl;
	return 0;
}
