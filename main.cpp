#include <chrono>
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <cmath>

# define	ROW		5
# define	COLUMN	6
# define	NUM		30

# define	RIGHT 0
# define	LEFT 1
# define	DOWN 2
# define	UP 3


# define	MANHATTAN_DISTANCE	1
# define	HAMMING_DISTANCE	2
# define	LINEAR_CONFLICT		3
# define	LIMIT_DEPTH			1000 
# define	NODE_LIMIT			100000005

using namespace std;
class Node;
using pin = pair<int, Node>;

int	puzzle[5][6] = {
	{ 7, 1, 14, 10, 11, 6},
	{ 2, 15, 8, 16, 4, 5},
	{ 13, 0, 22, 9, 18, 3},
	{ 19, 20, 21, 28, 17, 12},
	{ 25, 26, 23, 27, 29, 24},
};

struct NodeInfo
{
	bool	isClosed; // is Closed
	int		cost; // now Cost
	int		parent; // parent

	bool operator==(const NodeInfo &other) const {
		return (parent == other.parent && cost == other.cost);
	}

	bool operator!=(const NodeInfo &other) const {
		return (!(other == *this));
	}
};

static int dirX[4] = {0, 0, 1, -1}; // RIGHT-LEFT-DOWN-UP
static int dirY[4] = {1, -1, 0, 0}; // RIGHT-LEFT-DOWN-UP
static map<Node, NodeInfo>	visited;

const std::string	RED("\033[0;31m");
const std::string	SKY("\033[0;36m");
const std::string	BLUE("\033[0;34m");
const std::string	RESET("\033[0m");

// Node 정의
class Node
{
public:
	int		**board;
	bool	emptyNode;

	// 매개변수 없이 초기화
	Node()
	{
		int	i;

		board = nullptr;
		emptyNode = true;
		// 2차원 배열 동적할당
		board = new int *[ROW];
		for (i = 0; i < ROW; i++)
		{
			board[i] = new int[COLUMN];
			// 할당 후 0으로 초기화
			memset(board[i], 0, COLUMN * sizeof(board[0][0]));
		}
	}

	// Node인자를 받아서 초기화
	Node(const Node &node)
	{
		int	i;
		int	j;

		this->~Node();
		this->emptyNode = node.emptyNode;
		board = new int *[ROW];
		for (i = 0; i < ROW; i++)
		{
			board[i] = new int[COLUMN];
		}
		for (i = 0; i < ROW; i++)
		{
			for (j = 0; j < COLUMN; j++)
			{
				board[i][j] = node.board[i][j];
			}
		}
	}

	Node &operator=(const Node &node)
	{
		int	i;
		int	j;

		this->~Node();
		this->emptyNode = node.emptyNode;
		board = new int *[ROW];
		for (i = 0; i < ROW; i++)
		{
			board[i] = new int[COLUMN];
		}
		for (i = 0; i < ROW; i++)
		{
			for (j = 0; j < COLUMN; j++)
			{
				board[i][j] = node.board[i][j];
			}
		}
		return *this;
	}

	~Node()
	{
		if (board == nullptr)
			return ;
		board = nullptr;
	}

	// board가 하나라도 다르면 False 리턴
	bool operator==(const Node &node) const {
		int	i;
		int	j;

		for (i = 0; i < ROW; i++)
		{
			for (j = 0; j < COLUMN; j++)
			{
				if (board[i][j] != node.board[i][j])
					return (false);
			}
		}
		return (true);
	}

	bool operator!=(const Node &node) const {
		return (!(*this == node));
	}

	// 좌측 상단부터 보드 값이 하나라도 크면 true
	bool operator<(const Node &node) const {
		int	i;
		int	j;

		for (i = 0; i < ROW; i++)
		{
			for (j = 0; j < COLUMN; j++)
			{
				if (board[i][j] < node.board[i][j])
					return (true);
				else if (board[i][j] > node.board[i][j])
					return (false);
				else
					continue ;
			}
		}
		return (false);
	}

