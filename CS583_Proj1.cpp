#include <afxdb.h>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <vector>
#include <list>
#include <queue>
#include <map>
using namespace std;

struct Item //store item, MIS, # of items in T-set
{
	int item;
	double mis;
	int supCount;
};
struct Iset //as an element { }, which include items
{
	vector <Item> items;
	int count;
	int tailcount;
};
struct FreqSet //a k-frequent set has some { } with size = k
{
	vector <Iset> element;
	int kfreq;
};
struct Tset //a transaction
{
	vector <Item> trans;
	int TID;
};
struct Cos //constraint
{
	vector <int> constraint;
};
bool a_misless_b(const Item &a, const Item &b)
{
	return a.mis < b.mis;
}
bool a_less_b(const Item &a, const Item &b)
{
	return a.item < b.item;
}

vector <FreqSet> UC, UF; //union of Ck set and Fk set
vector <Item> M; //item set
vector <Tset> T; //transaction set
vector <Item> L;
vector <Item> MS; //MIS rate sets
vector <Cos> CONT; //Constraint sets
vector <int> Must; //Must have sets
double SDC; //SDC value
int tN;
time_t totalTime = 0;

void readInput();
void readPara();
void genMset(vector <Item>& MIS, vector <Tset>& T);
void genLset(vector <Item>& M);
FreqSet genF1(vector <Item>& L);
void genF();
bool checkc(Tset t, Iset c);
bool checkc_1(Tset t, Iset c);
bool checks(Iset& s, FreqSet Fk_1);
bool chkElement(Iset& f1, Iset& f2);
void dGenF();
bool checkConst(Iset c);
bool kminus1Freqempty(int k);
FreqSet L2Gen(vector <Item>& L, double SDC);
FreqSet MSGen(FreqSet Fk_1, double SDC);

void main(int argc, char *argv[])
{
	time_t start, end;
	//initialize all items
	start = clock();
	UC.clear();
	//read input data (items + parameter files)
	T.clear();
	end = clock();
	totalTime += double(end - start) * 1000 / CLOCKS_PER_SEC;
	readInput(); //read items
	start = clock();
	//read parameters
	MS.clear();
	CONT.clear();
	Must.clear();
	SDC = 0.0;
	end = clock();
	totalTime += double(end - start) * 1000 / CLOCKS_PER_SEC;
	readPara(); //read parameter file
	start = clock();
	std::cout << endl << endl << endl;
	//count all items and sort to creat M-set;
	M.clear();
	genMset(MS, T); //generate M set;
	tN = T.size(); //# of transaction, n
	//generate L set
	L.clear();
	genLset(M);
	//generate F1 set
	FreqSet F1;
	F1.element.clear();
	F1 = genF1(L);
	UF.clear();
	UF.push_back(F1);
	genF(); //generate F, C set with k >= 2
	dGenF();
	end = clock();
	totalTime += double(end - start) * 1000 / CLOCKS_PER_SEC;
	printf("the running time is (ms) : %f\n", double(totalTime) * 1000/ CLOCKS_PER_SEC);
	std::cout << "Successfully done!\a" << endl;
	cin.get();
}

void readInput()
{
	bool filetest = false;
	string filename;
	std::cout << "Please enter the input file name: " << endl;
	getline(cin, filename);
	while (!filetest)
	{
		ifstream fin(filename, ios::_Nocreate);
		if (!fin)
		{
			std::cout << "File does not exist, please try again:" << endl;
			filetest = false;
			getline(cin, filename);
		}
		else
		{
			std::cout << "Reading input file correct." << endl;
			filetest = true;
			continue;
		}
	}
	time_t start, end;
	start = clock();
	char *buffer = new char[100000];
	fstream out;
	out.open(filename, ios::in);
	int id = 1;
	while (!out.eof())
	{
		Tset t;
		t.trans.clear();
		t.TID = id;
		out.getline(buffer, 100000, '\n');
		string line;
		line = buffer;
		replace(line.begin(), line.end(), ',', ' ');
		replace(line.begin(), line.end(), '{', ' ');
		replace(line.begin(), line.end(), '}', ' ');
		const char *cc = line.c_str();
		char ss[100000];
		strcpy(ss, cc);
		const char *d = " ,{}";
		char *p;
		p = strtok(ss, d);
		while (p)
		{
			Item it;
			int k = atoi(p);
			it.item = k;
			t.trans.push_back(it);
			p = strtok(NULL, d);
		}
		T.push_back(t);
		id++;
	}
	out.close();
	end = clock();
	totalTime += double(end - start) * 1000 / CLOCKS_PER_SEC;
}

