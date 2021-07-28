#pragma once
#ifndef _combine_h__
#define _combine_h__

#include <string>

enum class combine_type
{
	combine_pass = 0,
	combine_1 = 1,		//����-o
	combine_2,			//����-o
	combine_3,				//3����-o
	combine_4,				//ը��
	combine_4_1,			//�ĸ���2�ŵ���
	combine_4_2,		//�ĸ���2��
	combine_seq_1,			//����-o
	combine_seq_2,			//����-o
	combine_seq_3,			//�ɻ�����-o
	combine_33_1,	//�ɻ�������-
	combine_33_2,	//�ɻ�������-
	combine_j2,//o
	combine_error,	//���������
};

struct combine_info
{
	combine_type combine = combine_type::combine_error;
	uint32_t combine_parameter = 0;
};

struct _Combine_effect_info
{
	std::string target_sound_file, effect_sound_file;
};

combine_info get_combine_info(const char* data, size_t len);

_Combine_effect_info get_combine_effect(const combine_info& c, bool type_g);

bool is_combine_available(const combine_info& this_info, const combine_info* _Previous);

bool is_combine_available(const combine_info& previous, const char* data, size_t length);

struct combine_find_state {
	combine_info _Previous;
	combine_info _f_param;
	int  _S_break = 0;
	unsigned char px[15];
};

struct combine_find_result {
	char result[15];
};

void combine_find_init(combine_find_state* state, const combine_info& previous, const char* data, size_t length);

bool combine_find_next(combine_find_state* state, combine_find_result* res);

void combine_find_reset(combine_find_state* state);

#endif // combine_h__