	static bool	get_zero_pos(const Node &node, int &zx, int &zy)
	{
		int	i;
		int	j;

		zx = zy = -1;
		for (i = 0; i < ROW; i++)
		{
			for (j = 0; j < COLUMN; j++)
			{
				if (node.board[i][j] == 0)
				{
					zx = i;
					zy = j;
					return (true);
				}
			}
		}
		return (false);
	}

	static int	opposite_direction(int direction)
	{
		switch (direction)
		{
			case LEFT:
				return (RIGHT);
			case RIGHT:
				return (LEFT);
			case UP:
				return (DOWN);
			case DOWN:
				return (UP);
			default:
				return EOF;
		}
	}

	static int	get_inv_count(const vector<int> &arr)
	{
		int	i, j;
		int	cnt;

		cnt = 0;
		for (i = 0; i < arr.size(); i++)
		{
			for (j = i + 1; j < arr.size(); j++)
			{
				if (arr[j] == 0)
					continue ;
				if (arr[i] > arr[j])
				{
					cnt++;
				}
			}
		}
		return (cnt);
	}

	bool	is_empty_node() const {
		return (emptyNode);
	}

	Node	getNode(int direction, int zx = -1, int zy = -1)
	{
		Node	node;
		int		nx;
		int		ny;

		// direction 3 ?
		if (board == nullptr || direction > 3)
			return (*this);
		// invalid -> 새로운 노드 반환
		if (zx == -1 || zy == -1) {
			if (!get_zero_pos(*this, zx, zy))
				return (Node());
		}

		nx = zx + dirX[direction];
		ny = zy + dirY[direction];
		if (nx < 0 || ny < 0 || nx >= ROW || ny >= COLUMN)
			return (Node());

		node = *this;
		// 다음 node 반환
		swap(node.board[zx][zy], node.board[nx][ny]);
		return (node);
	}
};

static bool is_valid(int x, int y)
{
	return (x >= 0 && y >= 0 && x < ROW && y < COLUMN);
}

// 해밍 거리 구하기
static int	get_hamming_distance(const Node &a, const Node &b)
{
	int	i;
	int	j;
	int	conflict;

	conflict = 0;
	for (i = 0; i < ROW; i++)
		for (j = 0; j < COLUMN; j++)
			// 값이 있으면서 서로 다르면 conflict
			if (a.board[i][j] != 0 && a.board[i][j] != b.board[i][j])
				conflict++;
	return (conflict);
}

// manhattan distance 구하기
static int	get_manhattan_distance(const Node &a, const Node &b)
{
	int	r, c;
	int	num;
	int	md;
	int	pR[NUM]; // 임의의 값 num이 가지는 row index 저장할 예정
	int	pC[NUM];

	md = 0;
	for (r = 0; r < ROW; r++)
	{
		for (c = 0; c < COLUMN; c++)
		{
			// num에 해당하는 row, col을 pR, pC에 각각 넣어 줌
			num = a.board[r][c];
			pR[num] = (r);
			pC[num] = (c);
		}
	}

	for (r = 0; r < ROW; r++)
	{
		for (c = 0; c < COLUMN; c++)
		{
			num = b.board[r][c];
			// 원하는 값과 얼마나 다른지 파악
			if (num != 0) 
				md += abs(pR[num] - r) // row의 차이
						+ abs(pC[num] - c); // column의 차이
		}
	}
	return (md);
}

// row conflict 구하기
static int	get_row_conflicts(const Node &a, const Node &b, int pR[], int pC[])
{
	int	r, c;
	int	x, y;
	int	num1, num2;
	int	conflict;
	
	conflict = 0;
	// row 0부터 ROW - 1
	for (r = 0; r < ROW; r++)
	{
		// 한 줄마다 row conflict를 확인한다.
		// 0 인덱스부터 1 ~ COL - 1을 비교한다.
		for (x = 0; x < COLUMN; x ++)
		{
			num1 = b.board[r][x];
			for (y = x + 1; y < COLUMN; y++)
			{
				num2 = b.board[r][y];
				// 0이 아니면서
				if (num1 != 0 && num2 != 0)
				{
					// 모두 같은 로우에 있으면서
					if (r == pR[num1] && pR[num1] == pR[num2])
					{
						// num1(왼쪽에 있는 값이 더 커버리면 충돌 상태)
						if (pC[num1] > pC[num2])
							conflict++;
					}
				}
			}
		}
	}
	return (conflict);
}