void readPara()
{
	bool filetest = false;
	string filename;
	std::cout << "Please enter the parameter file name: " << endl;
	getline(cin, filename);
	while (!filetest)
	{
		ifstream fin(filename, ios::_Nocreate);
		if (!fin)
		{
			std::cout << "File does not exist, please try again:" << endl;
			filetest = false;
			getline(cin, filename);
		}
		else
		{
			std::cout << "Reading parameter file correct." << endl;
			filetest = true;
			continue;
		}
	}
	time_t start, end;
	start = clock();
	char *line = new char[100000];
	ifstream out;
	out.open(filename, ios::in);
	char *lf;
	while (out.getline(line, 100000))
	{
		lf = line;
		if (strstr(line, "MIS") != NULL) //read MIS
		{
			Item it;
			while (*lf != '(' && *lf != NULL)
				lf++;
			it.item = int(atof(lf + 1));
			while (*lf != '=' && *lf != NULL)
				lf++;
			it.mis = double(atof(lf + 1));

			MS.push_back(it);
		}
		if (strstr(line, "SDC") != NULL) //read SDC
		{
			while (*lf != '=' && *lf != NULL)
				lf++;
			SDC = double(atof(lf + 1));
		}
		if (strstr(line, "cannot_be_together") != NULL) //read constraint rule
		{
			const char *d = "{}";
			char *p;
			vector <char*> gp;
			gp.clear();
			p = strtok(line, d);
			while (p)
			{
				char *c1 = "cannot_be_together: ";
				char *c2 = ", ";
				if (strcmp(p, c1) != 0 && strcmp(p, c2) != 0)
					gp.push_back(p);
				p = strtok(NULL, d);
			}		//end while (p)
			for (auto size = 0; size < gp.size(); size++)
			{
				Cos cos;
				cos.constraint.clear();
				char* p1 = gp[size];
				//cout << p1 << endl;
				const char *d2 = " ,";
				char *p2;
				p2 = strtok(p1, d2);
				while (p2)
				{
					int k = atoi(p2);
					cos.constraint.push_back(k);
					p2 = strtok(NULL, d2);
				}
				CONT.push_back(cos);
			}
		}
		if (strstr(line, "must-have") != NULL) //read must have rule
		{
			const char *d = ":";
			const char *c1 = "must-have";
			char *p;
			p = strtok(line, d);
			while (p)
			{
				if (strcmp(p, c1) != 0)
				{
					char *d2 = " or";
					char *p2 = strtok(p, d2);
					while (p2)
					{
						int k = atoi(p2);
						Must.push_back(k);
						p2 = strtok(NULL, d2);
					}
				}
				p = strtok(NULL, d);
			}
		}
	}
	end = clock();
	totalTime += double(end - start) * 1000 / CLOCKS_PER_SEC;
}

void genMset(vector <Item>& MS, vector <Tset>& T)
{
	sort(MS.begin(), MS.end(), a_less_b);
	for (auto it = MS.begin(); it != MS.end(); it++)
	{
		Item i;
		i.item = (*it).item;
		i.mis = (*it).mis;
		i.supCount = 0;
		M.push_back(i);
	}
	for (auto it1 = T.begin(); it1 != T.end(); it1++)
		for (auto it2 = (*it1).trans.begin(); it2 != (*it1).trans.end(); it2++)
			for (auto it3 = M.begin(); it3 != M.end(); it3++)
				if ((*it2).item == (*it3).item)
					(*it3).supCount++;	
	stable_sort(M.begin(), M.end(), a_misless_b);	
}

