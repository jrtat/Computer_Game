#include <iostream>
#include <vector>
#include <stack>
using namespace std;
stack<int> dfs;
struct stat {
	int config; // ��ǰ���
	int type; // type = 0 �Լ� type = 1 ����
	int alpha = -1, beta = 2;
	vector<stat*> son;
	stat(int s = 0, int a = -1, int b = -1, int c = 2) { config = s, type = a, alpha = b, beta = c; };
}s[20001], *root;
// ���԰�һ��������״̬ת��������(3^9)  0��ʾ��λ, 1��ʾX, 2��ʾO
int vis[20001] = { 0 };
int main() {
	dfs.push(stat(2<<4, 0,-1,2)); // ��ʼ״̬
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