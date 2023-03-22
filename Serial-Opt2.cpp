
/**********************************************************************
 * \version5.cpp
 * \最优串行算法
 *
 * \author 2012516
 * \date   Jul 2022
***********************************************************************/

#include<iostream>
#include<fstream>
#include<string>
#include <vector>
#include <algorithm>
using namespace std;

const string INDEX_FILEPATH = "ExpIndex";
const string QUERY_FILEPATH = "ExpQuery.txt";
const string OUTPUT_FILE_1 = "res_1.txt";
const string OUTPUT_FILE_2 = "res_2.txt";
const int QUERY_NUM = 1000;
const int MAX_KEY_NUM = 5;

struct timespec sts, ets;
double time_sum = 0.0;

/*倒排列表集合*/
vector<vector<unsigned int>> index_lists;

/*求交结果集合*/
vector<vector<unsigned int>> res_lists;

/*第一列保存keywords数量*/
int query[QUERY_NUM][MAX_KEY_NUM + 1];

/*结果文件写入*/
ofstream ofile;

void Intersection_ByElement();

void Intersection_ByList();

void res_writing(string file);

bool cmp(pair<int, int> a, pair<int, int> b);

int main() {

	/*打开index文件*/
	fstream index_file;
	index_file.open(INDEX_FILEPATH, ios::binary | ios::in);
	if (!index_file.is_open()) {
		cout << "打开Index文件错误！" << endl;
		return 0;
	}


	cout << "正在运行：读取index..." << endl;
	/*读取index文件至内存*/

	for (int i = 0; i <= 1755; i++) {
		int length;
		index_file.read((char*)&length, sizeof(length));
		vector<unsigned int> temp_list;
		for (int j = 0; j < length; j++) {
			unsigned int next_int;
			index_file.read((char*)&next_int, sizeof(next_int));//索引数组内容
			temp_list.push_back(next_int);
		}
		index_lists.push_back(temp_list);
		for (int j = 0; j < temp_list.size() - 1;j++) {
			if (temp_list[j] < temp_list[j + 1]) continue;
			cout << "未排序！" << endl;
		}
	}
	index_file.close();

	/*打开query文件*/
	ifstream query_file;
	query_file.open(QUERY_FILEPATH, ios::in);
	if (!query_file.is_open()) {
		cout << "打开Query文件错误！" << endl;
		return 0;
	}

	cout << "正在运行：读取query..." << endl;
	/*读取query文件至内存*/
	for (int i = 0; i < QUERY_NUM; i++) {
		int temp;
		int j = 1;
		while (query_file >> temp) {
			query[i][j] = temp;
			if (query_file.get() == '\n') {
				query[i][0] = j;
				break;
			}
			j++;
		}
	}
	query_file.close();

	Intersection_ByElement();

	res_writing(OUTPUT_FILE_1);

	res_lists.clear();
	vector<vector<unsigned int>> res_temp(QUERY_NUM);
	res_lists = res_temp;
	Intersection_ByList();

	res_writing(OUTPUT_FILE_2);

	return 0;
}

void Intersection_ByElement()
{
	cout << "正在运行：按元素求交..." << endl;
	timespec_get(&sts, TIME_UTC);
	for (int n = 0; n < QUERY_NUM; n++) {
		//vector<vector<unsigned int>> lists;
		//int lists[5][30000];

		/*每个倒排列表总长度*/
		int* list_length = new int[query[n][0]];

		/*每个倒排列表未探索长度*/
		int* list_undetected = new int[query[n][0]];

		/*列表下标与长度的映射<长度，下标>*/
		pair<int, int>* list_order = new pair<int, int>[query[n][0]];

		/*列表下标与index下标映射*/
		int* list_to_index = new int[query[n][0]];

		for (int i = 0; i < query[n][0]; i++) {
			/*
			for (int j = 0; j < index_lists[query[n][i + 1]].size(); j++) {
				lists[i][j] = index_lists[query[n][i + 1]][j];
			}*/
			list_length[i] = index_lists[query[n][i + 1]].size();
			list_undetected[i] = 0;
			list_order[i] = pair<int, int>(list_length[i], i);
			list_to_index[i] = query[n][i + 1];
		}
		vector<unsigned int> S;
		sort(list_order, list_order + query[n][0], cmp);
		bool flag = false;
		while (list_order[0].first != 0 && !flag) {
			//挑出第一个未探查的元素
			int element = index_lists[list_to_index[list_order[0].second]][list_undetected[list_order[0].second]];
			list_undetected[list_order[0].second]++;
			for (int i = 1; i < query[n][0]; i++) {
				int now_list = list_order[i].second;
				bool found = false;
				//在其他表中未探查的元素开始，寻找该元素是否存在
				for (; list_undetected[now_list] < list_length[now_list]; list_undetected[now_list]++) {
					if (element == index_lists[list_to_index[now_list]][list_undetected[now_list]]) {
						found = true;
						break;
					}
					if (element < index_lists[list_to_index[now_list]][list_undetected[now_list]]) {
						found = false;
						break;
					}
				}
				if (found == false) break;
				if (i == query[n][0] - 1) {
					S.push_back(element);
				}
			}
			//更新未探查个数、列表长度排序
			for (int i = 0; i < query[n][0]; i++) {
				list_order[i].first = list_length[list_order[i].second] - list_undetected[list_order[i].second];
			}
			sort(list_order, list_order + query[n][0], cmp);
		}
		delete[]list_length;
		delete[]list_undetected;
		delete[]list_order;
		delete[]list_to_index;
		res_lists.push_back(S);
	}
	timespec_get(&ets, TIME_UTC);
	time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	cout << "按元素处理求交用时： " << time_sum << "毫秒" << endl;
}



bool cmp(pair<int, int>a, pair<int, int>b) {
	return a.first < b.first;
}

void res_writing(string file)
{
	cout << "正在运行： " + file + " 文件写入..." << endl;
	if (!ofile.is_open()) {
		ofile = ofstream(file, ios::out);
	}
	if (!ofile.is_open()) {
		cout << "打开输出文件错误！" << endl;
	}
	for (int n = 0; n < QUERY_NUM; n++) {
		ofile << "keywords: ";
		for (int i = 1; i <= query[n][0];i++)
			ofile << query[n][i] << " ";
		ofile << endl << "result: ";
		for (int i = 0; i < res_lists[n].size();i++)
			ofile << res_lists[n][i] << " ";
		ofile << endl;
	}
	ofile.close();
	return;
}

void Intersection_ByList() {
	/*struct timespec sts1, ets1;
	timespec_get(&sts1, TIME_UTC);*/
	for (int n = 0; n < QUERY_NUM; n++) {
		vector<unsigned int> S = index_lists[query[n][1]];
		bool* marks = new bool[S.size()];
		for (int i = 0; i < S.size(); i++) {
			marks[i] = false;
		}
		for (int i = 2; i <= query[n][0];i++) {
			int detected = 0;
			for (int j = 0; j < S.size(); j++) {
				if (marks[j]) continue;
				bool found = false;
				for (;detected < index_lists[query[n][i]].size();detected++) {
					if (S[j] == index_lists[query[n][i]][detected]) {
						found = true;
						break;
					}
					if (S[j] < index_lists[query[n][i]][detected]) {
						break;
					}
				}
				if (!found) {
					marks[j] = true;
					continue;
				}
			}
		}
		vector<unsigned int> S2;
		for (int i = 0; i < S.size(); i++) {
			if (!marks[i]) S2.push_back(S[i]);
		}
		delete[]marks;
		res_lists[n] = S2;
	}
	return;
}