# -*- coding: utf-8 -*-

"""
"""

# ------------------------------------------------------------------------------
# entity state
# ------------------------------------------------------------------------------
ENTITY_STATE_UNKNOW										= -1
ENTITY_STATE_FREE										= 0
ENTITY_STATE_DEAD										= 1
ENTITY_STATE_REST										= 2
ENTITY_STATE_FIGHT										= 3
ENTITY_STATE_MAX    									= 4

# sub state
ENTITY_SUB_STATE_NORMAL									= 0
ENTITY_SUB_STATE_RANDOM_STROLL							= 1
ENTITY_SUB_STATE_GO_BACK								= 2
ENTITY_SUB_STATE_CHASE_TARGET							= 3
ENTITY_SUB_STATE_FLEE									= 4

# entity的一些行为禁止标志
FORBID_NO												= 0x00000000
FORBID_MOTION											= 0x00000001
FORBID_CHAT												= 0x00000002
FORBID_SPELL											= 0x00000004
FORBID_TRADE											= 0x00000008
FORBID_EQUIP											= 0x00000010
FORBID_INTONATE											= 0x00000020
FORBID_ATTACK_PHY_NEAR									= 0x00000040
FORBID_ATTACK_PHY_FAR									= 0x00000080
FORBID_ATTACK_MAGIC										= 0x00000080
FORBID_YAW												= 0x00008000

FORBID_ATTACK_PHY = FORBID_ATTACK_PHY_NEAR | FORBID_ATTACK_PHY_FAR
FORBID_ATTACK_MAG = FORBID_ATTACK_MAGIC
FORBID_ATTACK = FORBID_ATTACK_PHY | FORBID_ATTACK_MAG
FORBID_MOTION_YAW = FORBID_MOTION | FORBID_YAW

FORBID_ALL = [
	FORBID_MOTION,
	FORBID_YAW,
	FORBID_CHAT,
	FORBID_ATTACK,
	FORBID_SPELL,
	FORBID_TRADE,
	FORBID_EQUIP,
	FORBID_INTONATE,
	FORBID_ATTACK_PHY_NEAR,
	FORBID_ATTACK_PHY_FAR,
	FORBID_ATTACK_MAGIC,
]

FORBID_ACTIONS = {
	ENTITY_STATE_UNKNOW  : 0,
	ENTITY_STATE_FREE    : FORBID_NO,
	ENTITY_STATE_DEAD    : FORBID_MOTION_YAW | FORBID_TRADE | FORBID_ATTACK | FORBID_SPELL | FORBID_EQUIP,
	ENTITY_STATE_REST    : FORBID_MOTION_YAW | FORBID_TRADE | FORBID_ATTACK | FORBID_SPELL | FORBID_EQUIP,
	ENTITY_STATE_FIGHT   : FORBID_EQUIP | FORBID_TRADE,
	}

for f in FORBID_ALL: FORBID_ACTIONS[ENTITY_STATE_UNKNOW] |= f

# ------------------------------------------------------------------------------
# 定义对话相关
# ------------------------------------------------------------------------------
DIALOG_TYPE_NORMAL			= 0 # 普通对话
DIALOG_TYPE_QUEST			= 1 # 任务对话

# ------------------------------------------------------------------------------
# 技能相关
# ------------------------------------------------------------------------------
# 技能对象类别
SKILL_OBJECT_TYPE_UNKNOWN	= 0
SKILL_OBJECT_TYPE_ENTITY	= 1
SKILL_OBJECT_TYPE_POSITION	= 2
