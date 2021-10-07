#include <algorithm>
#include <unordered_set>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <stdlib.h>
#include <list>

using namespace std;

const int N = 9;

vector<vector<int>> elemek(27), neighbours(81), position(81);

class Sudoku
{
	vector<vector<bool>> cells;
	bool ok = false;
	bool naked = false;
	bool ptp = false;

public:
	Sudoku();

	Sudoku(string line);

	static void setup();

	int possibilityCount(int i)
	{
		return std::count(cells[i].begin(), cells[i].end(), true);
	}

	int value(int i)
	{
		auto it = find(cells[i].begin(), cells[i].end(), true);
		return (it != cells[i].end() ? 1 + (it - cells[i].begin()) : -1);
	}

	bool isPossible(int i, int ertek)
	{
		return cells[i][ertek - 1];
	}

	string stringForm();

	bool solved();
	bool putIn(int k, int val);
	bool remove(int k, int val);
	bool nakedpair();
	bool pointingpairs();

	void takeOut(int k);
	void putInBack(int k, int val);
	void kiir();

	int lowestPossibility();

	void setok(bool ok)
	{
		this->ok = ok;
	}

	bool getok()
	{
		return ok;
	}

	void setnaked(bool t)
	{
		this->naked = t;
	}

	bool getnaked()
	{
		return this->naked;
	}

	void setptp(bool b)
	{
		this->ptp = b;
	}

	bool getptp()
	{
		return ptp;
	}
};

Sudoku::Sudoku(string line) : cells(81, vector<bool>(9, true))
{
	if (line != "") // ha ures sudokut szeretnek
	{
		int k = 0;
		if (line.size() != 81)
		{
			throw -1;
		}
		else
		{
			for (int i = 0; i < 81; i++)
			{
				if (line[i] >= '1' && line[i] <= '9')
				{
					if (!putIn(k, line[i] - 48))
					{
						cout << "A sudoku nem helyes. Nincs Megoldas!\n";
						throw -1;
						break;
					}
				}
				k++;
			}
		}
	}
}

string Sudoku::stringForm()
{
	string res = "";
	for (int i = 0; i < 81; i++)
	{
		if (possibilityCount(i) == 1)
		{
			res += (value(i) + 48);
		}
		else
		{
			res += ".";
		}
	}
	return res;
}

void Sudoku::setup()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			const int k = i * 9 + j;
			const int x[3] = {i, 9 + j, 18 + (i / 3) * 3 + j / 3};
			for (int g = 0; g < 3; g++)
			{
				elemek[x[g]].push_back(k);
				position[k].push_back(x[g]);
			}
		}
	}
	for (int k = 0; k < 81; k++)
	{
		for (int x = 0; x < 3; x++) // sor, oszlop, box
		{
			for (int j = 0; j < 9; j++)
			{
				int k2 = elemek[position[k][x]][j];
				if (k2 != k)
					neighbours[k].push_back(k2);
			}
		}

		// Gyorsitas a duplikatumokat megszunteti
		// unordered_set<int> s;
		// for (int i : neighbours[k])
		// {
		//     s.insert(i);
		// }
		// neighbours[k].assign(s.begin(), s.end());
	}
}

bool Sudoku::putIn(int k, int sz)
{
	for (int i = 1; i <= 9; i++)
	{
		if (i != sz)
		{
			// toroljuk a a k.dik cellarol az i lehetseges szamot, kiveve a (sz) szamot
			if (!remove(k, i))
			{
				return false;
			}
			// cout << i << " .szam putInben!" << endl << endl;
			// kiir();
		}
	}
	return true;
}

