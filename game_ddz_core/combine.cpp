#include "combine.h"
#include <algorithm>

#define MARK(data) ((data) & 0x0f)
#define COLOR(data) ((data) >> 4)
#define MAKE_PARAM(lo,hi) (((hi) << 16) | ((lo) & 0xFFFF))

static bool _Check_sequence_n(const char* data, size_t len, int n)
{
	if (len < 5 || len % n != 0)
		return false;
	for (int i = 1; i < n; i++) {
		if (MARK(data[i]) != MARK(data[i - 1]))
			return false;
	}

	for (size_t i = n; i < len; i++) {
		if (MARK(data[i]) > 0x0B)
			return false;
		if (MARK(data[i]) != MARK(data[i - n]) + 1)
			return false;
	}
	return true;
}

static bool _Check_craft(const char* data, size_t len, combine_info& cb) {
	
	if (len < 4)
		return false;

	if (len % 4 != 0 && len % 5 != 0)
		return false;

	char px[16] = { 0 };
	size_t n_3 = 0;

	for (size_t i = 0; i < len; i++) {
		px[MARK(data[i])]++;
	}

	int first_3 = -1;
	for (int i = 0; i < 15; i++) {
		if (px[i] == 3) {
			first_3 = i;
			break;
		}
	}
	if (len > 5 && px[0x0C] == 3) {
		return false;
	}

	if (first_3 == -1)
		return false;
	for (int i = first_3; i < 15 && px[i] == 3; i++) {
		n_3++;
	}

	if (len % 4 == 0) {
		if (n_3 * 4 == len) {
			cb.combine = combine_type::combine_33_1;
			cb.combine_parameter = MAKE_PARAM(first_3,len);
			return true;
		}
	}
	if (len % 5 == 0) {
		int n_2 = 0;
		for (int i = 0; i < 15; i++) {
			if (px[i] == 2)
				n_2++;
			if (px[i] == 4)
				n_2 += 2;
		}
		if (n_3 != n_2 || n_3 * 5 != len) {
			return false;
		}
		cb.combine = combine_type::combine_33_2;
		cb.combine_parameter = MAKE_PARAM(first_3,len);
		return true;
	}
	return false;
}

static bool _Check_4_1(const char* data) {
	for (int i = 3; i < 6; i++)
		if (MARK(data[i]) == MARK(data[i - 3]))
			return true;
	return false;
}

static bool _Check_4_2(const char* data,uint32_t& parameter) {
	int _Off_1[] = { 0,2,4 };
	int _Off_2[] = { 4,0,0 };
	int _Off_3[] = { 6,6,2 };
	for (int i = 0; i < 3; i++) {
		if (MARK(data[_Off_1[i]]) == MARK(data[_Off_1[i] + 3]) &&
			MARK(data[_Off_2[i]]) == MARK(data[_Off_2[i] + 1]) &&
			MARK(data[_Off_3[i]]) == MARK(data[_Off_3[i] + 1]) &&
			MARK(data[_Off_2[i]]) != MARK(data[_Off_3[i]])) {
			parameter = data[_Off_1[i]];
			return true;
		}
	}
	return false;
}