// column conflict 구하기
static int	get_column_conflicts(const Node &a, const Node &b, int pR[], int pC[])
{
	int	r, c;
	int	x, y;
	int	num1, num2;
	int	conflict;
	
	conflict = 0;
	// col 0 부터 COL - 1
	for (r = 0; r < COLUMN; r++)
	{
		// 한 줄마다 column conflict를 확인한다.
		// 0 인덱스부터 1 ~ ROW - 1을 비교한다.
		for (x = 0; x < ROW; x ++)
		{
			num1 = b.board[x][r];
			for (y = x + 1; y < ROW; y++)
			{
				num2 = b.board[y][r];
				// 0이 아니면서
				if (num1 != 0 && num2 != 0)
				{
					// 모두 같은 컬럼에 있으면서
					if (c == pC[num1] && pC[num1] == pC[num2])
					{
						// num1(위쪽에 있는 값이 더 커버리면 충돌 상태)
						if (pR[num1] > pR[num2])
							conflict++;
					}
				}
			}
		}
	}
	return (conflict);
}

// 총 conflict 구하기
static int	get_conflict_cnt(const Node &a, const Node &b)
{
	int	r, c;
	int	num;
	int	conflict;
	int	pR[NUM];
	int	pC[NUM];

	for (r = 0; r < ROW; r++)
	{
		for (c = 0; c < COLUMN; c++)
		{
			// num에 해당하는 Row, col을 각각 넣어줌
			num = a.board[r][c];
			pR[num] = (r);
			pC[num] = (c);
		}
	}
	// find conflicts
	conflict = 0;
	conflict += get_row_conflicts(a, b, pR, pC);
	conflict += get_column_conflicts(a, b, pR, pC);
	return (conflict);
}

// h = md + 2 * lc (Linear conflict)
static	int	get_linear_conflict(const Node &a, const Node &b)
{
	return (get_manhattan_distance(a, b)
				+ 2 * get_conflict_cnt(a, b));
}

static int	get_heuristic(const Node &a, const Node &b, int heuristicType)
{
	switch (heuristicType)
	{
		case HAMMING_DISTANCE:
			return (get_hamming_distance(a, b));
		case MANHATTAN_DISTANCE:
			return (get_manhattan_distance(a, b));
		case LINEAR_CONFLICT:
			return (get_linear_conflict(a, b));
		default:
			break ;
	}
	return (0);
}

static void	go_a_star_search(const Node &start, const Node &goal, int heuristicType)
{
	priority_queue<pin, vector<pin>, greater<vector<pin>::value_type> > openList;
	int		x, y, nx, ny;
	int		dir, nextCost, priority;
	bool	isVisited;
	Node	currentNode, nextNode;

	visited[start] = { false, 0, EOF };
	openList.push({ 0, start });
	// openList가 비어있지 않을 동안에
	while (!openList.empty())
	{
		// 1개의 자식 노드를 확인한다.
		currentNode = openList.top().second;
		openList.pop();

		NodeInfo &uInfo = visited[currentNode];
		uInfo.isClosed = true;

		// goal을 찾았을 때
		if (currentNode == goal)
			break ;
		// 자식 부모 트리가 너무 깊어질 때
		if (uInfo.cost > LIMIT_DEPTH)
		{
			cout << "너무 많이 검색했습니다(DEPTH).\n";
			break ;
		}
		if (visited.size() > NODE_LIMIT)
		{
			cout << "너무 많이 검색했습니다(NODE).\n";
			break ;
		}

		// 실제로 움직이는 부분
		x = -1, y = -1; // zeroIndex
		Node::get_zero_pos(currentNode, x, y);
		for (dir = 0; dir < 4; dir++)
		{
			nx = x + dirX[dir];
			ny = y + dirY[dir];

			// 다음 zeroIndex가 유효한 움직임이었다면?
			if (is_valid(nx, ny))
			{
				nextNode = currentNode;
				// 기존 0 위치와 다음 0 위치를 바꾸어줌
				swap(nextNode.board[x][y], nextNode.board[nx][ny]);
				
				// 이미 방문 했었는지 찾아봄
				// 값을 찾으면 find는 end를 반환하지 않음
				isVisited = visited.find(nextNode) != visited.end();
				// 방문이 되어있거나, 닫힌 상태면 더이상 확인하지 않음
				if (isVisited && visited[nextNode].isClosed)
					continue ;
				nextCost = uInfo.cost + 1;
				// 방문을 하지 않았거나
				// 기존의 방문보다 비용이 더 적게들었으면 다시 작업한다.
				if (!isVisited || nextCost < visited[nextNode].cost)
				{
					visited[nextNode] = {false, nextCost, Node::opposite_direction(dir)};
					priority = nextCost + get_heuristic(nextNode, goal, heuristicType);
					openList.push({ priority, nextNode });
				}
			}
		}
	}
}