bool Sudoku::remove(int k, int sz)
{
	// ha a k. dik cellaban az (sz) szam mint isPossible nincs benne akkor nem kell kitorolni, hiszen ki van torolve
	if (!cells[k][sz - 1])
	{
		return true;
	}

	// ha a k. dik cellaban az (sz) szam mint isPossible benne van, akkor kitoroljuk az sz szamot -> false lesz
	cells[k][sz - 1] = false;
	int possCount = possibilityCount(k);

	// nem torolhetunk ha csak egy isPossible volt a cellaban
	if (possCount == 0)
	{
		return false;
	}
	else if (possCount == 1)
	{
		// ha egy elem maradt a k.dik cellaban, akkor ennek minden szomszedjabol kitoroljuk ezt az elemet
		// value - megadja az elso olyan szamot ami lehetseges a cellaba

		int v = value(k);

		// cout << "v = " << v << endl;

		// for(int i = 0; i < neighbours[k].size(); i++){
		//     cout << neighbours[k][i] << " ";
		// }
		// cout << endl;

		for (int i = 0; i < neighbours[k].size(); i++)
		{
			if (!remove(neighbours[k][i], v))
			{
				return false;
			}
		}
	}

	// ha tobb mint 1 megoldas maradt a k. cellara
	// megvizsgaljuk a szomszedjait  a k. cellanak, majd ujra szamoljuk a cellsat
	for (int i = 0; i < 3; i++) // sor, oszlop, box
	{
		int x = position[k][i];
		int n = 0;

		// az utolso cella a sor, oszlop vagy box neighbours kozul, amelyikebn benne van az (sz) szam
		int u_cel;

		// a k. dik cellanak nezzuk a szomszedjait sor -> oszlop -> box
		for (int j = 0; j < 9; j++)
		{
			int sob = elemek[x][j]; // sob.dik sor,oszlop vagy box elemei a k.dik cellanak

			if (sob != k && cells[sob][sz - 1]) // ha a szam benne van a lehetseges szamokban
			{
				u_cel = sob; // megorizzuk az utolso isPossibleet a k.dik cellaban
				n++;
			}
		}

		if (n == 0) // ha nem volt egyetlen isPossible sem
		{
			return false;
		}
		else if (n == 1) // ha csak egy isPossible volt
		{
			if (!putIn(u_cel, sz))
			{
				return false;
			}
		}
	}
	return true;
}

int Sudoku::lowestPossibility()
{
	// megkeresem azt a cellat amiben a legkevesebb isPossible van
	int minimum;
	int l;
	int k = -1;

	for (int i = 0; i < 81; i++)
	{
		l = possibilityCount(i);
		if (l > 1)
		{
			if (k == -1 || l < minimum)
			{
				minimum = l;
				k = i;
			}
		}
	}
	return k;
}

bool Sudoku::solved()
{
	for (int i = 0; i < cells.size(); i++)
	{
		if (possibilityCount(i) != 1)
		{
			return false;
		}
	}
	return true;
}

void Sudoku::kiir()
{
	// for (int i = 0; i < N; i++)
	// {
	//     for (int j = 0; j < N; j++)
	//     {
	//         int k = i * 9 + j;
	//         if (possibilityCount(k) == 1)
	//         {
	//             cout << value(k) << " ";
	//         }
	//         else
	//         {
	//             cout << ". ";
	//         }
	//     }
	//     cout << endl;
	// }
	// cout << endl;

	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			int k = i * 9 + j;

			int num = 0;
			for (int l = 0; l < 9; l++)
			{
				if (cells[k][l])
				{
					num = num * 10 + (l + 1);
				}
			}

			printf("%9d  ", num);

			if (j % 2 == 0)
			{
				printf(" ");
			}
		}

		printf("\n");

		if ((i + 1) % 3 == 0)
		{
			printf("\n");
		}
	}
	printf("\n");
}