combine_info get_combine_info(const char* data_X, size_t len)
{
	char data[100];
	memcpy(data, data_X, len);
	std::sort(data, data + len, [](char v1, char v2) {
		if (MARK(v1) == MARK(v2))
			return COLOR(v1) < COLOR(v2);
		return MARK(v1) < MARK(v2);
		});

	combine_info cb;
	cb.combine = combine_type::combine_error;
	cb.combine_parameter = 0;


	if (len == 0) {
		cb.combine = combine_type::combine_pass;
		cb.combine_parameter = 0;
		return cb;
	}
	if (len == 1) {

		cb.combine = combine_type::combine_1;
		cb.combine_parameter = MARK(data[0]);
		return cb;
	}
	if (len == 2)
	{
		if (MARK(data[0]) == MARK(data[1])) {
			cb.combine = combine_type::combine_2;
			cb.combine_parameter = MARK(data[0]);
		}
		else if (data[0] == 0x0D && data[1] == 0x0E) {
			cb.combine = combine_type::combine_j2;
		}
		return cb;
	}
	if (len == 3)
	{
		if (MARK(data[0]) == MARK(data[1]) && MARK(data[0]) == MARK(data[2])) {
			char buffer[24];
			sprintf(buffer, "w_tuple%d.mp3", (MARK(data[0])) + 3);
			cb.combine = combine_type::combine_3;
			cb.combine_parameter = MARK(data[0]);
		}
		return cb;
	}
	if (_Check_sequence_n(data, len, 1)) {
		cb.combine = combine_type::combine_seq_1;
		cb.combine_parameter = MAKE_PARAM(MARK(data[0]), len);
		return cb;
	}
	if (_Check_sequence_n(data, len, 2)) {
		cb.combine = combine_type::combine_seq_2;
		cb.combine_parameter = MAKE_PARAM(MARK(data[0]), len);
		return cb;
	}
	if (_Check_sequence_n(data, len, 3)) {
		cb.combine = combine_type::combine_seq_3;
		cb.combine_parameter = MAKE_PARAM(MARK(data[0]), len);
		return cb;
	}
	if (_Check_craft(data, len, cb))
	{
		return cb;
	}
	if (len == 4) {
		//bomb
		if (MARK(data[3]) == MARK(data[0]))
		{
			cb.combine = combine_type::combine_4;
			cb.combine_parameter = MARK(data[0]);
		}
		return cb;
	}
	if (len == 6) {
		if(_Check_4_1(data)){
			cb.combine = combine_type::combine_4_1;
			cb.combine_parameter = MARK(data[3]);
		}
		return cb;
	}
	if (len == 8) {
		if(_Check_4_2(data,cb.combine_parameter)){
			cb.combine = combine_type::combine_4_2;
			return cb;
		}
	}
	return cb;
}

_Combine_effect_info get_combine_effect(const combine_info& c, bool type_g)
{
	_Combine_effect_info E;
	char buffer[1024];
	switch (c.combine)
	{
	case combine_type::combine_pass:
		E.effect_sound_file = type_g?"nv_buyao.mp3":"nan_pass.mp3";
		break;
	case combine_type::combine_1:
		sprintf(buffer, type_g ? "w_1_%d.mp3" : "m_1_%d.mp3", c.combine_parameter + 3);
		E.effect_sound_file = buffer;
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_2:
		sprintf(buffer, type_g ? "w_2_%d.mp3" : "m_2_%d.mp3", c.combine_parameter + 3);
		E.effect_sound_file = buffer;
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_3:
		sprintf(buffer, type_g ? "w_tuple%d.mp3" : "m_tuple%d.mp3", c.combine_parameter + 3);
		E.effect_sound_file = buffer;
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_4:
		E.effect_sound_file = type_g ? "nv_bomb.mp3" : "nan_bomb.mp3";
		E.target_sound_file = "boombeffect.mp3";
		break;
	case combine_type::combine_4_1:
		E.effect_sound_file = type_g ? "nv_4dai2.mp3" : "nan_4dai2.mp3";
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_4_2:
		E.effect_sound_file = type_g ? "nv_4dai22.mp3" : "nan_4dai22.mp3";
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_seq_1:
		E.effect_sound_file = type_g ? "nv_shunzi.mp3" : "nan_shunzi.mp3";
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_seq_2:
		E.effect_sound_file = type_g ? "nv_liandui.mp3" : "nan_liandui.mp3";
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_seq_3:
		E.effect_sound_file = type_g ? "nv_3dai0.mp3" : "nan_3dai0.mp3";
		E.target_sound_file = "outcard.mp3";
		break;
	case combine_type::combine_33_1:
		if ((c.combine_parameter >> 16) == 4) {
			E.effect_sound_file = type_g ? "nv_3dai1.mp3" : "nan_3dai1.mp3";
			E.target_sound_file = "chupai.mp3";
		}
		else {
			E.effect_sound_file = type_g ? "nv_feiji.mp3" : "nan_feiji.mp3";
			E.target_sound_file = "planeeffect.mp3";
		}
		break;
	case combine_type::combine_33_2:
		if ((c.combine_parameter >> 16) == 5) {
			E.effect_sound_file = type_g ? "nv_3dai2.mp3" : "nan_3dai2.mp3";
			E.target_sound_file = "chupai.mp3";
		}
		else {
			E.effect_sound_file = type_g ? "nv_feiji.mp3" : "nan_feiji.mp3";
			E.target_sound_file = "planeeffect.mp3";
		}
		break;
	case combine_type::combine_j2:
		E.effect_sound_file = type_g ? "nv_wangzha.mp3" : "nan_wangzha.mp3";
		E.target_sound_file = "chupai.mp3";
		break;
	case combine_type::combine_error:
	default:
		break;
	}
	return E;
}

