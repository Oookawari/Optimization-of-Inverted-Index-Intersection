/**********************************************************************
 * \version1.cpp
 * \brief 最初串行版本，用于提供结果的正确性检验
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

/*结果文件*/
ofstream ofile;

/*按元素求交*/
void Intsersection_ByElement();

/*按列表求交*/
void Intsersection_ByList();

/*查询vector中是否包含某元素*/
bool vector_find(vector<unsigned int> v, unsigned int element);

/*结果文件写入*/
void res_writing(string file);

/*重载比较函数*/
bool cmp(pair<int, int> a, pair<int, int> b);

int main() {

	/*打开index文件*/
	fstream index_file;
	index_file.open(INDEX_FILEPATH, ios::binary | ios::in);
	if (!index_file.is_open()) {
		cout << "打开Index文件错误！" << endl;
		return 0;
	}

	/*读取index文件至内存*/
	for (int i = 0; i <= 1755; i++) {
		int length;
		index_file.read((char*)&length, sizeof(length));
		vector<unsigned int> temp_list;
		for (int j = 0; j < length; j++) {
			unsigned int next_int;
			index_file.read((char*)&next_int, sizeof(next_int));
			temp_list.push_back(next_int);
		}
		index_lists.push_back(temp_list);
	}
	index_file.close();

	/*打开query文件*/
	ifstream query_file;
	query_file.open(QUERY_FILEPATH, ios::in);
	if (!query_file.is_open()) {
		cout << "打开Query文件错误！" << endl;
		return 0;
	}

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

	Intsersection_ByElement();

	res_writing(OUTPUT_FILE_1);

	res_lists.clear();

	Intsersection_ByList();

	res_writing(OUTPUT_FILE_2);

	return 0;
}

void Intsersection_ByElement()
{
	cout << "正在运行：按元素求交..." << endl;
	timespec_get(&sts, TIME_UTC);
	for (int n = 0; n < QUERY_NUM; n++) {
		vector<vector<unsigned int>> lists;

		/*每个倒排列表总长度*/
		int* list_length = new int[query[n][0]];

		/*每个倒排列表未探索长度*/
		int* list_undetected = new int[query[n][0]];

		/*列表下标与长度的映射*/
		pair<int, int>* list_order = new pair<int, int>[query[n][0]];

		/*初始化各个列表功能数组*/
		for (int i = 0; i < query[n][0]; i++) {
			lists.push_back(index_lists[query[n][i + 1]]);
			list_length[i] = lists[i].size();
			list_undetected[i] = 0;
			list_order[i] = pair<int, int>(list_length[i], i);
		}
		/*结果集合S*/
		vector<unsigned int> S;

		/*列表未探查长度排序*/
		sort(list_order, list_order + query[n][0], cmp);
		bool flag = false;
		while (list_order[0].first != 0 && !flag) {
			/*挑出第一个未探查元素*/
			int element = lists[list_order[0].second][list_undetected[list_order[0].second]];
			list_undetected[list_order[0].second]++;
			for (int i = 1; i < query[n][0]; i++) {
				int now_list = list_order[i].second;
				/*标记是否找到该元素*/
				bool found = false;
				/*列表未探查长度排序，遍历的同时更新其他数组的探查指针*/
				for (; list_undetected[now_list] < lists[now_list].size(); list_undetected[now_list]++) {
					if (element == lists[now_list][list_undetected[now_list]]) {
						found = true;
						break;
					}
					if (element < lists[now_list][list_undetected[now_list]]) {
						found = false;
						break;
					}
				}
				if (found == false) break;
				/*加入结果集合*/
				if (i == query[n][0] - 1) {
					S.push_back(element);
				}
			}
			/*更新列表未探查长度*/
			for (int i = 0; i < query[n][0]; i++) {
				list_order[i].first = list_length[list_order[i].second] - list_undetected[list_order[i].second];
			}
			/*列表未探查长度排序*/
			sort(list_order, list_order + query[n][0], cmp);
		}
		delete[]list_length;
		delete[]list_undetected;
		delete[]list_order;
		res_lists.push_back(S);
	}
	timespec_get(&ets, TIME_UTC);
	time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	cout << "按元素用时： " << time_sum << "毫秒" << endl;
}

void Intsersection_ByList()
{
	cout << "正在运行：按列表求交..." << endl;
	timespec_get(&sts, TIME_UTC);
	for (int n = 0; n < QUERY_NUM; n++) {
		cout << "计算请求" << n << endl;
		/*第一个列表做为初始结果集合*/
		vector<unsigned int> S = index_lists[query[n][1]];

		for (int i = 2; i <= query[n][0]; i++) {
			/*遍历结果集合S*/
			for (int j = 0; j < S.size(); j++) {
				/*查找元素是否存在*/
				if (!vector_find(index_lists[query[n][i]], S[j])) {
					S.erase(S.begin() + j);
					j--;
					continue;
				}
			}
		}

		res_lists.push_back(S);
	}
	timespec_get(&ets, TIME_UTC);
	time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	cout << "按列表用时： " << time_sum << "毫秒" << endl;
	return;

}

bool vector_find(vector<unsigned int> v, unsigned int element) {
	vector<unsigned int>::iterator it;
	it = find(v.begin(), v.end(), element);
	if (it != v.end())
		return true;
	else
		return false;
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