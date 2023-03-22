
/**********************************************************************
 * \version6.cpp
 * \pthread ���б��� ���β�ѯ�Ż�
 *
 * \author 2012516
 * \date   Jul 2022
***********************************************************************/

#include<iostream>
#include<fstream>
#include<string>
#include <vector>
#include <algorithm>
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")

using namespace std;

typedef struct {
	int t_id;
}threadparam_t;
pthread_barrier_t barrier;

const string INDEX_FILEPATH = "ExpIndex";
const string QUERY_FILEPATH = "ExpQuery.txt";
const string OUTPUT_FILE_1 = "res_1.txt";
const string OUTPUT_FILE_2 = "res_2.txt";
const int QUERY_NUM = 1000;
const int MAX_KEY_NUM = 5;
const int NUM_THREADS = 8;

struct timespec sts, ets;
double time_sum = 0.0;

/*�����б���*/
vector<vector<unsigned int>> index_lists;

/*�󽻽������*/
vector<vector<unsigned int>> res_lists(QUERY_NUM);

vector<unsigned int> S1;

/*��һ�б���keywords����*/
int query[QUERY_NUM][MAX_KEY_NUM + 1];

/*����ļ�д��*/
ofstream ofile;

bool marks[30000];

void* Intersection_ByElement(void* param);

void* Intersection_ByList(void* param);

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

	timespec_get(&sts, TIME_UTC);
	pthread_t handles[NUM_THREADS];
	threadparam_t param[NUM_THREADS];
	pthread_barrier_init(&barrier, NULL, NUM_THREADS);

	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		param[t_id].t_id = t_id;
		pthread_create(&handles[t_id], NULL, Intersection_ByElement, (void*)&param[t_id]);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(handles[i], NULL);
	}
	pthread_barrier_destroy(&barrier);

	timespec_get(&ets, TIME_UTC);
	time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	cout << "��Ԫ�ش�������ʱ�� " << time_sum << "����" << endl;

	res_writing(OUTPUT_FILE_1);

	res_lists.clear();/*

	timespec_get(&sts, TIME_UTC);
	pthread_t handles[NUM_THREADS];
	threadparam_t param[NUM_THREADS];
	pthread_barrier_init(&barrier, NULL, NUM_THREADS);

	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		param[t_id].t_id = t_id;
		pthread_create(&handles[t_id], NULL, Intersection_ByList, (void*)&param[t_id]);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(handles[i], NULL);
	}
	pthread_barrier_destroy(&barrier);

	timespec_get(&ets, TIME_UTC);
	time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	cout << "��Ԫ�ش�������ʱ�� " << time_sum << "����" << endl;

	res_writing(OUTPUT_FILE_2);
*/
	return 0;
}

void* Intersection_ByElement(void* param)
{
	threadparam_t* p = (threadparam_t*)param;
	int t_id = p->t_id;

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
				for (; list_undetected[now_list] < list_length[now_list]; list_undetected[now_list] += NUM_THREADS) {
					if (element == index_lists[list_to_index[now_list]][list_undetected[now_list]]) {
						found = true;
						break;
					}
					if (element < index_lists[list_to_index[now_list]][list_undetected[now_list]]) {
						found = false;
						break;
					}
				}
				pthread_barrier_wait(&barrier);
				if (found == false) {
					list_undetected[now_list] -= NUM_THREADS;
					break;
				}
				if (i == query[n][0] - 1) {
					S1.push_back(element);
				}
			}
		}
		delete[]list_length;
		delete[]list_undetected;
		delete[]list_order;
		delete[]list_to_index;
		if (t_id == 0) {
			res_lists.push_back(S1);
		}
		pthread_barrier_wait(&barrier);
	}

	/*timespec_get(&ets1, TIME_UTC);
		time_sum = (ets1.tv_nsec - sts1.tv_nsec) * 0.000001 + (ets1.tv_sec - sts1.tv_sec) * 1000;
		cout << "thread" << t_id << "��ʱ�� " << time_sum << " ����" << endl;*/
	pthread_exit(NULL);
	return NULL;
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

void* Intersection_ByList(void* param) {
	/*struct timespec sts1, ets1;
	timespec_get(&sts1, TIME_UTC);*/
	threadparam_t* p = (threadparam_t*)param;
	int t_id = p->t_id;

	for (int n = 0; n < QUERY_NUM; n++) {
		vector<unsigned int> S = index_lists[query[n][1]];
		/*ÿ���߳�ѭ��ʱ�����Լ�Ҫ����������±����marks���*/
		for (int i = t_id; i < S.size(); i += NUM_THREADS) {
			marks[i] = false;
		}
		for (int i = 2; i <= query[n][0];i++) {
			int detected = 0;
			/*ѭ�����ֵķ�ʽ����ÿ���߳���Ҫ���ҵ�Ԫ��*/
			for (int j = t_id; j < S.size(); j += NUM_THREADS) {
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
		/*ÿ����������ͬ��*/
		pthread_barrier_wait(&barrier);
		if (t_id == 0) {
			vector<unsigned int> S2;
			for (int i = 0; i < S.size(); i++) {

				if (!marks[i]) S2.push_back(S[i]);
			}
			res_lists[n] = S2;
		}
		pthread_barrier_wait(&barrier);
	}
	pthread_exit(NULL);
	return NULL;
}