bool is_combine_available(const combine_info& this_info, const combine_info* _Previous)
{
	if (this_info.combine == combine_type::combine_pass)
		return _Previous->combine != combine_type::combine_pass;

	if (_Previous->combine == combine_type::combine_pass)
		return true;
	if (_Previous->combine == combine_type::combine_j2)
		return false;
	if (this_info.combine == combine_type::combine_j2)
		return true;

	if (this_info.combine != _Previous->combine) {
		if (this_info.combine != combine_type::combine_4)
			return false;
		return true;
	}
	return ((this_info.combine_parameter & 0xFFFF0000) == (_Previous->combine_parameter & 0xFFFF0000)) &&
		((this_info.combine_parameter & 0xFFFF) > (_Previous->combine_parameter & 0xFFFF));
}

bool is_combine_available(const combine_info& previous, const char* data_X, size_t length)
{
	char data[100];
	for (size_t i = 0; i < length; i++) {
		data[i] = MARK(data_X[i]);
	}
	std::sort(data, data + length);
	if (previous.combine == combine_type::combine_j2)
		return false;
	if (length == 2 && data[0] == 0x0D && data[1] == 0x0E)
	{
		return true;
	}
	if (length == 4 && data[0] == data[3] && previous.combine != combine_type::combine_4)
		return true;

	combine_info C;
	switch (previous.combine)
	{
	case combine_type::combine_pass:
		return length > 0 && get_combine_info(data_X,length).combine != combine_type::combine_error;
	case combine_type::combine_4:
		return length == 4 && data[0] == data[3] && data[0] > previous.combine_parameter;
	case combine_type::combine_1:
		return length == 1 && data[0] > previous.combine_parameter;
	case combine_type::combine_2:
		return length == 2 && data[0] == data[1] && data[0] > previous.combine_parameter;
	case combine_type::combine_3:
		return length == 3 && data[0] == data[2] && data[0] > previous.combine_parameter;
	case combine_type::combine_4_1:
		return length == 6 && _Check_4_1(data) && data[3] > previous.combine_parameter;
	case combine_type::combine_4_2:
		return length == 8 && _Check_4_2(data,C.combine_parameter) && C.combine_parameter > previous.combine_parameter;
	case combine_type::combine_seq_1:
		return length == (previous.combine_parameter >> 16) && data[0] > (previous.combine_parameter & 0xFFFF) &&
			_Check_sequence_n(data, length, 1);
	case combine_type::combine_seq_2:
		return length == (previous.combine_parameter >> 16) && data[0] > (previous.combine_parameter & 0xFFFF) &&
			_Check_sequence_n(data, length, 2);
	case combine_type::combine_seq_3:
		return length == (previous.combine_parameter >> 16) && data[0] > (previous.combine_parameter & 0xFFFF) &&
			_Check_sequence_n(data, length, 3);
	case combine_type::combine_33_1:
	case combine_type::combine_33_2:
		return length == (previous.combine_parameter >> 16) && _Check_craft(data, length, C) &&
			C.combine == previous.combine && (C.combine_parameter & 0xFFFF) > (previous.combine_parameter & 0xFFFF);
	case combine_type::combine_j2:
	default:
		return false;
	}
}

void combine_find_init(combine_find_state* _S, const combine_info& _Previous, const char* data, size_t length)
{
	_S->_Previous = _Previous;
	_S->_f_param = _Previous;
	_S->_S_break = 0;

	memset(_S->px, 0, 15);

	for (size_t i = 0; i < length; i++) {
		_S->px[MARK(data[i])]++;
	}
}

//8x1 seq_1
static int _Find_combine_nn(combine_find_state* _S, int n1, int n2) {
	int n_Cx = 0;
	int max_check_px = n1 == 1 ? 0x0D : 0x0C;
	for (int i = (_S->_Previous.combine_parameter & 0xFFFF) + 1; i < max_check_px; i++) {
		if (_S->px[i] >= n2 && _S->px[i] != 4) {
			n_Cx++;
		}
		else {
			n_Cx = 0;
		}
		if (n_Cx == n1) {
			return i - n1 + 1;
		}
	}
	return -1;
}

int _Find_n(combine_find_state* _S,int n) {

	for (_S->_S_break = std::max(_S->_S_break,n); _S->_S_break < 4; _S->_S_break++) {
		for (int i = (_S->_Previous.combine_parameter & 0x0ffff) + 1; i < 15; i++) {
			if (_S->px[i] == _S->_S_break)
				return i;
		}
		_S->_Previous = _S->_f_param;
	}
	return -1;
}

