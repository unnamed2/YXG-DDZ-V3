#pragma once
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <string>
#include <vector>

struct blob
{
	size_t blob_size;
	char blob_data[0];
};

struct file_item
{
	std::string path;
	std::string etag;
};

struct version_request
{
	size_t app_version;
	std::vector<file_item> items;
};

enum class room_state
{
	waiting, calling, gaming
};

enum room_player_state
{
	no_player,
	loss_connection,
	ready,
	waiting,
};

struct room_info
{
	room_state state;
	std::string player_name[3];
	int player_score[3];
	room_player_state player_state[3];
};

struct player_joined
{
	std::string player_id;
	int player_score;
	size_t player_position;
};

enum class call_score
{
	not_called,
	_0, _1, _2,
};

struct call_state
{
	call_score call_score[3];
	size_t first_caller;
};

enum class action_state
{
	passed,
	not_acted,
	acted
};

struct game_state
{
	action_state state[3];
	std::string actions[3];
	size_t current_actor;
	std::string hand_card;
	size_t d_player;
	std::string dd_card;
	size_t current_score_board;
};

struct game_finish_action
{
	bool is_spring;
	int this_game_score[3];
	int score_after[3];
	size_t winner;
};

struct game_start
{
	std::string initial_cards;
	size_t first_caller;
};

struct player_do_ready
{
	std::string token;
};

struct player_do_call
{
	std::string token;
	call_score score;
};

struct player_do_action
{
	std::string token;
	std::string card_action;
};

struct player_do_join
{
	std::string player_join_string;
	std::string room_name;
};

struct player_do_reset
{
	std::string token;
};

struct player_do_get_game_details {};

#endif