void genLset(vector <Item>& M)
{
	for (auto it = M.begin(); it != M.end(); it++)
		if (double(double((*it).supCount) / double(tN)) >= (*it).mis) //Find the first item
		{
			L.push_back(*it); //push into L
			for (auto it2 = it + 1; it2 != M.end(); it2++) //for all items after, if count/n >first[mis], push into L.
				if (double(double((*it2).supCount) / double(tN)) >= (*it).mis)
					L.push_back(*it2);
			break; //after checking all items after first item, stop
		}
}

FreqSet genF1(vector <Item>& L)
{
	FreqSet F;
	F.element.clear();
	F.kfreq = 1;
	for (auto it = L.begin(); it != L.end(); it++)
	{
		Iset iset;
		iset.items.clear();
		iset.count = 0;
		iset.tailcount = 0;
		if (double((*it).supCount) / double(tN) >= (*it).mis)
		{
			iset.items.push_back(*it);
			iset.count = (*it).supCount;
			F.element.push_back(iset);
		}
	}
	return F;
}

void genF()
{
	int k = 2;
	while (kminus1Freqempty(k))
	{
		FreqSet C_k;
		C_k.element.clear();
		C_k.kfreq = 0;
		if (k == 2)
		{
			FreqSet C2;
			C2.kfreq = k;
			C2.element.clear();
			C2 = L2Gen(L, SDC);
			C_k = C2;
			UC.push_back(C2);
		}
		else
		{
			FreqSet Ck, Fk_1;
			Ck.kfreq = k;
			Ck.element.clear();
			for (auto it = UF.begin(); it != UF.end(); it++)
				if ((*it).kfreq == k-1)
				{
					Fk_1 = (*it);
					break;
				}
			Ck = MSGen(Fk_1, SDC);			
			C_k = Ck;
			UC.push_back(Ck);
		}
		//count and tail count
		for (auto c = C_k.element.begin(); c != C_k.element.end(); c++)
		{
			(*c).count = 0;
			(*c).tailcount = 0;
		}
		for (auto t = T.begin(); t != T.end(); t++)
			for (auto c = C_k.element.begin(); c != C_k.element.end(); c++)
			{
				if (checkc((*t), (*c)))
					(*c).count += 1;
				if (checkc_1((*t), (*c)))
					(*c).tailcount += 1;
			}				
		//check c into Fk
		FreqSet Fk;
		Fk.element.clear();
		Fk.kfreq = k;
		for (auto c = C_k.element.begin(); c != C_k.element.end(); c++)
			if (double((*c).count) / double(tN) >= (*c).items[0].mis)
				Fk.element.push_back(*c);
		UF.push_back(Fk);
		k++;
	}
}

void dGenF() //DISPLAY FINAL RESULT
{
	ofstream fout("output.txt", std::ios::out | std::ios::app);
	for (auto f = UF.begin(); f != UF.end(); f++)
	{
		std::cout << "Frequent " << (*f).kfreq << "-itemsets" << endl;
		fout << "Frequent " << (*f).kfreq << "-itemsets" << endl;
		std::cout << endl;
		fout << endl;
		//cout << "original size: " << (*f).element.size() << endl;
		int size = 0;
		if ((*f).kfreq > 1)
			for (auto it = (*f).element.begin(); it != (*f).element.end(); it++)
				if (checkConst(*it))
				{
					std::cout << "        " << (*it).count << " : {";
					fout << "        " << (*it).count << " : {";
					for (auto it2 = (*it).items.begin(); it2 != (*it).items.end(); it2++)
						if (it2 != (*it).items.end() - 1)
						{
							std::cout << (*it2).item << ", ";
							fout << (*it2).item << ", ";
						}
						else
						{
							std::cout << (*it2).item << "}" << endl;
							fout << (*it2).item << "}" << endl;
						}				
					std::cout << "Tailcount = " << (*it).tailcount << endl;
					fout << "Tailcount = " << (*it).tailcount << endl;
					size++;
				}	
		if ((*f).kfreq == 1)
			for (auto it = (*f).element.begin(); it != (*f).element.end(); it++)
				if (checkConst(*it))
				{
					std::cout << "        " << (*it).count << " : {";
					fout << "        " << (*it).count << " : {";
					for (auto it2 = (*it).items.begin(); it2 != (*it).items.end(); it2++)
						if (it2 != (*it).items.end() - 1)
						{
							std::cout << (*it2).item << ", ";
							fout << (*it2).item << ", ";
						}
						else
						{
							std::cout << (*it2).item << "}" << endl;
							fout << (*it2).item << "}" << endl;
						}
					size++;
				}
		std::cout << endl;
		fout << endl;
		std::cout << "        Total number of frequent " << (*f).kfreq << "-itemsets = " << size << endl;
		fout << "        Total number of frequent " << (*f).kfreq << "-itemsets = " << size << endl;

		if (size < (*f).element.size() && (*f).element.size() > 0)
			std::cout << "        The original total number of frequent " << (*f).kfreq << "-itemsets = " << (*f).element.size() << " ,by constraint rules, " << (*f).element.size() - size << " itemsets have been pruned." << endl;
		std::cout << endl << endl;
		fout << endl << endl;
	}
}