bool Sudoku::nakedpair()
{

	// sor, oszlop vagy box parokrol van szo

	// egy sorban van a par
	for (int i = 0; i < 9; i++)
	{

		for (int j = 0; j < 8; j++)
		{

			int x = elemek[i][j];

			for (int k = j + 1; k < 9; k++)
			{

				int y = elemek[i][k];
				vector<int> pair;

				if (possibilityCount(x) == 2 && possibilityCount(y) == 2)
				{

					for (int l = 1; l <= 9; l++)
					{

						if (isPossible(x, l) && isPossible(y, l))
						{
							pair.push_back(l);
						}
					}

					if (pair.size() == 2)
					{ // kaptam egy naked part a sorban
						// cout << i <<". sorban naked pair!" << endl;
						// kiir();
						return true;
					}
				}
			}
		}
	}

	// naked pair oszlopban
	for (int i = 9; i <= 18; i++)
	{

		for (int j = 0; j < 8; j++)
		{

			int x = elemek[i][j];

			for (int k = j + 1; k < 9; k++)
			{

				int y = elemek[i][k];
				vector<int> pair;

				if (possibilityCount(x) == 2 && possibilityCount(y) == 2)
				{

					for (int l = 1; l <= 9; l++)
					{

						if (isPossible(x, l) && isPossible(y, l))
						{
							pair.push_back(l);
						}
					}

					if (pair.size() == 2)
					{
						// kaptam egy naked part a sorban
						// cout << i <<". oszlopban naked pair!" << endl;
						// kiir();
						return true;
					}
				}
			}
		}
	}

	// naked pair boxban
	for (int i = 18; i < 27; i++)
	{

		for (int j = 0; j < 8; j++)
		{

			int x = elemek[i][j];

			for (int k = j + 1; k < 9; k++)
			{

				int y = elemek[i][k];
				vector<int> pair;

				if (possibilityCount(x) == 2 && possibilityCount(y) == 2)
				{
					for (int l = 1; l <= 9; l++)
					{

						if (isPossible(x, l) && isPossible(y, l))
						{
							pair.push_back(l);
						}
					}
					if (pair.size() == 2)
					{
						// kaptam egy naked part a sorban
						// cout << i <<". boxban naked pair!" << endl;
						// kiir();
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool Sudoku::pointingpairs()
{
	// boxokban keresi egy sorban
	for (int i = 18; i < 27; i++)
	{
		for (int j = 0; j < 9; j += 3)
		{
			for (int k = 0; k < 2; k++)
			{
				int x = elemek[i][k + j];
				for (int l = k + 1; l < 3; l++)
				{
					int y = elemek[i][l + j];
					for (int m = 1; m <= 9; m++)
					{
						if (possibilityCount(x) > 1 && possibilityCount(y) > 1 && isPossible(x, m) && isPossible(y, m))
						{
							// ez az elem nem szabad eloforduljon mashol a boxban csa x es y ban
							bool ok = true;
							for (int p = 0; p < 9; p++)
							{
								// bejarom a tobbi elemet a boxban
								int c = elemek[i][p];

								if (c != x && c != y && isPossible(c, m))
								{
									ok = false;
									break;
								}
							}

							if (ok)
							{
								// megnezem hogy ebben a sorban van ezen a paron kivul, mas boxban , ilyen elem
								// megkeresem a sorat a poziciobol(sor, oszlop, box)
								int sor = position[x][0];

								for (int n = 0; n < 9; n++)
								{
									int c = elemek[sor][n];

									if (c != x && c != y && possibilityCount(c) > 1 && isPossible(c, m))
									{
										// cout << i << ". boxban pointingpairs - soros!" << endl;
										// cout << "x=" << x << " - y=" << y << " - m=" << m <<  endl;
										// kiir();
										return true;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// boxokban keresi egy oszlopban
	for (int i = 18; i < 27; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 6; k += 3)
			{ // oszlop ugras
				int x = elemek[i][k + j];
				for (int l = k + 3; l < 9; l += 3)
				{
					int y = elemek[i][l + j];
					for (int m = 1; m <= 9; m++)
					{
						if (possibilityCount(x) > 1 && possibilityCount(y) > 1 && isPossible(x, m) && isPossible(y, m))
						{

							// ez az elem nem szabad eloforduljon mashol a boxban csak x es y ban
							bool ok = true;
							for (int p = 0; p < 9; p++)
							{
								// bejarom a tobbi elemet a boxban
								int c = elemek[i][p];

								if (c != x && c != y && isPossible(c, m))
								{
									ok = false;
									break;
								}
							}

							if (ok)
							{
								// megnezem hogy ebben a sorban van ezen a paron kivul, mas boxban , ilyen elem
								// megkeresem a sorat a poziciobol(sor, oszlop, box)
								int oszlop = position[x][1];

								for (int n = 0; n < 9; n++)
								{
									int c = elemek[oszlop][n];

									if (c != x && c != y && possibilityCount(c) > 1 && isPossible(c, m))
									{
										// cout << i << ". boxban pointingpairs - oszlopos!" << endl;
										// cout << "x=" << x << " - y=" << y  << " - m = " << m << endl;
										// kiir();
										return true;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return false;
}

string starting_rand_sudoku()
{

	vector<int> rv(9);

	for (int i = 0; i < 9; i++)
	{
		rv[i] = i + 1;
	}

	string res = "";
	for (int i = 0; i < 81; i++)
	{
		res += ".";
	}

	std::random_shuffle(rv.begin(), rv.end());
	for (int i = 0; i < 9; i++)
	{
		res[elemek[18][i]] = (rv[i] + 48);
	}

	std::random_shuffle(rv.begin(), rv.end());
	for (int i = 0; i < 9; i++)
	{
		res[elemek[22][i]] = (rv[i] + 48);
	}

	std::random_shuffle(rv.begin(), rv.end());
	for (int i = 0; i < 9; i++)
	{
		res[elemek[26][i]] = (rv[i] + 48);
	}
	return res;
}

void solutions(Sudoku S, int &count)
{
	if (S.solved())
	{
		count++;
		return;
	}

	int k = S.lowestPossibility();

	for (int i = 1; i <= 9; i++)
	{
		// ha a k. cellaban az i. ertek valaszthato(isPossible)
		if (S.isPossible(k, i))
		{
			Sudoku S1(S);

			if (S1.putIn(k, i))
			{
				if (count > 1)
				{
					return;
				}
				solutions(S1, count);
				if (count > 1)
				{
					return;
				}
			}
		}
	}
	return;
}

bool isSingleSolution(Sudoku S)
{
	int count = 0;
	solutions(S, count);
	return count > 1 ? false : true;
}

Sudoku solve(Sudoku s)
{
	if (s.solved())
	{
		s.setok(true);
		return s;
	}

	int k = s.lowestPossibility();

	for (int i = 1; i <= 9; i++)
	{

		// ha a k. cellaban az i. ertek valaszthato(isPossible)
		if (s.isPossible(k, i))
		{

			// ha sikerul berakni es kiloni az elemeket akkor a kovetkezo szamot probaljuk
			// lemasoljuk az eredetit, hogy megvizsgaljuk a torlest, igy nem kel visszalepni
			Sudoku saux(s);

			if (saux.putIn(k, i))
			{
				Sudoku s_res = solve(saux);

				if (s_res.getok())
				{
					return s_res;
				}
			}
		}
	}
	Sudoku serr("");
	return serr;
}

Sudoku solve_diff(Sudoku s)
{

	if (!s.getnaked() && s.nakedpair())
	{
		s.setnaked(true);
	}

	if (!s.getptp() && s.pointingpairs())
	{
		s.setptp(true);
	}

	if (s.solved())
	{
		s.setok(true);
		return s;
	}

	int k = s.lowestPossibility();

	for (int i = 1; i <= 9; i++)
	{

		// ha a k. cellaban az i. ertek valaszthato(isPossible)
		if (s.isPossible(k, i))
		{

			// ha sikerul berakni es kiloni az elemeket akkor a kovetkezo szamot probaljuk
			// lemasoljuk az eredetit, hogy megvizsgaljuk a torlest, igy nem kel visszalepni
			Sudoku saux(s);

			if (saux.putIn(k, i))
			{
				Sudoku s_res = solve(saux);

				if (s_res.getok())
				{
					return s_res;
				}
			}
		}
	}
	Sudoku serr("");
	return serr;
}

vector<int> randomset_n(int n)
{
	vector<int> numbers;
	vector<int> poz;
	int r;

	for (int i = 0; i < 81; i++)
	{
		numbers.push_back(i);
	}

	for (int i = 0; i < n; i++)
	{
		do
		{
			r = rand() % numbers.size();
		} while (find(poz.begin(), poz.end(), r) != poz.end());
		poz.push_back(r);
	}
	return poz;
}

string _delete(vector<int> poz, string s)
{

	for (int i = 0; i < poz.size(); i++)
	{
		s[poz[i]] = '.';
	}
	return s;
}

string _generate(string s_solved, int n)
{
	string s_aux;
	do
	{
		vector<int> v = randomset_n(n);
		s_aux = _delete(v, s_solved);
	} while (!isSingleSolution(Sudoku(s_aux)));

	return s_aux;
}

void printStringForm(string s_in)
{
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << s_in[i * 9 + j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

string generate(int n, string type)
{

	Sudoku s_solved(starting_rand_sudoku());
	s_solved = solve(s_solved);

	if (type == "easy")
	{
		while (true)
		{
			string res_easy = _generate(s_solved.stringForm(), n);
			Sudoku s(res_easy);
			s = solve_diff(s);

			if (!s.getnaked() && !s.getptp())
			{
				return res_easy;
			}
		}
	}
	else if (type == "medium")
	{
		while (true)
		{
			string res_medium = _generate(s_solved.stringForm(), n);
			Sudoku s(res_medium);
			s = solve_diff(s);

			if (s.getnaked() && !s.getptp())
			{
				// ha alkalmaztunk naked pairt
				return res_medium;
			}
		}
	}
	else
	{
		// hard
		while (true)
		{
			string res_hard = _generate(s_solved.stringForm(), n);
			Sudoku s(res_hard);
			s = solve_diff(s);

			if (s.getptp() && s.getnaked())
			{ // ha alkalmaztunk pointingpairst
				return res_hard;
			}
		}
	}
	return "";
}

int main()
{
	srand(time(NULL));
	auto start = std::chrono::high_resolution_clock::now();
	Sudoku::setup();
	string s;
	fstream file2("easy50.txt");

	while (file2 >> s)
	{
		Sudoku s_egy(s);

		if (isSingleSolution(s_egy))
		{
			s_egy = solve(s_egy);
			cout << "Single solution sudoku!" << endl;
		}
		else
		{
			cout << "Multiple solution sudoku!" << endl;
		}
	}

	// cout << "----------------------------" << endl;

	// Generate random sudoku

	// string sgen = generate(50, "hard");
	// printStringForm(sgen);

	// Sudoku res = solve(res);
	// res.kiir();

	// cout << "-----------------------------" << endl;

	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<long double> elapsed = finish - start;
	std::cout << "Elapsed time: " << std::setprecision(10) << elapsed.count() << " s\n";

	return 0;
}