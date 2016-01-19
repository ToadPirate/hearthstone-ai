#ifndef _STAGES_STAGE_H
#define _STAGES_STAGE_H

enum StageType {
	STAGE_TYPE_FLAG = 0x000F,
	STAGE_TYPE_PLAYER = 0x0001,
	STAGE_TYPE_OPPONENT = 0x0002,
	STAGE_TYPE_GAME_FLOW = 0x0003 // game flow including randoms
};

enum Stage {
	STAGE_UNKNOWN = 0,

	STAGE_PLAYER_TURN_START           = 0x0010 | STAGE_TYPE_GAME_FLOW,
	STAGE_PLAYER_CHOOSE_BOARD_MOVE    = 0x0020 | STAGE_TYPE_PLAYER,
	STAGE_PLAYER_TURN_END             = 0x0030 | STAGE_TYPE_GAME_FLOW,
	STAGE_OPPONENT_TURN_START         = 0x0040 | STAGE_TYPE_GAME_FLOW,
	STAGE_OPPONENT_CHOOSE_BOARD_MOVE  = 0x0050 | STAGE_TYPE_OPPONENT,
	STAGE_OPPONENT_TURN_END           = 0x0060 | STAGE_TYPE_GAME_FLOW,

	STAGE_WIN  = 0x0070 | STAGE_TYPE_GAME_FLOW,
	STAGE_LOSS = 0x0080 | STAGE_TYPE_GAME_FLOW
};
#endif