bool checkConst(Iset c)
{
	bool pass = true;
	bool must = false;
	bool cont = false;
	//check must-have
	int matchMust = 0;
	for (auto it = c.items.begin(); it != c.items.end(); it++)
	{
		if (matchMust > 0)
			break;
		for (auto m = Must.begin(); m != Must.end(); m++)
			if ((*it).item == (*m))
			{
				matchMust += 1;
				break;
			}
	}
	if (matchMust > 0)
		must = true;
	else
		must = false;
	//check constraints
	int matchSize = 0; //for each constraint, # of matched items
	int matchCont = 0; //for all constraints, # of mateched constraints
	for (auto ct = CONT.begin(); ct != CONT.end(); ct++) //read each constraint
	{
		if (matchCont > 0)
			break;
		matchSize = 0;
		for (auto ict = (*ct).constraint.begin(); ict != (*ct).constraint.end(); ict++) //read each item in constraint
			for (auto it = c.items.begin(); it != c.items.end(); it++)
				if ((*it).item == (*ict))
				{
					matchSize += 1;
					break;
				}
		if (matchSize == (*ct).constraint.size())
		{
			matchCont += 1;
			break;
		}
	}
	if (matchCont > 0)
		cont = false;
	if (matchCont == 0)
		cont = true;
	if (cont == true && must == true)
		pass = true;
	else
		pass = false;
	return pass;
}

bool checkc(Tset t, Iset c)
{
	bool include = false;	
	int match = 0;
	for (auto i = t.trans.begin(); i != t.trans.end(); i++)
		for (auto j = c.items.begin(); j != c.items.end(); j++)
			if ((*j).item == (*i).item)
			{
				match++;
				break; //when one item match, can continue the next item
			}
	if (match == c.items.size())
		include = true;
	else
		include = false;
	return include;
}

bool checkc_1(Tset t, Iset c)
{
	bool include = false;	
	int match = 0;
	for (auto i = t.trans.begin(); i != t.trans.end(); i++)
		for (auto j = c.items.begin() + 1; j != c.items.end(); j++)
			if ((*j).item == (*i).item)
			{
				match++;
				break; //when one item match, can continue the next item
			}
	if (match == c.items.size() - 1)
		include = true;
	else
		include = false;
	return include;
}

FreqSet L2Gen(vector <Item>& L, double SDC)
{
	FreqSet Ck;
	Ck.element.clear();
	Ck.kfreq = 2;
	for (auto l = L.begin(); l != L.end(); l++)
		if (double((*l).supCount) / double(tN) >= (*l).mis)
			for (auto h = l + 1; h != L.end(); h++)
				if (double((*h).supCount) / double(tN) >= (*l).mis && abs(double((*h).supCount) / double(tN) - double((*l).supCount) / double(tN)) <= SDC)
				{
					Iset iset;
					iset.items.clear();
					iset.count = 0;
					iset.tailcount = 0;
					iset.items.push_back(*l);
					iset.items.push_back(*h);
					Ck.element.push_back(iset);
				}
	return Ck;
}

