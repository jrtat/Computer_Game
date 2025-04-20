#include <iostream>
#include <vector>
#include <stack>
using namespace std;
stack<int> dfs;
struct stat {
	int config; // 当前格局
	int type; // type = 0 自己 type = 1 机器
	int alpha = -1, beta = 2;
	vector<stat*> son;
	stat(int s = 0, int a = -1, int b = -1, int c = 2) { config = s, type = a, alpha = b, beta = c; };
}s[20001], *root;
// 可以把一个井字棋状态转成三进制(3^9)  0表示空位, 1表示X, 2表示O
int vis[20001] = { 0 };
int main() {
	dfs.push(stat(2<<4, 0,-1,2)); // 初始状态
	vis[0] = 1;
	while (!dfs.empty()) {
		stat *cur = dfs.top();
		dfs.pop();
		for (int i = 1; i <= 9; i++) {
			int test = (cur->config % (3 << i)) / (3 << (i - 1));
			cout << test;
		}
	}


}