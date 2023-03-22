/**********************************************************************
 * \version1.cpp
 * \brief bitmap unsigned int
 *
 * \author 2012516
 * \date   Jul 2022
***********************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
using namespace std;

struct timespec sts, ets;
double time_sum = 0.0;

const string INDEX_FILEPATH = "ExpIndex";
const string QUERY_FILEPATH = "ExpQuery.txt";
const string OUTPUT_FILE_1 = "res_1.txt";
const string OUTPUT_FILE_2 = "res_2.txt";
const int QUERY_NUM = 1000;
const int MAX_KEY_NUM = 5;

/*�����б���*/
vector<vector<unsigned int>> index_lists;

/*�󽻽������*/
vector<vector<unsigned int>> res_lists;

/*��һ�б���keywords����*/
int query[QUERY_NUM][MAX_KEY_NUM + 1];

/*����ļ�д��*/
ofstream ofile;

class vector_list {

public:
	vector<unsigned int> index;
	vector<unsigned int> Secondary;
	~vector_list() {
		return;
	};
	vector_list() {
		this->index.resize(787662);
		this->Secondary.resize(24615);
		return;
	};
	vector_list(const vector_list& ano) {
		this->index.resize(787662);
		this->Secondary.resize(24615);
		for (int i = 0; i < 787662; i++) {
			index[i] = ano.index[i];
		}
		for (int i = 0; i < 24615; i++) {
			Secondary[i] = ano.Secondary[i];
		}
		return;
	}
	/*�б��λ����ת��*/
	vector_list(int query_no) {
		this->index.resize(787662);
		this->Secondary.resize(24615);
		for (int i = 0; i < 787662; i++) {
			index[i] = 0;
		}
		for (int i = 0; i < 24615; i++) {
			Secondary[i] = 0;
		}
		for (int i = 0; i < index_lists[query_no].size(); i++) {
			/*index��1*/
			int temp = index_lists[query_no][i];
			int pos = temp / 32;//�����±�
			int bit = temp % 32;//λ
			unsigned int temp_bit = 1;
			temp_bit = temp_bit << bit;
			index[pos] = index[pos] | temp_bit;//��λ���������λ
			/*Secondary��1*/
			int pos_s = pos / 32;//�����±�
			int bit_s = pos % 32;//λ
			unsigned int temp_bit_s = 1;
			temp_bit_s = temp_bit_s << bit_s;
			Secondary[pos_s] = Secondary[pos_s] | temp_bit_s;//��λ���������λ
		}
		return;
	};
};

void vector_list_intersection();

void vector_and(vector_list& ori, vector_list& tem);

bool judge(unsigned int num, unsigned int pos) {
	unsigned int tool = 1;
	if (num & (tool << pos))
	{
		return true;
	}
	else
	{
		return false;
	}

}

void res_writing(string file);

int main() {

	/*��index�ļ�*/
	fstream index_file;
	index_file.open(INDEX_FILEPATH, ios::binary | ios::in);
	if (!index_file.is_open()) {
		cout << "��Index�ļ�����" << endl;
		return 0;
	}

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
	}
	index_file.close();
	/*��query�ļ�*/
	ifstream query_file;
	query_file.open(QUERY_FILEPATH, ios::in);
	if (!query_file.is_open()) {
		cout << "��Query�ļ�����" << endl;
		return 0;
	}

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
	vector_list_intersection();

	timespec_get(&ets, TIME_UTC);
	time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	cout << "λ������ʱ�� " << time_sum << "����" << endl;

	res_writing(OUTPUT_FILE_1);

	return 0;
}

void vector_list_intersection() {
	for (int n = 0; n < QUERY_NUM; n++) {
		/*ת��Ϊλ����*/
		vector<vector_list> vectors(query[n][0]);
		for (int i = 1; i <= query[n][0]; i++) {
			vectors[i - 1] = vector_list(query[n][i]);
		}

		/*0��λ��������������������λ��*/
		for (int i = 1; i < query[n][0]; i++) {
			vector_and(vectors[0], vectors[i]);
		}

		/*��ȡ���*/
		vector<unsigned int> S;
		for (int i = 0; i < 24615; i++) {
			if (vectors[0].Secondary[i] != 0) {
				unsigned int temp = vectors[0].Secondary[i];
				for (int j = 0; j < 32; j++) {
					/*�ҵ���Ϊ0��λ*/
					bool exist = judge(temp, j);
					if (exist) {
						//�ҵ���Ӧ��32λ
						unsigned int temp_index = vectors[0].index[i * 32 + j];
						for (int k = 0; k < 32; k++) {
							/*�ҵ���Ϊ0��λ*/
							bool exist_index = judge(temp_index, k);
							if (exist_index) {
								//ת��Ϊ��Ӧ��ֵ
								S.push_back((i * 32 + j) * 32 + k);
							}
						}
					}
				}
			}

		}
		res_lists.push_back(S);
	}
	return;
}

void vector_and(vector_list& ori, vector_list& tem) {
	for (int i = 0; i < 24615; i++) {
		/*����������λ��*/
		unsigned int temp = ori.Secondary[i] & tem.Secondary[i];
		if (temp == 0) {
			ori.Secondary[i] = 0;//���¶�������
			continue;
		}
		else {
			/*��Ӧλ��Ҫ��λ��*/
			unsigned int new_second = 0;//���������������µ�ֵ
			unsigned int new_second_temp = 1;
			for (int j = 0; j < 32; j++) {
				bool exist = judge(temp, j);
				if (exist) {
					ori.index[i * 32 + j] = tem.index[i * 32 + j] & ori.index[i * 32 + j];//��������λ��Ӧ��int��λ��
					if (ori.index[i * 32 + j] != 0) new_second = new_second | new_second_temp;//��λ���������λ
				}
				new_second_temp = new_second_temp << 1;
			}
			ori.Secondary[i] = new_second;//���¶�������
		}
	}
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