FreqSet MSGen(FreqSet Fk_1, double SDC)
{
	FreqSet Ck;
	Ck.element.clear();
	Ck.kfreq = Fk_1.kfreq + 1;
	for (auto it1 = Fk_1.element.begin(); it1 != Fk_1.element.end() - 1; it1++)
		for (auto it2 = it1 + 1; it2 != Fk_1.element.end(); it2++)
			if (chkElement((*it1), (*it2)))
			{
				Iset c;
				c.count = 0;
				c.tailcount = 0;			
				if ((*it1).items[Fk_1.kfreq - 1].mis < (*it2).items[Fk_1.kfreq - 1].mis && abs(double((*it1).items[Fk_1.kfreq - 1].supCount - (*it2).items[Fk_1.kfreq - 1].supCount)) <= SDC * tN) //ik_1 < ik_1' 
				{
					for (auto j = 0; j < Ck.kfreq - 2; j++)
						c.items.push_back((*it1).items[j]);
					c.items.push_back((*it1).items[Fk_1.kfreq - 1]);
					c.items.push_back((*it2).items[Fk_1.kfreq - 1]);
					Ck.element.push_back(c);
				}
				if ((*it1).items[Fk_1.kfreq - 1].mis >(*it2).items[Fk_1.kfreq - 1].mis && abs(double((*it1).items[Fk_1.kfreq - 1].supCount - (*it2).items[Fk_1.kfreq - 1].supCount)) <= SDC * tN)
				{
					for (auto j = 0; j < Ck.kfreq - 2; j++)
						c.items.push_back((*it1).items[j]);
					c.items.push_back((*it2).items[Fk_1.kfreq - 1]);
					c.items.push_back((*it1).items[Fk_1.kfreq - 1]);
					Ck.element.push_back(c);
				}
				if (c.items.size() == 0)
					continue;				
				Item c1; //check c
				c1 = c.items[0];
				double MISc1 = c.items[0].mis;
				double MISc2 = c.items[1].mis;
				bool MISc12;
				if (MISc1 == MISc2)
					MISc12 = true;
				else
					MISc12 = false;
				for (auto i = 0; i < c.items.size(); i++)
				{
					Iset s;
					s.items.clear();

					s.items.assign(c.items.begin(), c.items.end());
					s.items.erase(s.items.begin() + i);

					if (i != 0 || MISc12 == true)
						if (!checks(s, Fk_1))
						{
							Ck.element.pop_back();
							break;
						}
				}//end for (s)
			}//end each c
	return Ck;
}

bool checks(Iset& s, FreqSet Fk_1)
{
	bool include = false;
	for (auto f = Fk_1.element.begin(); f != Fk_1.element.end(); f++) //go through each elements f in F(k-1)
	{
		int match = 0;
		for (auto size = 0; size < s.items.size(); size++) //go through each item in each f
			if (s.items[size].item == (*f).items[size].item)
				match += 1;
			else
			{
				match = 0;
				include = false;
				break;
			}
		if (match == s.items.size())
		{
			include = true;
			break;
		}
	}
	return include;
}

bool chkElement(Iset& f1, Iset& f2)
{
	bool equal = true;

	int size1 = f1.items.size();
	int size2 = f2.items.size();
	if (size1 != size2)
	{
		equal = false;
		return equal;
	}
	else
		for (auto i = 0; i < size1; i++)
		{
			if (i < size1 - 1)
			{
				if (f1.items[i].item != f2.items[i].item)
				{
					equal = false;
					return equal;
					break;
				}
				else
					equal = true;
			}
			//cout << "k-1 elements are same. " << endl;
			if (i == size1 - 1)
				if (f1.items[i].item == f2.items[i].item)
					cout << "f1 = f2" << endl;	
		}
	return equal;
}

bool kminus1Freqempty(int k)
{
	bool empty = false;
	int size = k - 1;

	for (auto it = UF.begin(); it != UF.end(); it++)
		if ((*it).kfreq == size)
			if ((*it).element.size() > 0)
			{
				empty = true;
				break;
			}
			else
			{
				empty = false;
				break;
			}			
		else
			empty = false;			
	return empty;
}