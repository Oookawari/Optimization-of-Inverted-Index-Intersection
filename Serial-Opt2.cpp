
/**********************************************************************
 * \version5.cpp
 * \���Ŵ����㷨
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

/*�����б���*/
vector<vector<unsigned int>> index_lists;

/*�󽻽������*/
vector<vector<unsigned int>> res_lists;

/*��һ�б���keywords����*/
int query[QUERY_NUM][MAX_KEY_NUM + 1];

/*����ļ�д��*/
ofstream ofile;

void Intersection_ByElement();

void Intersection_ByList();

void res_writing(string file);

bool cmp(pair<int, int> a, pair<int, int> b);

int main() {

	/*��index�ļ�*/
	fstream index_file;
	index_file.open(INDEX_FILEPATH, ios::binary | ios::in);
	if (!index_file.is_open()) {
		cout << "��Index�ļ�����" << endl;
		return 0;
	}


	cout << "�������У���ȡindex..." << endl;
	/*��ȡindex�ļ����ڴ�*/

	for (int i = 0; i <= 1755; i++) {
		int length;
		index_file.read((char*)&length, sizeof(length));
		vector<unsigned int> temp_list;
		for (int j = 0; j < length; j++) {
			unsigned int next_int;
			index_file.read((char*)&next_int, sizeof(next_int));//������������
			temp_list.push_back(next_int);
		}
		index_lists.push_back(temp_list);
		for (int j = 0; j < temp_list.size() - 1;j++) {
			if (temp_list[j] < temp_list[j + 1]) continue;
			cout << "δ����" << endl;
		}
	}
	index_file.close();

	/*��query�ļ�*/
	ifstream query_file;
	query_file.open(QUERY_FILEPATH, ios::in);
	if (!query_file.is_open()) {
		cout << "��Query�ļ�����" << endl;
		return 0;
	}

	cout << "�������У���ȡquery..." << endl;
	/*��ȡquery�ļ����ڴ�*/
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
	cout << "�������У���Ԫ����..." << endl;
	timespec_get(&sts, TIME_UTC);
	for (int n = 0; n < QUERY_NUM; n++) {
		//vector<vector<unsigned int>> lists;
		//int lists[5][30000];

		/*ÿ�������б��ܳ���*/
		int* list_length = new int[query[n][0]];

		/*ÿ�������б�δ̽������*/
		int* list_undetected = new int[query[n][0]];

		/*�б��±��볤�ȵ�ӳ��<���ȣ��±�>*/
		pair<int, int>* list_order = new pair<int, int>[query[n][0]];

		/*�б��±���index�±�ӳ��*/
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
			//������һ��δ̽���Ԫ��
			int element = index_lists[list_to_index[list_order[0].second]][list_undetected[list_order[0].second]];
			list_undetected[list_order[0].second]++;
			for (int i = 1; i < query[n][0]; i++) {
				int now_list = list_order[i].second;
				bool found = false;
				//����������δ̽���Ԫ�ؿ�ʼ��Ѱ�Ҹ�Ԫ���Ƿ����
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
			//����δ̽��������б�������
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
	cout << "��Ԫ�ش�������ʱ�� " << time_sum << "����" << endl;
}



bool cmp(pair<int, int>a, pair<int, int>b) {
	return a.first < b.first;
}

void res_writing(string file)
{
	cout << "�������У� " + file + " �ļ�д��..." << endl;
	if (!ofile.is_open()) {
		ofile = ofstream(file, ios::out);
	}
	if (!ofile.is_open()) {
		cout << "������ļ�����" << endl;
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