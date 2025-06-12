

// 计算潜力值
int ExpNum = 15, Tolerant = 2;
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

