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

/*倒排列表集合*/
vector<vector<unsigned int>> index_lists;

/*求交结果集合*/
vector<vector<unsigned int>> res_lists;

/*第一列保存keywords数量*/
int query[QUERY_NUM][MAX_KEY_NUM + 1];

/*结果文件写入*/
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
	/*列表的位向量转化*/
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
			/*index置1*/
			int temp = index_lists[query_no][i];
			int pos = temp / 32;//数组下标
			int bit = temp % 32;//位
			unsigned int temp_bit = 1;
			temp_bit = temp_bit << bit;
			index[pos] = index[pos] | temp_bit;//按位或运算添加位
			/*Secondary置1*/
			int pos_s = pos / 32;//数组下标
			int bit_s = pos % 32;//位
			unsigned int temp_bit_s = 1;
			temp_bit_s = temp_bit_s << bit_s;
			Secondary[pos_s] = Secondary[pos_s] | temp_bit_s;//按位或运算添加位
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
			index_file.read((char*)&next_int, sizeof(next_int));//索引数组内容
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

	timespec_get(&sts, TIME_UTC);
	vector_list_intersection();

	timespec_get(&ets, TIME_UTC);
	time_sum = (ets.tv_nsec - sts.tv_nsec) * 0.000001 + (ets.tv_sec - sts.tv_sec) * 1000;
	cout << "位向量用时： " << time_sum << "毫秒" << endl;

	res_writing(OUTPUT_FILE_1);

	return 0;
}

void vector_list_intersection() {
	for (int n = 0; n < QUERY_NUM; n++) {
		/*转存为位向量*/
		vector<vector_list> vectors(query[n][0]);
		for (int i = 1; i <= query[n][0]; i++) {
			vectors[i - 1] = vector_list(query[n][i]);
		}

		/*0号位向量与其余所有向量按位与*/
		for (int i = 1; i < query[n][0]; i++) {
			vector_and(vectors[0], vectors[i]);
		}

		/*获取结果*/
		vector<unsigned int> S;
		for (int i = 0; i < 24615; i++) {
			if (vectors[0].Secondary[i] != 0) {
				unsigned int temp = vectors[0].Secondary[i];
				for (int j = 0; j < 32; j++) {
					/*找到不为0的位*/
					bool exist = judge(temp, j);
					if (exist) {
						//找到对应的32位
						unsigned int temp_index = vectors[0].index[i * 32 + j];
						for (int k = 0; k < 32; k++) {
							/*找到不为0的位*/
							bool exist_index = judge(temp_index, k);
							if (exist_index) {
								//转化为对应的值
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
		/*二级索引按位与*/
		unsigned int temp = ori.Secondary[i] & tem.Secondary[i];
		if (temp == 0) {
			ori.Secondary[i] = 0;//更新二级索引
			continue;
		}
		else {
			/*对应位需要按位与*/
			unsigned int new_second = 0;//二级索引将被更新的值
			unsigned int new_second_temp = 1;
			for (int j = 0; j < 32; j++) {
				bool exist = judge(temp, j);
				if (exist) {
					ori.index[i * 32 + j] = tem.index[i * 32 + j] & ori.index[i * 32 + j];//二级索引位对应的int按位与
					if (ori.index[i * 32 + j] != 0) new_second = new_second | new_second_temp;//按位或运算添加位
				}
				new_second_temp = new_second_temp << 1;
			}
			ori.Secondary[i] = new_second;//更新二级索引
		}
	}
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