// reference : https://www.cs.mcgill.ca/~newborn/nxp_puzzleOct9.htm
static bool	is_solvable_puzzle(const Node &start, const Node &goal)
{
	int			zx, zy;
	int			inversion_cnt;
	int			i, j;
	vector<int>	board;

	for (i = 0; i < ROW; i++)
	{
		for (j = 0; j < COLUMN; j++)
		{
			board.push_back(start.board[i][j]);
		}
	}
	inversion_cnt = Node::get_inv_count(board);
	Node::get_zero_pos(start, zx, zy);
	// NI + Row of the blank % 2 == 1 -> is not solvable
	if ((inversion_cnt + zx) % 2 == 1)
		return (false);
	return (true);
}

static void	print_solution(const Node &start, const Node &goal)
{
	vector<char>	paths;
	int				x, y;
	int				nx, ny;
	Node			now;

	now = goal;
	Node::get_zero_pos(now, x, y);
	while (visited[now].parent != EOF)
	{
		now = now.getNode(visited[now].parent);
		Node::get_zero_pos(now, nx, ny);
		if (nx == x + 1 && ny == y)
			paths.push_back('U');
		else if (x == nx && ny == y + 1)
			paths.push_back('L');
		else if (nx == x - 1 && ny == y)
			paths.push_back('D');
		else
			paths.push_back('R');
		x = nx;
		y = ny;
	}
	Node::get_zero_pos(start, x, y);
	reverse(paths.begin(), paths.end());
	std::cout << "SOLVED PATHS' LENGTH : " << paths.size() << "\n";
	for (auto &i : paths)
		cout << i << " ";
	cout << '\n';
}

void	Solver(int puzzle[5][6])
{
	int		i;
	int		j;
	Node	start;
	Node	goal;

	// Infomation
	std::cout << SKY << "========  팀  ========\n";
	std::cout << RESET << "\t이상민\n\t이서영\n\t이찬웅\n";
	std::cout << SKY << "======================\n" << RESET;
	// board 초기화
	for (i = 0; i < ROW; i++)
	{
		for (j = 0; j < COLUMN; j++)
		{
			goal.board[i][j] = (i * COLUMN + j + 1);
			start.board[i][j] = (puzzle[i][j]);
		}
	}
	goal.board[ROW - 1][COLUMN - 1] = 0;
	if (is_solvable_puzzle(start, goal) == false)
	{
		cout << RED << "Current Puzzle is not solvable, Put the another puzzle plz.\n" << RESET;
		return ;
	}

	cout << "Linear Conflict : " << get_linear_conflict(start, goal) << endl;
	go_a_star_search(start, goal, LINEAR_CONFLICT);

	// LINEAR만 작동시키기
//	cout << "Manhattan Distance : " << get_manhattan_distance(start, goal) << endl;
//	go_a_star_search(start, goal, MANHATTAN_DISTANCE);

//	cout << "Hamming Distance : " << get_hamming_distance(start, goal) << endl;
//	go_a_star_search(start, goal, HAMMING_DISTANCE);

	print_solution(start, goal);
}

int	main(void)
{
	std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
	Solver(puzzle);
	std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
	std::chrono::milliseconds millisecond =
			std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
	std::cout << "TimeCost: " << millisecond.count() << " ms\n";
}