static bool _Find_n_no_limit_except(combine_find_state* _S, int n, int count,char* px) 
{
	int n_finded = 0;
	for (int breaking = n; breaking < 4; breaking++) {
		for (int i = 0; i < 15; i++) {
			if (_S->px[i] == breaking && px[i] == 0) {
				int aval_count = std::min(_S->px[i] / n, count - n_finded);
				px[i] = aval_count * n;
				n_finded += aval_count;
				if (n_finded == count)
					return true;
			}
		}
	}
	return false;
}

bool combine_find_next_nbomb(combine_find_state* _S, combine_find_result* _Res)
{
	int _fRes;
	bool fType = true;
	int f_P1 = 0, f_P2 = 0,f_P3 = 0;

	switch (_S->_Previous.combine)
	{
	case combine_type::combine_pass:
		return false;
	case combine_type::combine_1:
		f_P1 = 1; f_P2 = 1;
		break;
	case combine_type::combine_2:
		f_P1 = 1; f_P2 = 2;
		break;
	case combine_type::combine_3:
		f_P1 = 1; f_P2 = 3;
		break;
	case combine_type::combine_4:
		return false;
	case combine_type::combine_4_1:
		fType = false;
		f_P1 = 1;f_P2 = 4;f_P3 = 1;
		break;
	case combine_type::combine_4_2:
		fType = false;
		f_P1 = 1; f_P2 = 4; f_P3 = 2;
		break;
	case combine_type::combine_seq_1:
		f_P1 = _S->_Previous.combine_parameter >> 16; f_P2 = 1; f_P3 = 0;
		break;
	case combine_type::combine_seq_2:
		f_P1 = _S->_Previous.combine_parameter >> 17; f_P2 = 2; f_P3 = 0;
		break;
	case combine_type::combine_seq_3:
		f_P1 = (_S->_Previous.combine_parameter >> 16) / 3; f_P2 = 3; f_P3 = 0;
		break;
	case combine_type::combine_33_1:
		fType = false;
		f_P1 = (_S->_Previous.combine_parameter >> 18); f_P2 = 3; f_P3 = 1;
		break;
	case combine_type::combine_33_2:
		fType = false;
		f_P1 = (_S->_Previous.combine_parameter >> 16) / 5; f_P2 = 3; f_P3 = 2;
		break;
	case combine_type::combine_j2:
		return false;
	case combine_type::combine_error:
		return false;
	}

	int  bRes = -1;
	if (fType) {
		if (f_P1 == 1) {
			bRes = _Find_n(_S, f_P2);
		}
		else {
			bRes = _Find_combine_nn(_S, f_P1, f_P2);
		}
		if (bRes > 0) {
			for (int i = 0; i < f_P1; i++) {
				_Res->result[i + bRes] = f_P2;
			}
			_S->_Previous.combine_parameter = f_P1 == 1 ? bRes : MAKE_PARAM(bRes, f_P1 * f_P2);
		}
		return bRes > 0;
	}
	else
	{
		int count = f_P2 == 3 ? f_P1 : 2;
		bRes = _Find_combine_nn(_S, f_P1, f_P2);
		if (bRes > 0) {
			for (int i = 0; i < f_P1; i++) {
				_Res->result[i + bRes] = f_P2;
			}
			if (_Find_n_no_limit_except(_S, f_P3, count, _Res->result))
			{
				_S->_Previous.combine_parameter = MAKE_PARAM(bRes, f_P1 * f_P2 + f_P3 * count);
				return true;
			}
		}
	}
	return false;
}

bool combine_find_next(combine_find_state* state, combine_find_result* res)
{
	memset(res->result, 0, 15);
	if (combine_find_next_nbomb(state, res))
		return true;
	if (state->_f_param.combine != combine_type::combine_j2) {
		for (int i = state->_Previous.combine == combine_type::combine_4? 
			state->_Previous.combine_parameter + 1 : 0; i < 13; i++) {
			if (state->px[i] == 4)
			{
				res->result[i] = 4;
				state->_Previous.combine = combine_type::combine_4;
				state->_Previous.combine_parameter = i;
				return true;
			}
		}
		if (state->px[0x0D] == 1 && state->px[0x0E] == 1) {
			res->result[0x0D] = res->result[0x0E] = 1;
			state->_Previous.combine = combine_type::combine_j2;
			state->_Previous.combine_parameter = 0;
			return true;
		}
	}
	return false;
}

void combine_find_reset(combine_find_state* state)
{
	state->_Previous = state->_f_param;
	state->_S_break = 0;
}
