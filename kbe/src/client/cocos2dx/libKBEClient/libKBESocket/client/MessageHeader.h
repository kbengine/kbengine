//9miao.com 首发
#pragma once

#define msg_player_role_list_s2c 5


// 查询分线服务器
#define msg_role_line_query_c2s 6

// 成功查询分线服务器
#define msg_role_line_query_ok_s2c 7

// 换线
#define msg_role_change_line_c2s 9

// 选定某个角色
#define msg_player_select_role_c2s 10

// 客户端加载地图数据完成
#define msg_map_complete_c2s 13

// 玩家初始化数据
#define msg_role_map_change_s2c 14

// NPC的初始信息
#define msg_npc_init_s2c 15

// 已经存在于地图的角色数据
#define msg_other_role_map_init_s2c   16

// 其他角色进入地图
//#define msg_other_role_into_map_s2c  20

// 其他角色离开地图
//#define msg_other_role_leave_map_s2c  21

// 其他角色更换地图
#define msg_role_change_map_c2s  22

// 角色更换地图成功
#define msg_role_change_map_ok_s2c  23

// 角色更换地图失败
#define msg_role_change_map_fail_s2c  24

// 角色移动
#define msg_role_move_c2s  25

// 移动心跳包
#define msg_heartbeat_c2s  26

// 他人移动广播消息
#define msg_other_role_move_s2c  27

// 移动确认失败
#define msg_role_move_fail_s2c  28

// 角色发起攻击
#define msg_role_attack_c2s  29

// 角色发起攻击回复
#define msg_role_attack_s2c  31

// 攻击取消
#define msg_role_cancel_attack_s2c  32

// 攻击伤害回复
#define msg_be_attacked_s2c  33

// 玩家被KILL掉
#define msg_be_killed_s2c  34

// 某玩家进入了视野
#define msg_other_role_into_view_s2c   35

// 说明：把NPC和Role的进入视野消息分开：客户端使用不同类创建他们；
//NPC和Role离开视野使用同一个消息：客户端只需要根据ID便可以删除他们；
// NPC进入视野
#define msg_npc_into_view_s2c  36

// 某玩家离开了视野
#define msg_creature_outof_view_s2c  37

// 调试消息
#define msg_debug_c2s  38

//物品的使用
#define msg_use_item_c2s 39

//装备物品
#define msg_auto_equip_item_c2s  40

//物品转换位置失败：装备物品或其他
#define msg_change_item_failed_s2c 41

#define msg_use_item_error_s2c 42

#define msg_buff_immune_s2c 43

// 玩家的属性信息
#define msg_role_attribute_s2c  53

// Npc的属性信息
#define msg_npc_attribute_s2c  54

#define msg_role_rename_c2s 55

#define msg_guild_rename_c2s 56

#define msg_rename_result_s2c 57

////////////////////////////////////////////////////////////////////////////////
// 场景切换消息
////////////////////////////////////////////////////////////////////////////////
#define msg_role_map_change_c2s   61
#define msg_npc_map_change_c2s  62
#define msg_map_change_failed_s2c 63

////////////////////////////////////////////////////////////////////////////////
// 快捷栏和技能面板消息
////////////////////////////////////////////////////////////////////////////////

// 客户端请求获取技能面板中已经学习了那些技能
#define msg_skill_panel_c2s  70

// 服务器通知客户端学会了那些技能
#define msg_learned_skill_s2c  71

// 返回给客户端快捷栏里的内容
#define msg_display_hotbar_s2c  72

// 客户端对快捷栏进行操作
#define msg_update_hotbar_c2s  73

// 客户端对快捷栏操作返回失败
#define msg_update_hotbar_fail_s2c  74

#define msg_update_skill_s2c	  75

////////////////////////////////////////////////////////////////////////////////
//
//任务相关
//
////////////////////////////////////////////////////////////////////////////////

//服务器返回角色所有任务的列表 
#define msg_quest_list_update_s2c 		 81

//服务器返回角色任务被从列表中删除
#define msg_quest_list_remove_s2c	 82

//服务器返回,添加新任务
#define msg_quest_list_add_s2c 	   83

//服务器返回角色，任务状态改变
#define msg_quest_statu_update_s2c 	  84

//请求任务列表
#define msg_questgiver_hello_c2s			  85

//返回任务列表
#define msg_questgiver_quest_details_s2c			  86

//接任务请求
#define msg_questgiver_accept_quest_c2s			 87

//放弃任务请求
#define msg_quest_quit_c2s			 88

//交任务请求
#define msg_questgiver_complete_quest_c2s 89

////交任务成功
#define msg_quest_complete_s2c 90

//交任务失败
#define msg_quest_complete_failed_s2c 91


//请求刷新npc任务状态
#define msg_questgiver_states_update_c2s 92

//npc任务状态
#define msg_questgiver_states_update_s2c 93

//查看任务详情
#define msg_quest_details_c2s 94

//任务详情
#define msg_quest_details_s2c 95

//查看可接任务
#define msg_quest_get_adapt_c2s 96

#define msg_quest_get_adapt_s2c 97

//接任务失败
#define msg_quest_accept_failed_s2c 98

#define msg_quest_direct_complete_c2s 99
////////////////////////////////////////////////////////////////////////////////
// BUFF/DEBUFF相关
////////////////////////////////////////////////////////////////////////////////

// 某些单元中了某个BUFF
#define msg_add_buff_s2c 101

// 删除某单元已存在的BUFF
#define msg_del_buff_s2c 102

// BUFF导致某属性信息的改变
#define msg_buff_affect_attr_s2c 103

//停止Move
#define msg_move_stop_s2c 104


////////////////////////////////////////////////////////////////////////////////
// //掉落相关
////////////////////////////////////////////////////////////////////////////////
//掉落
#define msg_loot_s2c 105

//查询掉落
#define msg_loot_query_c2s 106

//掉落返回
#define msg_loot_response_s2c  107

//捡起
#define msg_loot_pick_c2s   108

//删除掉落物品
#define msg_loot_remove_item_s2c 109

//删除掉落包裹
#define msg_loot_release_s2c 110

//角色升级属性改变
#define msg_player_level_up_s2c  111

//用户主动取消buff
#define msg_cancel_buff_c2s 112

////////////////////////////////////////////////////////////////////////////////
// 物品等操作相关
////////////////////////////////////////////////////////////////////////////////

//更新已有物品位置或属性
#define msg_update_item_s2c 120

//创建新物品
#define msg_add_item_s2c 121

//创建物品失败
#define msg_add_item_failed_s2c 122

//客户端请求删除物品
#define msg_destroy_item_c2s  123

//删除物品
#define msg_delete_item_s2c 124

//分割物品,涉及创建新物品
#define msg_split_item_c2s 125

//交换物品位置
#define msg_swap_item_c2s   126

//玩和背家身上包里的物品
#define msg_init_onhands_item_s2c   127

//获取仓库物品
#define msg_npc_storage_items_c2s 128

#define msg_npc_storage_items_s2c 129

//整理
#define msg_arrange_items_c2s 130
//整理结果
#define msg_arrange_items_s2c 131

////////////////////////////////////////////////////////////////////////////////
// 聊天消息
////////////////////////////////////////////////////////////////////////////////
#define msg_chat_c2s					 140
#define msg_chat_s2c					 141
#define msg_chat_failed_s2c					 142
#define msg_loudspeaker_queue_num_c2s 143
#define msg_loudspeaker_queue_num_s2c 144
#define msg_loudspeaker_opt_s2c 145
#define msg_chat_private_c2s 146
#define msg_chat_private_s2c 147  //新私聊

////////////////////////////////////////////////////////////////////////////////
// 组队相关 C -> S
////////////////////////////////////////////////////////////////////////////////

//申请入队
#define msg_group_apply_c2s 150

//同意入队
#define msg_group_agree_c2s 151

//创建
#define msg_group_create_c2s 152

//邀请
#define msg_group_invite_c2s 153

//接受邀请
#define msg_group_accept_c2s 154

//拒绝邀请
#define msg_group_decline_c2s 155

//踢出队友
#define msg_group_kickout_c2s 156

//设置队长
#define msg_group_setleader_c2s 157

//解散队伍
#define msg_group_disband_c2s 158

//离开队伍
#define msg_group_depart_c2s 159

////////////////////////////////////////////////////////////////////////////////
// 组队相关 S -> C
////////////////////////////////////////////////////////////////////////////////
#define msg_group_invite_s2c 160

#define msg_group_decline_s2c 161

#define msg_group_destroy_s2c 162

//更新队伍信息
#define msg_group_list_update_s2c 163

//操作返回结果
#define msg_group_cmd_result_s2c 164

//更新队友状态
#define msg_group_member_stats_s2c 165

#define msg_group_apply_s2c 166

////////////////////////////////////////////////////////////////////////////////
// 招募
////////////////////////////////////////////////////////////////////////////////
//发布队伍招募
#define msg_recruite_c2s 167
//取消队伍招募
#define msg_recruite_cancel_c2s 168
//队伍招募查询
#define msg_recruite_query_c2s 169
//队伍招募查询返回
#define msg_recruite_query_s2c 170
//取消队伍招募
#define msg_recruite_cancel_s2c 171
//发布个人求组
#define msg_role_recruite_c2s 172
//取消个人求组
#define msg_role_recruite_cancel_c2s 173
//个人求组被取消的原因
#define msg_role_recruite_cancel_s2c 174


//aoi玩家信息
#define msg_aoi_role_group_c2s 175

//返回aoi内玩家信息
#define msg_aoi_role_group_s2c 176
////////////////////////////////////////////////////////////////////////////////
// NPC 消息整理
////////////////////////////////////////////////////////////////////////////////

// NPC 功能公用错误消息
#define msg_npc_fucnction_common_error_s2c	  300
////////////////////////////////////////////////////////////////////////////////
// npc功能请求
////////////////////////////////////////////////////////////////////////////////
#define msg_npc_function_c2s 301
#define msg_npc_function_s2c 302

////////////////////////////////////////////////////////////////////////////////
// 买卖消息
////////////////////////////////////////////////////////////////////////////////
// 枚举商店物品请求
#define msg_enum_shoping_item_c2s  310
// 枚举Fail
#define msg_enum_shoping_item_fail_s2c   311
// 回复商店物品
#define msg_enum_shoping_item_s2c  312

// 购买请求
#define msg_buy_item_c2s 313
// 购买失败
#define msg_buy_item_fail_s2c  314
// 出售请求
#define msg_sell_item_c2s  315
// 出售成功
#define msg_sell_item_fail_s2c   316
//修理
#define msg_repair_item_c2s								  317

////////////////////////////////////////////////////////////////////////////////
// NPC 消息整理 *结束*
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// 疲劳系统消息
////////////////////////////////////////////////////////////////////////////////
//带类型的警告提示
#define msg_fatigue_prompt_with_type_s2c 340

//时间未满 不能登录
#define msg_fatigue_login_disabled_s2c 341

//警告提示
#define msg_fatigue_prompt_s2c	 350
//弹出消息
#define msg_fatigue_alert_s2c	 351
//完成注册url
#define msg_finish_register_s2c	 352
////////////////////////////////////////////////////////////////////////////////
// 疲劳系统消息  *结束*
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
//优化的信息更新
//
////////////////////////////////////////////////////////////////////////////////
#define msg_object_update_s2c 353

////////////////////////////////////////////////////////////////////////////////
//
//															仙门
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////基本操作////////////////////////////////////////
//创建
#define msg_guild_create_c2s 360
//解散
#define msg_guild_disband_c2s 361
//邀请
#define msg_guild_member_invite_c2s 362
//拒绝
#define msg_guild_member_decline_c2s 363

#define msg_guild_member_accept_c2s 364
//申请
#define msg_guild_member_apply_c2s 365
//离开
#define msg_guild_member_depart_c2s 366
//踢出
#define msg_guild_member_kickout_c2s 367

////////////////////////////////职位操作////////////////////////////////////////
//禅让
#define msg_guild_set_leader_c2s 368
//升值
#define msg_guild_member_promotion_c2s 369
//降职
#define msg_guild_member_demotion_c2s 370

////////////////////////////////日志查看////////////////////////////////////////
//查看日志
#define msg_guild_log_normal_c2s 371
//查看大事记
#define msg_guild_log_event_c2s 372
//修改通知
#define msg_guild_notice_modify_c2s 373
////////////////////////////////建筑使用facilityid:1:仙门 2:铁匠铺 3:百宝箱////////////////////////////////////////
//设置建筑规则:-1:不允许直接加入 || 0:允许任何人加入/使用 || x:允许x级别/贡献度 以上的人加入/使用 
#define msg_guild_facilities_accede_rules_c2s 374
//升级建筑 
#define msg_guild_facilities_upgrade_c2s 375
//加速建筑升级,Itemid
#define msg_guild_facilities_speed_up_c2s 376 
//领取仙门奖励
#define msg_guild_rewards_c2s 377
//查看仙门招募
#define msg_guild_recruite_info_c2s 378
//捐献
#define msg_guild_member_contribute_c2s 379
//铁匠铺升星加成设置条件
#define change_smith_need_contribution_c2s 357

///////////////////////////////////S -> C /////////////////////////////////////////////
//仙门信息
#define msg_guild_info_s2c 380
//操作错误提示
#define msg_guild_opt_result_s2c 381   
//仙门基本信息更新 {Id,Name,Level,Silver,Gold,Notice}
#define msg_guild_base_update_s2c 382
//仙门人员信息更新roleid
#define msg_guild_member_update_s2c 383
//建筑信息更新facilitesid
#define msg_guild_facilities_update_s2c 384
//人员离开
#define msg_guild_member_delete_s2c 385
//人员加入
#define msg_guild_member_add_s2c 386
//你离开了仙门
#define msg_guild_destroy_s2c 387
//拒绝
#define msg_guild_member_decline_s2c 388
//邀请
#define msg_guild_member_invite_s2c 389
//招募信息
#define msg_guild_recruite_info_s2c 391
//日志
#define msg_guild_log_normal_s2c 392
//大事记
#define msg_guild_log_event_s2c 393  


///////////////////////仙门二期新增/////////////////////////////////////////////
//获取入门申请列表
#define msg_guild_get_application_c2s 394
//发送入门申请列表
#define msg_guild_get_application_s2c 395
//处理某人的入门申请
#define msg_guild_application_op_c2s 396   
//修改称号
#define msg_guild_change_nickname_c2s 397   
//修改第三方聊天和语音群号
#define msg_guild_change_chatandvoicegroup_c2s 398

//更新仙门log
#define msg_guild_update_log_s2c 399



//仙门仓库
#define msg_guild_storage_init_c2s  1961
#define msg_guild_storage_init_s2c  1962

//仙门仓库中捐献确定
#define msg_guild_storage_donate_c2s  1963
//仙门仓库 取出仓库物品
#define msg_guild_storage_take_out_c2s  1964
//公会仓库添加物品
#define msg_guild_storage_add_item_s2c  1965
//仙门仓库 分配物品
#define msg_guild_storage_distribute_item_c2s  1968
//仙门仓库中仓库管理确定
#define msg_guild_storage_set_state_c2s  1969
#define msg_guild_storage_update_state_s2c  1970


//仙门仓库中请求批准  (帮主和副帮主)
#define msg_guild_storage_init_apply_c2s  1971
//仙门仓库中请求批准，申请道具初始化
#define msg_guild_storage_init_apply_s2c  1972
//仙门仓库 批准
#define msg_guild_storage_approve_apply_c2s  1973
//仙门仓库 拒绝
#define msg_guild_storage_refuse_apply_c2s  1974
//仙门仓库 全部拒绝
#define msg_guild_storage_refuse_all_apply_c2s  1975
//仙门仓库 申请公会仓库物品
#define msg_guild_storage_apply_item_c2s  1976

//申请列表（帮众，长老）
#define msg_guild_storage_self_apply_c2s  1977
//申请列表结果（帮众，长老）
#define msg_guild_storage_self_apply_s2c  1978
//取消申请列表
#define msg_guild_storage_cancel_apply_c2s  1979

//仙门仓库中仓库记录
#define msg_guild_storage_log_c2s  1980
#define msg_guild_storage_log_s2c  1981
//仙门仓库 仓库物品设置闲置
#define msg_guild_storage_set_item_state_c2s  1982
//仙门仓库整理按钮
#define msg_guild_storage_sort_items_c2s  1983


////////////////////////////////////////////////////////////////////////////////
//
//															仙门结束
//
////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//注册流程相关
///////////////////////////////////////////////////////////////////////////////////
//创建角色
#define msg_create_role_request_c2s 400
//创建角色
#define msg_create_role_sucess_s2c 401
//创建角色失败
#define msg_create_role_failed_s2c 402

///////////////////////////////////////////////////////////////////////////////////
//结束注册流程相关
///////////////////////////////////////////////////////////////////////////////////

//////观察    查看他人资料
#define msg_inspect_c2s 403

#define msg_inspect_s2c 404

#define msg_inspect_faild_s2c 405

///////////////////////////////////////////////////////////////////////////////////
//平台用户信息，从平台登入到游戏时认证
///////////////////////////////////////////////////////////////////////////////////

#define msg_user_auth_c2s 410

#define msg_user_auth_fail_s2c 411

// 技能学习消息
////////////////////////////////////////////////////////////////////////////////
// 枚举技能列表请求
#define msg_enum_skill_item_c2s  412
// 枚举Fail
#define msg_enum_skill_item_fail_s2c   413
// 回复技能列表（成功）
#define msg_enum_skill_item_s2c  414


// 学习技能请求
#define msg_skill_learn_item_c2s  415
// 一键学习技能
#define msg_skill_auto_learn_item_c2s 2055
// 学习技能失败
#define msg_skill_learn_item_fail_s2c   416

//技能学习消息结束

//反馈信息
#define msg_feedback_info_c2s  417

//gm提交信息返回值
#define msg_feedback_info_ret_s2c  418

//复活
#define msg_role_respawn_c2s  419

//重复登录
#define msg_other_login_s2c			  420

//角色被禁
#define msg_block_s2c			  421

#define msg_is_jackaroo_s2c 422
//游客登录
#define msg_is_visitor_c2s 423
//在平台上注册成功,loginKey访问key.php获得传当前的时间戳
#define msg_is_finish_visitor_c2s 425
//提示改角色名称
#define msg_visitor_rename_s2c 426
//改角色名称
#define msg_visitor_rename_c2s 427
//修改角色名称失败
#define msg_visitor_rename_failed_s2c 428

#define msg_mall_item_list_c2s 429	//请求某类别商城列表
#define msg_mall_item_list_s2c 430	//返回某类型商城列表
#define msg_init_mall_item_list_c2s 438	//请求某类别商城列表初始化数据
#define msg_init_mall_item_list_s2c 439	//返回某类型商城列表初始化数据

#define msg_buy_mall_item_c2s 431	//购买某一件商品
#define msg_init_hot_item_s2c 432	//初始化包裹商场物品列表
#define msg_init_latest_item_s2c 433	//初始化最近购买列表
#define msg_mall_item_list_special_c2s 434	//请求特殊类别商品列表
#define msg_mall_item_list_special_s2c 435	//返回特殊类别商品列表
#define msg_mall_item_list_sales_c2s 436	//请求优惠商品列表
#define msg_mall_item_list_sales_s2c 437	//返回优惠商品列表
#define msg_change_role_mall_integral_s2c 440//更新人物商城积分

//用户客户端设置查询 
#define msg_query_player_option_c2s 450

//用户客户端设置查询返回
#define msg_query_player_option_s2c 451

////用户客户端设置替换
#define msg_replace_player_option_c2s 452

//日志回传
#define msg_info_back_c2s 453
////////////////////////////////////////////////////////////////////////////////
//好友消息
////////////////////////////////////////////////////////////////////////////////

//我的好友
#define msg_update_friend_info_s2c 468//@@abandon
#define msg_revert_black_c2s 469//@@abandon
#define msg_revert_black_s2c 470//@@abandon
#define msg_init_signature_s2c 471//@@abandon
#define msg_add_signature_c2s 472//@@abandon

#define msg_get_friend_signature_c2s 473//@@abandon
#define msg_get_friend_signature_s2c 474//@@abandon

#define msg_set_black_c2s 475
#define msg_set_black_s2c 476//@@abandon
#define msg_delete_black_c2s 477
#define msg_delete_black_s2c 478
#define msg_black_list_s2c 479


#define msg_myfriends_c2s 480//@@abandon
//我的好友返回消息
#define msg_myfriends_s2c 481
//添加好友
#define msg_add_friend_c2s 482
#define msg_add_friend_success_s2c 483
#define msg_add_friend_failed_s2c 484//@@abandon
//被关注好友提醒
#define msg_becare_friend_s2c 485//@@abandon
//删除好友
#define msg_delete_friend_c2s 486
//删除好友成功、失败
#define msg_delete_friend_success_s2c 487
#define msg_delete_friend_failed_s2c  488//@@abandon
//好友上线
#define msg_online_friend_s2c 489
//好友下线
#define msg_offline_friend_s2c 490
//取详细资料
#define msg_detail_friend_c2s 491
//返回详细资料
#define msg_detail_friend_s2c 492//@@abandon
#define msg_detail_friend_failed_s2c 493//@@abandon
#define msg_position_friend_c2s 494
#define msg_position_friend_s2c 495//@@abandon
#define msg_position_friend_failed_s2c 496//@@abandon
#define msg_add_black_c2s 497
#define msg_add_black_s2c 498
#define msg_send_flower_to_friend_c2s 499

#define msg_search_role_error_s2c	2249//查找好友失败
#define msg_add_friend_rejuct_s2c	2250//拒绝加为好友
#define msg_auto_find_friend_c2s 2251//自动找朋友
#define msg_friend_intimacy_update_s2c	2252//亲密度更新
#define msg_friend_add_recent_s2c	2253//添加最近好友
#define msg_friend_add_enemy_s2c	2254//添加仇人
#define msg_friend_update_level_s2c	2255//好友等级更新
#define msg_friend_send_flower_c2s 2256//给人送花
#define msg_friend_send_flower_s2c 2257//有人送花
#define msg_add_friend_confirm_c2s 2258//是否同意加为好友
#define msg_add_friend_confirm_s2c 2259//有人加你为好友
#define msg_search_role_c2s 2260//找人
#define msg_search_role_s2c 2261//找人结果
#define msg_friend_opt_s2c 2262//
#define msg_friend_init_recent_s2c 2263//
#define msg_friend_init_enemy_s2c 2264//
#define msg_auto_find_friend_s2c 2269  //自动找朋友

////////////////////////////////////////////////////////////////////////////////
//抽奖消息
////////////////////////////////////////////////////////////////////////////////
#define msg_lottery_lefttime_s2c 501
#define msg_lottery_leftcount_s2c 502
#define msg_lottery_clickslot_c2s 504
#define msg_lottery_clickslot_s2c 505
#define msg_lottery_otherslot_s2c 506
#define msg_lottery_notic_s2c 507
#define msg_lottery_clickslot_failed_s2c 508
#define msg_lottery_querystatus_c2s 509

//////////////挂机////////////////
#define msg_start_block_training_c2s 510	
#define msg_start_block_training_s2c 511
#define msg_end_block_training_c2s 512
#define msg_end_block_training_s2c 513


////////////////////////////////////////////////////////////////////////////////
//邮箱消息
////////////////////////////////////////////////////////////////////////////////
#define msg_mail_status_query_c2s 530
#define msg_mail_status_query_s2c 531
#define msg_mail_arrived_s2c  532
#define msg_mail_query_detail_c2s 533
#define msg_mail_query_detail_s2c 534
#define msg_mail_get_addition_c2s 535
#define msg_mail_get_addition_s2c 536
#define msg_mail_send_c2s		  537	//写邮件
#define msg_mail_delete_c2s		  538
#define msg_mail_delete_s2c   539
#define msg_mail_operator_failed_s2c 540
#define msg_mail_sucess_s2c 541
///////////////////////////////////////////////////////////////////////////////////
//交易
///////////////////////////////////////////////////////////////////////////////////
//C->S
#define msg_trade_role_apply_c2s 560
#define msg_trade_role_accept_c2s 561
#define msg_trade_role_decline_c2s 562
#define msg_set_trade_money_c2s 563
#define msg_set_trade_item_c2s 564
#define msg_cancel_trade_c2s 565
#define msg_trade_role_lock_c2s 566
#define msg_trade_role_dealit_c2s 567
//S->C
#define msg_trade_role_errno_s2c 570
#define msg_trade_begin_s2c 571
#define msg_update_trade_status_s2c 572
#define msg_trade_role_lock_s2c 573
#define msg_trade_role_dealit_s2c 574
#define msg_trade_role_decline_s2c 575
#define msg_trade_role_apply_s2c 576
#define msg_cancel_trade_s2c 577
#define msg_trade_success_s2c 578

//装备强化
#define msg_equipment_stonemix_c2s 599
#define msg_equipment_riseup_c2s 600
#define msg_equipment_riseup_s2c 601
#define msg_equipment_riseup_failed_s2c 602
#define msg_equipment_sock_c2s 603
#define msg_equipment_sock_s2c 604
#define msg_equipment_sock_failed_s2c 605
#define msg_equipment_inlay_c2s 606
#define msg_equipment_inlay_s2c 607
#define msg_equipment_inlay_failed_s2c 608
#define msg_equipment_stone_remove_c2s 609
#define msg_equipment_stone_remove_s2c 610
#define msg_equipment_stone_remove_failed_s2c 611
#define msg_equipment_stonemix_single_c2s 612
#define msg_equipment_stonemix_s2c 613
#define msg_equipment_stonemix_failed_s2c 614
#define msg_equipment_upgrade_c2s 615
#define msg_equipment_upgrade_s2c 616
#define msg_equipment_enchant_c2s 617
#define msg_equipment_enchant_s2c 618
#define msg_equipment_recast_c2s 619
#define msg_equipment_recast_s2c 620
#define msg_equipment_recast_confirm_c2s 621
#define msg_equipment_convert_c2s 622
#define msg_equipment_convert_s2c 623
#define msg_equipment_move_c2s 624
#define msg_equipment_move_s2c 625
#define equipment_remove_seal_s2c 626//解封
#define equipment_remove_seal_c2s 627//请求解封
#define msg_equipment_fenjie_c2s 628//请求分解装备
#define msg_equip_fenjie_optresult_s2c 629//分解结果


//成长系统
//初始化请求
#define msg_achieve_init_c2s 630
//初始化返回
#define msg_achieve_init_s2c 631
//更新消息
#define msg_achieve_update_s2c 632
//奖励领取消息
#define msg_achieve_reward_c2s 633
//错误码
#define msg_achieve_error_s2c 634
//查看别人成就
#define msg_inspect_achieve_s2c 635
//雪域目标
#define msg_goals_init_s2c 640
#define msg_goals_update_s2c 641
#define msg_goals_reward_c2s 642
#define msg_goals_error_s2c 643
#define msg_goals_init_c2s 644

//轮回塔
#define msg_loop_tower_enter_c2s 650
#define msg_loop_tower_enter_failed_s2c 651
#define msg_loop_tower_masters_c2s 652
#define msg_loop_tower_masters_s2c 653
#define msg_loop_tower_enter_s2c 654
#define msg_loop_tower_challenge_c2s 655
#define msg_loop_tower_challenge_success_s2c 656
#define msg_loop_tower_reward_c2s 657
#define msg_loop_tower_challenge_again_c2s 658
#define msg_loop_tower_enter_higher_s2c 659

//VIP
#define msg_vip_ui_c2s 670
#define msg_vip_ui_s2c 671
#define msg_vip_reward_c2s 672
#define msg_vip_error_s2c 673
#define msg_vip_level_up_s2c 674
#define msg_vip_init_s2c 675
#define msg_vip_npc_enum_s2c 676
#define msg_login_bonus_reward_c2s 677
#define msg_vip_role_use_flyshoes_s2c 678
#define msg_join_vip_map_c2s 679

//查询系统开关
#define msg_query_system_switch_c2s 700
#define msg_system_status_s2c 701

//决斗申请
#define msg_duel_invite_c2s 710
//拒绝决斗
#define msg_duel_decline_c2s 711
//接受决斗
#define msg_duel_accept_c2s 712

#define msg_duel_invite_s2c 720
#define msg_duel_decline_s2c 721
#define msg_duel_start_s2c 722
#define msg_duel_result_s2c 723
//设置pk开关
#define msg_set_pkmodel_c2s 730
#define msg_set_pkmodel_faild_s2c 731
#define msg_clear_crime_c2s 733
#define msg_clear_crime_time_s2c 734
//同步时间
#define msg_query_time_c2s 740
#define msg_query_time_s2c 741
#define msg_stop_move_c2s 742


//游戏内完成身份证认证
#define msg_identify_verify_c2s  800
#define msg_identify_verify_s2c  801

//小飞鞋
#define msg_fly_shoes_c2s 810
//大血包
#define msg_hp_package_s2c 811

//仓库
#define msg_npc_swap_item_c2s 812
//指向目标使用型物品
#define msg_use_target_item_c2s 813

//战场
#define msg_tangle_battlefield_info_s2c  819     // 战场信息
#define msg_guild_battlefield_info_s2c   1087	 // 仙门战场
#define msg_battlefield_info_c2s		 1088	 // 请求战场信息
#define msg_battlefield_info_error_s2c   1089	 // 
#define msg_battlefield_totle_info_s2c   1090	 // 
#define msg_yhzq_battlefield_info_s2c	 1091	 //
#define msg_jszd_battlefield_info_s2c    1710    // 战场信息 包括荣誉
#define msg_honor_stores_buy_items_c2s	 1821	 // 购买
#define msg_camp_battle_entry_c2s		 1852    // 进入战场
#define msg_camp_battle_player_num_c2s   1864	 //
#define msg_camp_battle_last_record_c2s	 1866	 // 战场记录
#define msg_travel_battle_entry_c2s      1911    // 纵横九州进入战场
#define msg_travel_battle_all_result_c2s 1917	 //
#define msg_travel_battle_self_result_c2s 1924   // 纵横九州战场记录、我的战报

// 原有战场消息,现在没有采用
#define msg_battle_self_join_s2c		 823
#define msg_tangle_update_s2c			 824
#define msg_tangle_remove_s2c			 825
#define msg_battle_end_s2c				 826
#define msg_battle_reward_c2s			 827
#define msg_battle_other_join_s2c		 828
#define msg_battle_waiting_s2c			 829  //战场等待信息
#define msg_tangle_more_records_c2s		 836  //请求战场详细信息
#define msg_tangle_more_records_s2c      837
#define msg_guild_battle_score_init_s2c 1658
#define msg_battle_reward_by_record_c2s 1010  //战场外领取奖励
#define msg_yhzq_apply_start_s2c		1112  //开始报名
#define msg_yhzq_apply_over_s2c			1113  //结束报名
#define msg_yhzq_all_battle_over_s2c	1098  //所有战场都已结束 没必要再等待了
#define msg_yhzq_error_s2c				1099  //战场错误信息
#define msg_yhzq_list_info_c2s			1100  //获取战场列表信息
#define msg_yhzq_list_info_s2c			1101  //发送战场列表信息
#define msg_apply_yhzq_c2s			    1102  //申请加入战场
#define msg_apply_yhzq_s2c				1103  //申请加入成功 
#define msg_cancel_apply_yhzq_c2s	    1104  //退出排队
#define msg_notify_to_join_yhzq_s2c		1105  //通知可以进入战场
#define msg_join_yhzq_c2s			    1106  //进入战场
#define msg_leave_yhzq_c2s				1107  //退出战场
#define msg_yhzq_award_s2c				1108  //战场结束 可以领取奖励
#define msg_yhzq_award_c2s				1109  //领取奖励
#define msg_yhzq_camp_info_s2c			1110  //战场阵营信息
#define msg_yhzq_zone_info_s2c			1111  //战场区域信息
#define msg_yhzq_battle_self_join_s2c	1114  //自己加入战场
#define msg_yhzq_battle_other_join_s2c	1115  //其他人加入战场
#define msg_yhzq_battle_update_s2c		1116  //战场信息更新
#define msg_yhzq_battle_remove_s2c		1117
#define msg_yhzq_battle_player_pos_s2c  1118
#define msg_yhzq_battle_end_s2c			1119  //战场结束
#define msg_entry_guild_battle_s2c		1655  //进入仙门战场结果
#define msg_leave_guild_battle_c2s		1656  //请求离开仙门战
#define msg_guild_battle_start_s2c		1653  //仙门战开始
#define msg_leave_guild_battle_s2c	    1657  //离开仙门战
#define msg_entry_guild_battle_c2s		1654  //请求进入仙门战场

//副本信息
#define msg_instance_info_s2c 830
#define entry_loop_instance_apply_c2s 1800//队长开启副本
#define entry_loop_instance_vote_s2c 1801//投票表决
#define entry_loop_instance_vote_update_s2c 1802//投票表决变更
#define entry_loop_instance_vote_c2s 1803//投票表决
#define entry_loop_instance_c2s 1804//分层的副本消息

#define entry_loop_instance_s2c 1805//分层的副本消息
#define loop_instance_kill_monsters_info_init_s2c 1813//分层的副本中杀怪消息


#define msg_get_instance_log_c2s 831
#define msg_get_instance_log_s2c 832
#define msg_tangle_records_s2c 833
#define msg_tangle_records_c2s 834
#define msg_tangle_topman_pos_s2c 835

//退出当前副本
#define msg_instance_exit_c2s 838
//队长进入副本,通知队员进入
#define msg_instance_leader_join_s2c 840
//进入副本
#define msg_instance_leader_join_c2s 841
//日常活动
#define msg_start_everquest_s2c 850
#define msg_update_everquest_s2c 851
#define msg_refresh_everquest_c2s 852
#define msg_refresh_everquest_s2c 853
#define msg_npc_start_everquest_c2s 854
#define msg_npc_everquests_enum_c2s 855
#define msg_npc_everquests_enum_s2c 856
#define msg_everquest_list_s2c 857

#define msg_instance_end_seconds_s2c 858

/////////////宠物相关//////////
//初始化宠物
#define msg_init_pets_s2c 900
//创建宠物
#define msg_create_pet_s2c 901
//召唤宠物
#define msg_summon_pet_c2s 902
//移动
#define msg_pet_move_c2s  903
#define msg_pet_stop_move_c2s  904
#define msg_pet_attack_c2s 905
//改名
#define msg_pet_rename_c2s 906
//宠物礼物
#define msg_pet_present_s2c 907
//宠物升级
#define msg_pet_up_level_c2s 908
#define msg_pet_present_apply_s2c 909
//刷新三维点数
#define msg_pet_up_reset_c2s 910
#define msg_pet_up_reset_s2c 911

#define msg_pet_xs_c2s 912//宠物洗髓
#define msg_pet_xs_update_s2c 913//宠物洗髓更新

#define msg_pet_up_growth_s2c 914
#define msg_pet_up_stamina_growth_s2c 915

#define pet_evolution_growthvalue_c2s 1492
#define pet_evolution_growthvalue_s2c 1493

#define pet_auto_advance_c2s 947 //宠物自动进阶
#define pet_auto_advance_result_s2c 948 //宠物自动进阶
#define pet_advance_c2s 946 //宠物进阶
#define pet_advance_update_s2c 945//宠物进阶
//错误消息
#define msg_pet_opt_error_s2c 916

//学技能
#define msg_pet_learn_skill_c2s 917
#define msg_pet_up_exp_c2s 918
#define msg_pet_forget_skill_c2s 919


//删除宠物
#define msg_pet_delete_s2c 920
#define msg_pet_swap_slot_c2s 921
//查看其他人宠物
#define msg_inspect_pet_c2s 922
#define msg_inspect_pet_s2c 923

//坐骑升星
#define msg_pet_riseup_c2s 924
#define msg_pet_riseup_s2c 925
//解锁开锁技能槽
#define msg_pet_skill_slot_lock_c2s 926
#define msg_update_pet_skill_slot_s2c 927
#define msg_update_pet_skill_s2c 928

#define msg_pet_skill_book_init_s2c 929  // 初始化宠物技能书
//#define msg_init_pet_skill_slots_s2c 930
#define msg_init_pet_skill_slots_c2s 930 //刷新技能购买面板

//有高级技能是否覆盖
#define msg_pet_learn_skill_cover_best_s2c 931

#define msg_pet_inheritance_c2s	932//请求继承
#define msg_pet_inheritance_s2c	933//接受继承
#define msg_pet_shop_init_s2c	934//接受宠物商店初始化
#define msg_pet_shop_buy_c2s	936//请求购买宠物
#define msg_pet_shop_init_c2s	938//请求宠物商店初始化
//购买宠物栏
#define msg_buy_pet_slot_c2s 940

//更新宠物栏信息
#define msg_update_pet_slot_num_s2c 941

//喂养宠物
#define msg_pet_feed_c2s 942


//宠物驯养
#define msg_pet_training_info_s2c 950	 //返回驯养信息
#define msg_pet_start_training_c2s 951	 //开始驯养
#define msg_pet_stop_training_c2s 952	 //停止驯养(领取驯养收益)
#define msg_pet_speedup_training_c2s 953	 //加速驯养
#define msg_pet_training_init_info_s2c 954		//初始化驯养信息

//宠物探险仓库
//请求仓库数据
#define msg_explore_storage_init_c2s 960

//发送仓库数据
#define msg_explore_storage_info_s2c 961

//仓库信息初始化结束
#define msg_explore_storage_init_end_s2c 962

//取出某个物品
#define msg_explore_storage_getitem_c2s 963
//取出全部物品
#define msg_explore_storage_getallitems_c2s 964

//更新物品个数
#define msg_explore_storage_updateitem_s2c 965

//添加物品
#define msg_explore_storage_additem_s2c 966

//删除物品
#define msg_explore_storage_delitem_s2c 967

//探险仓库操作码
#define msg_explore_storage_opt_s2c 968

//宠物探险
#define msg_pet_explore_info_c2s 970		//探险信息初始化
#define msg_pet_explore_info_s2c 971			//探险信息初始化返回消息
#define msg_pet_explore_start_c2s 972			//探险请求
#define msg_pet_explore_speedup_c2s 973 		//宠物加速探险
#define msg_pet_explore_stop_c2s 974				//中止宠物探险
#define msg_pet_explore_error_s2c 975			//宠物探险错误消息	
#define msg_pet_explore_gain_info_s2c 976	//宠物探险获得的物品

//宠物资质提升
#define msg_pet_qualification_c2s 1490 
#define msg_pet_qualification_result_s2c 1491

/////////////////元宝的宝箱消息//////////////
//宝箱刷新
#define msg_treasure_chest_flush_c2s 981
//宝箱刷新成功
#define msg_treasure_chest_flush_ok_s2c  982
//宝箱刷新失败
#define msg_treasure_chest_failed_s2c  983
//抽奖请求开始
#define msg_treasure_chest_raffle_c2s  984
//抽奖请求结果
#define msg_treasure_chest_raffle_ok_s2c  985
//领取物品
#define msg_treasure_chest_obtain_c2s  986
//领取物品成功
#define msg_treasure_chest_obtain_ok_s2c  987
//宝箱上次内容查询
#define msg_treasure_chest_query_c2s  989
//宝箱上次内容查询结果
#define msg_treasure_chest_query_s2c  990
//抽中宝箱好东西广播
#define msg_treasure_chest_broad_s2c  991
//去掉某些物品
#define msg_treasure_chest_disable_c2s 992

//天珠祈福改版添加
//祈福请求
#define msg_beads_pray_request_c2s 995
// 祈福成功消息返回
#define msg_beads_pray_response_s2c 996
//祈福失败消息
#define msg_beads_pray_fail_s2c 997
//转盘成功返回消息
#define msg_turntable_response_s2c 998
//转盘成功确认消息
#define msg_turntable_confirm_c2s 999


// 枚举兑换物品请求
#define msg_enum_exchange_item_c2s  1001
// 枚举Fail
#define msg_enum_exchange_item_fail_s2c   1002
// 回复商店物品
#define msg_enum_exchange_item_s2c  1003
// 兑换请求
#define msg_exchange_item_c2s 1004
// 兑换失败
#define msg_exchange_item_fail_s2c  1005



//限时礼包
//限时礼包信息
#define msg_timelimit_gift_info_s2c 1020

//领取礼包
#define msg_get_timelimit_gift_c2s 1021

//错误信息
#define msg_timelimit_gift_error_s2c 1022

//当天领奖已结束
#define msg_timelimit_gift_over_s2c 1023

//                 摆摊消息	begin                                  
//上架
#define msg_stall_sell_item_c2s 2020			
//下架
#define msg_stall_recede_item_c2s 2021
//搜索摊位/所有摊位/下一页/上一页
#define msg_stalls_search_c2s 2023
//查看摊位具体信息
#define msg_stall_detail_c2s 2029
//购买物品
#define msg_stall_buy_item_c2s 2022
//修改摊位名
#define msg_stall_rename_c2s 1036
//搜索物品/下一页/上一页
#define msg_stalls_search_item_c2s 2028
//摊位具体信息
#define msg_stall_detail_s2c 2031
//摊位信息
#define msg_stalls_search_s2c 2025
//摊位日志更新
#define msg_stall_log_add_s2c 1042
//操作返回
#define msg_stall_opt_result_s2c 2033
//以玩家查看摊位信息
#define msg_stall_role_detail_c2s 1044
//搜索物品返回结果
#define msg_stalls_search_item_s2c 1045

//                 摆摊消息	end                                 




//随机角色名
#define msg_init_random_rolename_s2c 1120
#define msg_reset_random_rolename_c2s 1121

//答题
#define msg_answer_sign_notice_s2c 1122
#define msg_answer_sign_request_c2s 1123
#define msg_answer_sign_success_s2c 1124
#define msg_answer_start_notice_s2c 1125
#define msg_answer_question_c2s 1126
#define msg_answer_question_s2c 1127
#define msg_answer_question_ranklist_s2c 1128
#define msg_answer_end_s2c 1129
#define msg_answer_error_s2c 1130	

//离线经验
#define msg_offline_exp_init_s2c 1131
#define msg_offline_exp_quests_init_s2c 1132
#define msg_offline_exp_exchange_c2s 1133
#define msg_offline_exp_error_s2c 1134
#define msg_offline_exp_exchange_gold_c2s 1135

//新手祝贺
#define msg_congratulations_levelup_remind_s2c 1140
#define msg_congratulations_levelup_c2s 1141
#define msg_congratulations_levelup_s2c 1142
#define msg_congratulations_receive_s2c 1143
#define msg_congratulations_error_s2c 1144
#define msg_congratulations_received_c2s 1145

//天降宝箱显示buff
#define msg_treasure_buffer_s2c 1160

//新手礼包状态
#define msg_gift_card_state_s2c 1161
#define msg_gift_card_apply_c2s 1162
#define msg_gift_card_apply_s2c 1163

//棋魂	
//初始化棋魂信息
#define msg_chess_spirit_info_s2c 1170//通灵道副本初始化消息
//初始化玩家信息
#define msg_chess_spirit_role_info_s2c 1171
//更新玩家仙灵值
#define msg_chess_spirit_update_power_s2c 1172
//更新玩家棋魂技能
#define msg_chess_spirit_update_skill_s2c 1173
//更新棋魂能量值
#define msg_chess_spirit_update_chess_power_s2c 1174
//升级技能
#define msg_chess_spirit_skill_levelup_c2s 1175
//释放技能
#define msg_chess_spirit_cast_skill_c2s 1176
//释放大招,队长用
#define msg_chess_spirit_cast_chess_skill_c2s 1177
//操作结果
#define msg_chess_spirit_opt_result_s2s 1178
//请求棋魂记录
#define msg_chess_spirit_log_c2s 1179
//返回棋魂记录
#define msg_chess_spirit_log_s2c 1180
//领奖
#define msg_chess_spirit_get_reward_c2s 1181
//退出棋魂
#define msg_chess_spirit_quit_c2s 1182
//game over
#define msg_chess_spirit_game_over_s2c 1183
//开始倒计时
#define msg_chess_spirit_prepare_s2c 1184

//////仙门二期新增//////////////
//商城
//获取商城物品列表
#define msg_guild_get_shop_item_c2s 1200
//发送商城物品列表
#define msg_guild_get_shop_item_s2c 1201
//购买商城物品
#define msg_guild_shop_buy_item_c2s 1202

//百宝阁
//获取百宝阁物品列表
#define msg_guild_get_treasure_item_c2s 1203
//发送百宝阁物品列表
#define msg_guild_get_treasure_item_s2c 1204
//购买百宝阁物品
#define msg_guild_treasure_buy_item_c2s 1205

//设置百宝阁物品价格
#define msg_guild_treasure_set_price_c2s 1206

//更新百宝阁物品
#define msg_guild_treasure_update_item_s2c 1207

//发布门务
#define msg_publish_guild_quest_c2s 1208

//更新门务信息
#define msg_update_guild_quest_info_s2c 1209

#define msg_update_guild_apply_state_s2c 1210

//添加 删除 仙门申请信息
#define msg_update_guild_update_apply_info_s2c 1211

//仙门申请处理结果
#define msg_guild_update_apply_result_s2c 1212

//请求仙门公告
#define msg_get_guild_notice_c2s 1213

//发送仙门公告
#define msg_send_guild_notice_s2c 1214	

//更新商城物品
#define msg_guild_shop_update_item_s2c 1215

#define change_guild_right_limit_s2c 1217

#define msg_guild_bonfire_start_s2c	1219

///仙门二期 结束///////////////////

//增加已经完成过的升级操作
#define msg_add_levelup_opt_levels_s2c 1220
//请求完成升级操作
#define msg_levelup_opt_c2s 1221

//活动开始预告
#define msg_activity_forecast_begin_s2c 1230
//活动结束预告
#define msg_activity_forecast_end_s2c 1231
//系统广播
#define msg_system_broadcast_s2c 1235

//刷钱副本剩余时间
#define msg_moneygame_left_time_s2c 1240
//副本结果
#define msg_moneygame_result_s2c 1241
//开始倒计时
#define msg_moneygame_prepare_s2c 1242
//绑定钱币，每杀死一只，就会发消息
#define money_from_monster_s2c 113

//当前波数
#define msg_moneygame_cur_sec_s2c 1243
#define msg_expgame_cur_sec_s2c 1823
//仙门召集
#define msg_guild_mastercall_s2c 1245
#define msg_guild_mastercall_accept_c2s 1246
#define msg_guild_mastercall_success_s2c 1247
//仙门成员位置信息
#define msg_guild_member_pos_c2s 1248
#define msg_guild_member_pos_s2c 1249
//离开仙门副本
#define  msg_leave_guild_instance_c2s 358
//进入仙门副本
#define  msg_join_guild_instance_c2s 359
//打坐
#define msg_sitdown_c2s 1250
//停止打坐
#define msg_stop_sitdown_c2s 1251
//请求双修
#define msg_companion_sitdown_apply_c2s 1252		
//某人请求与你双修
#define msg_companion_sitdown_apply_s2c 1253
//开始双修
#define msg_companion_sitdown_start_c2s 1254
//双修操作回复
#define msg_companion_sitdown_result_s2c 1255
//谢绝邀请
#define msg_companion_reject_c2s 1256
//对方谢绝了你的邀请	
#define msg_companion_reject_s2c 1257
//转换阵营
#define msg_dragon_fight_faction_s2c 1258
//暴龙
#define msg_dragon_fight_left_time_s2c 1259
//查看状态
#define msg_dragon_fight_state_s2c 1260
//检测人数
#define msg_dragon_fight_num_c2s 1261
//回复人数
#define msg_dragon_fight_num_s2c 1262
//转换阵营
#define msg_dragon_fight_faction_c2s 1263
//活动开始
#define msg_dragon_fight_start_s2c 1264
//活动结束
#define msg_dragon_fight_end_s2c 1265
//参加活动
#define msg_dragon_fight_join_c2s 1266

//群星陨落
#define msg_star_spawns_section_s2c 1267

//经脉领悟精通开始
#define msg_venation_advanced_start_c2s 1276

//更新经脉顿悟信息
#define msg_venation_advanced_update_s2c 1277

//经脉领悟精通操作结果返回
#define msg_venation_advanced_opt_result_s2c 1278

//经脉初始化
#define msg_venation_init_s2c 1280

//更新经脉穴道及加成信息
#define msg_venation_update_s2c 1281

//更新经脉经验分享信息
#define msg_venation_shareexp_update_s2c 1282

//冲穴开始
#define msg_venation_active_point_start_c2s 1283

//冲穴操作返回值
#define msg_venation_active_point_opt_s2c 1284

//冲穴结束
#define msg_venation_active_point_end_c2s 1285

//经脉操作返回值
#define msg_venation_opt_s2c 1286

//经脉倒计时
#define msg_venation_time_countdown_s2c 1287

//别人的经脉信息
#define msg_other_venation_info_s2c 1288

//跨服地图标志
#define msg_server_travel_tag_s2c 1290

#define msg_continuous_logging_gift_c2s 1300
#define msg_continuous_logging_board_c2s 1301
#define msg_continuous_days_clear_c2s 1302
#define msg_continuous_opt_result_s2c 1303
#define msg_continuous_logging_board_s2c 1304


//祈福仓库
//请求仓库数据
#define msg_treasure_storage_init_c2s 1310

//发送仓库数据
#define msg_treasure_storage_info_s2c 1311

//仓库信息初始化结束
#define msg_treasure_storage_init_end_s2c 1312

//取出某个物品
#define msg_treasure_storage_getitem_c2s 1313
//取出全部物品
#define msg_treasure_storage_getallitems_c2s 1314

//更新物品个数
#define msg_treasure_storage_updateitem_s2c 1315

//添加物品
#define msg_treasure_storage_additem_s2c 1316

//删除物品
#define msg_treasure_storage_delitem_s2c 1317

//祈福仓库操作码
#define msg_treasure_storage_opt_s2c 1318

//请求活跃度初始化
#define msg_activity_value_init_c2s 1400

//活跃度初始化
#define msg_activity_value_init_s2c 1401

//更新活跃度
#define msg_activity_value_update_s2c 1402

//领取活跃度奖励
#define msg_activity_value_reward_c2s 1403

//活跃度操作码
#define msg_activity_value_opt_s2c 1404

//活跃度飘字
#define msg_activity_value_notice_s2c 1405

//活动状态
#define msg_activity_state_init_c2s 1410
//活动状态
#define msg_activity_state_init_s2c 1411
//活动状态更新
#define msg_activity_state_update_s2c 1412


//boss重生状态
#define msg_activity_boss_born_init_c2s 1413
//boss重生状态更新
#define msg_activity_boss_born_init_s2c 1414
//boss重生状态更新
#define msg_activity_boss_born_update_s2c 1415

//首充礼包领取状态
#define msg_first_charge_gift_state_s2c 1416
//领取首充礼包
#define msg_first_charge_gift_reward_c2s 1417
//领取首充礼包
#define msg_first_charge_gift_reward_opt_s2c 1418

//请求排行榜信息
#define msg_rank_get_rank_c2s 1428
//请求上榜人物信息
#define msg_rank_get_rank_role_c2s 1429

//排行榜信息
#define msg_rank_list_s2c 1430
//	msg_rank_loop_tower_s2c 轮回塔原来为1430，现在取消掉

//战场杀人榜
#define msg_rank_killer_s2c 1431
//财富榜
#define msg_rank_moneys_s2c 1432
//近战攻击榜
#define msg_rank_melee_power_s2c 1433
//射手攻击榜
#define msg_rank_range_power_s2c 1434
//法师攻击榜
#define msg_rank_magic_power_s2c 1435
//轮回塔层数
#define msg_rank_loop_tower_num_s2c 1436
//等级排行
#define msg_rank_level_s2c 1437
//答题排行
#define msg_rank_answer_s2c 1438
//排行榜玩家信息
#define msg_rank_get_rank_role_s2c 1439
//鄙视
#define msg_rank_disdain_role_c2s 1440
//赞扬
#define msg_rank_praise_role_c2s 1441
//评价结果返回
#define msg_rank_judge_opt_result_s2c 1442	
//斗转棋魂单人排行
#define msg_rank_chess_spirits_single_s2c 1443
//斗转棋魂多人排行
#define msg_rank_chess_spirits_team_s2c 1444


// facebook
#define msg_facebook_bind_check_c2s 1445
#define msg_facebook_bind_check_result_s2c 1446

//清除仙门称号
#define msg_guild_clear_nickname_c2s 1447

//每日首次登陆提醒
#define msg_everyday_show_s2c 1448

//崇拜鄙视提示
#define msg_rank_judge_to_other_s2c 1450
//宠物天赋排行
#define msg_rank_talent_score_s2c 1451
//主线排行榜
#define msg_rank_mail_line_s2c 1452
//查询主线排行请求
#define msg_rank_get_main_line_rank_c2s 1453
//战斗力排行榜
#define msg_rank_fighting_force_s2c 1454
//星运排行榜
#define msg_rank_astrology_s2c 1455

//福利面板消息
//福利面板初始化请求
#define msg_welfare_panel_init_c2s 1460
#define msg_welfare_panel_init_s2c 1461
//福利活动状态更新
#define msg_welfare_gifepacks_state_update_s2c 1462
//消耗元宝兑换活动
#define msg_welfare_gold_exchange_init_c2s 1463
#define msg_activity_boss_entrust_init_c2s 1900

#define msg_welfare_gold_exchange_init_s2c 1464
//兑换请求
#define msg_welfare_gold_exchange_c2s 1465
//召唤坐骑
#define msg_ride_opt_c2s 1466
#define msg_ride_opt_result_s2c 1467
//物品鉴定请求
#define msg_item_identify_c2s 1480
//物品鉴定错误返回
#define msg_item_identify_error_s2c 1481
//坐骑合成请求
#define msg_ride_pet_synthesis_c2s 1482
//坐骑合成操作结果返回
#define msg_ridepet_synthesis_opt_result_s2c 1483
//坐骑合成错误返回
#define msg_ridepet_synthesis_error_s2c 1484
//宠物洗天赋请求
#define msg_pet_random_talent_c2s 1485
//宠物天赋替换
#define msg_pet_change_talent_c2s 1486
//宠物天赋随机属性返回
#define msg_pet_random_talent_s2c 1487
//物品鉴定结果返回
#define msg_item_identify_opt_result_s2c 1488

//宠物进化请求
#define msg_pet_evolution_c2s 1489
#define msg_pet_growup_c2s 1494

//资质和资质上限提升消息

//资质提升请求
#define msg_pet_upgrade_quality_c2s 1500
//资质上限提升请求
#define msg_pet_upgrade_quality_up_c2s 1501
//宠物属性加点请求
#define msg_pet_add_attr_c2s 1502
//宠物属性洗点
#define msg_pet_wash_attr_c2s 1503
//宠物资质提升结果返回
#define msg_pet_upgrade_quality_s2c 1504
//宠物资质上限提升结果返回
#define msg_pet_upgrade_quality_up_s2c 1505
//请求获得宠物技能书
#define msg_pet_get_skill_book_c2s 937
//宠物装备更新
#define msg_update_item_for_pet_s2c 1510
//给宠物穿装备
#define msg_equip_item_for_pet_c2s 1511
//给宠物脱装备
#define msg_unequip_item_for_pet_c2s 1512
//操作提示
#define msg_pet_item_opt_result_s2c  1513


//炼制系统

#define msg_refine_system_c2s 1520
#define msg_refine_system_s2c 1521

//福利活动面板活动更新
#define msg_welfare_activity_update_c2s 1530
#define msg_welfare_activity_update_s2c 1531

//称号相关
#define msg_designation_init_s2c 1540	
//获得称号更新消息
#define msg_designation_update_s2c 1541
//查看他人的称号信息
#define msg_inspect_designation_s2c 1542

//运镖剩余时间
#define msg_treasure_transport_time_s2c 1550
//运镖失败
#define msg_treasure_transport_failed_s2c 1551
//无法开启仙门运镖
#define msg_start_guild_transport_failed_s2c 1552
//劫镖成功
#define msg_rob_treasure_transport_s2c 1553
//全服运镖开始
#define msg_server_treasure_transport_start_s2c 1554
//全服运镖结束
#define msg_server_treasure_transport_end_s2c 1555
//运镖倒计时检查
#define msg_role_treasure_transport_time_check_c2s 1556
//仙门运镖剩余时间
#define msg_guild_transport_left_time_s2c 1557
//开启仙门运镖
#define msg_start_guild_treasure_transport_c2s 1558
//仙门运镖求助通知其他仙门成员
#define msg_treasure_transport_call_guild_help_s2c 1559

//主线系统初始化
#define msg_mainline_init_c2s 1560
#define msg_mainline_init_s2c 1561

//更新主线状态
#define msg_mainline_update_s2c 1562

//开始进入
#define msg_mainline_start_entry_c2s 1563
//进入成功
#define msg_mainline_start_entry_s2c 1564

//开始挑战
#define msg_mainline_start_c2s 1565
//挑战开始
#define msg_mainline_start_s2c 1566

//结束挑战
#define msg_mainline_end_c2s 1567
//结束挑战成功
#define msg_mainline_end_s2c 1568

//挑战结果
#define msg_mainline_result_s2c 1569

//领取主线奖励
#define msg_mainline_reward_c2s 1570

//主线副本倒计时
#define msg_mainline_lefttime_s2c 1571

//副本倒计时结束
#define msg_mainline_timeout_c2s 1572

//副本剩余怪物信息更新
#define msg_mainline_remain_monsters_info_s2c 1573
//副本击杀怪物信息更新
#define msg_mainline_kill_monsters_info_s2c 1574
//防守副本怪物波数更新
#define msg_mainline_section_info_s2c 1575
//防守副本守护NPC信息更新
#define msg_mainline_protect_npc_info_s2c 1576

//主线系统操作码
#define msg_mainline_opt_s2c 1577

//领取奖励成功
#define msg_mainline_reward_success_s2c 1578

//温泉
#define msg_spa_start_notice_s2c 1600
#define msg_spa_request_spalist_c2s 1601
#define msg_spa_request_spalist_s2c 1602
#define msg_spa_join_c2s 1603
#define msg_spa_join_s2c 1604
#define msg_spa_chopping_s2c 1605
#define msg_spa_swimming_c2s 1607
#define msg_spa_swimming_s2c 1608
#define msg_spa_error_s2c 1610
#define msg_spa_leave_c2s 1611
#define msg_spa_leave_s2c 1612
#define msg_spa_stop_s2c 1613
#define msg_spa_chopping_c2s 1614
#define msg_spa_update_count_s2c 1615

//仙门运镖求助结果
#define msg_treasure_transport_call_guild_help_result_s2c 1620
//仙门运镖求助请求
#define msg_treasure_transport_call_guild_help_c2s 1621

//系统版本号
#define msg_server_version_c2s 1630
#define msg_server_version_s2c 1631


//国家系统
//初始化国家信息
#define msg_country_init_s2c 1640
//更新国家公告
#define msg_change_country_notice_c2s 1641
#define msg_change_country_notice_s2c 1642

//发布国运
#define msg_change_country_transport_c2s 1643
#define msg_change_country_transport_s2c 1644
//职位任免
//升职
#define msg_country_leader_promotion_c2s 1645
//降职
#define msg_country_leader_demotion_c2s 1646
#define msg_country_leader_update_s2c 1647

//禁言
#define msg_country_block_talk_c2s 1648
//罪恶值
#define msg_country_change_crime_c2s 1649

//国家领导人上线
#define msg_country_leader_online_s2c 1650
//领取专属道具
#define msg_country_leader_get_itmes_c2s 1651
//领取每日奖励
#define msg_country_leader_ever_reward_c2s 1652



//王座积分状态
#define msg_guild_battle_score_update_s2c 1659

//王座状态更新
#define msg_guild_battle_status_update_s2c 1660

//仙门战结果
#define msg_guild_battle_result_s2c 1661

#define msg_country_opt_s2c 1662
#define msg_guild_battle_opt_s2c 1663

//仙门战结束
#define msg_guild_battle_stop_s2c 1664

//初始化国家信息
#define msg_country_init_c2s 1665

//初始化仙门战准备时间倒计时
#define msg_guild_battle_ready_s2c 1666

//仙门战报名
#define msg_apply_guild_battle_c2s 1667

//仙门战报名开始
#define msg_guild_battle_start_apply_s2c 1668

//仙门战报名结束
#define msg_guild_battle_stop_apply_s2c 1669  	

//开服活动初始化面板
#define msg_init_open_service_activities_s2c 1680
//更新开服活动状态
#define msg_open_sercice_activities_update_s2c 1681
//领取奖励
#define msg_open_service_activities_reward_c2s 1682
//开服活动初始化面板请求
#define msg_init_open_service_activities_c2s 1683
//初始化开服活动子标签状态请求
#define msg_init_open_service_sub_tab_state_c2s 1684

//节日活动
//活动面板标签页初始化消息
#define msg_activity_tab_isshow_s2c 1690
//节日活动初始化请求
#define msg_festival_init_c2s 1691
//充值送礼活动初始化返回
#define msg_festival_recharge_s2c 1692
//节日活动错误返回消息
#define msg_festival_error_s2c 1693
//充值送礼兑换请求消息
#define msg_festival_recharge_exchange_c2s 1694
//充值送礼活动更新信息
#define msg_festival_recharge_update_s2c 1695
//领取滑块通知消息
#define msg_festival_recharge_notice_s2c 1696
//活动面板中子活动状态初始化
#define msg_activity_tab_sub_activity_state_s2c 1697



//晶石争夺战
#define msg_jszd_start_notice_s2c 1700
#define msg_jszd_join_c2s 1701
#define msg_jszd_join_s2c 1702
#define msg_jszd_leave_c2s 1703
#define msg_jszd_leave_s2c 1704
#define msg_jszd_update_s2c 1705
#define msg_jszd_end_s2c 1706
#define msg_jszd_reward_c2s 1707
#define msg_jszd_error_s2c 1708
#define msg_jszd_stop_s2c 1709


//仙门第三次修改消息

//捐献日志
#define msg_guild_contribute_log_c2s 1720
#define msg_guild_contribute_log_s2c 1721

//弹劾仙主
#define msg_guild_impeach_c2s 1722
//弹劾操作结果
#define msg_guild_impeach_result_s2c 1723

//弹劾信息查询
#define msg_guild_impeach_info_c2s 1724
#define msg_guild_impeach_info_s2c 1725

//投票
#define msg_guild_impeach_vote_c2s 1726

//弹劾结束
#define msg_guild_impeach_stop_s2c 1727

//加入仙门剩余时间
#define msg_guild_join_lefttime_s2c 1728

//灵魂力状态
#define msg_spiritspower_state_update_s2c 1730

//灵魂力清零
#define msg_spiritspower_reset_c2s 1731

//圣诞树提交物品请求活动
#define msg_christmas_tree_grow_up_c2s 1740
//领取圣诞节奖励
#define msg_christmas_activity_reward_c2s 1741
//返回圣诞树血量
#define msg_christmas_tree_hp_s2c 1742
//播放特效
#define msg_play_effects_s2c 1743

//群雄逐鹿击杀信息
//群雄逐鹿击杀信息请求消息
#define msg_tangle_kill_info_request_c2s 1751
//群雄逐鹿击杀信息返回消息
#define msg_tangle_kill_info_request_s2c 1752

//竞技场
//提示选手进入竞技场
#define msg_world_cup_notify_fighter_join_s2c 1761
//提示选手准备
#define msg_world_cup_notify_fighter_prepare_s2c 1762
//提示选手比赛开始
#define msg_world_cup_notify_fighter_start_s2c 1763
//战场结束
#define msg_world_cup_notify_fighter_end_s2c  1764
//进入战场请求
#define msg_world_cup_enter_battle_c2s 1765
//退出战场请求
#define msg_world_cup_leave_battle_c2s 1766
//战场战斗信息
#define msg_world_cup_battle_kill_info_s2c 1767
//请求当前轮次信息
#define msg_world_cup_cur_section_info_c2s 1768
//返回轮次信息
#define msg_world_cup_cur_section_info_s2c 1769
//查看战场记录	
#define msg_world_cup_record_c2s 1770
//返回战场记录		
#define msg_world_cup_record_s2c 1771
//领取战场奖励
#define msg_world_cup_reward_c2s 1772
//请求奖励列表信息
#define msg_world_cup_reward_info_c2s 1773
//返回奖励信息
#define msg_world_cup_reward_info_s2c 1774
//通知玩家做好进入竞技场的准备
#define msg_world_cup_notice_prepare_s2c 1775
//告知玩家轮空直接晋级
#define msg_world_cup_promotion_s2c 1776
//玩家弃权
#define msg_world_cup_give_up_c2s 1777
//倒计时消息
#define msg_world_cup_start_left_time 1778

//签到活动
//签到初始化信息请求
#define msg_sign_activity_init_c2s 1791
//返回签到初始化信息
#define msg_sign_activity_init_s2c 1792
//签到请求消息
#define msg_sign_activity_request_c2s 1793
//签到请求返回消息
#define msg_sign_activity_request_s2c 1794


//占星

//占星请求  占星返回
#define msg_astrology_action_c2s 2190
#define msg_astrology_action_s2c 2191
//初始化面板
#define msg_astrology_init_c2s 2192
#define msg_astrology_init_s2c 2193
//拾取
#define msg_astrology_pickup_c2s 2194
#define msg_astrology_pickup_s2c 2195
//一键拾取
#define msg_astrology_pickup_all_c2s 2196
#define msg_astrology_pickup_all_s2c 2197
//卖出
#define msg_astrology_sale_c2s 2198
#define msg_astrology_sale_s2c 2199
//一键卖出
#define msg_astrology_sale_all_c2s 2200
#define msg_astrology_sale_all_s2c 2201
//更新钱和位置，增加星魂值
#define msg_astrology_money_and_pos_s2c 2202
//合成
#define msg_astrology_mix_c2s 2203
//一键合成
#define msg_astrology_mix_all_c2s 2204
//锁定
#define msg_astrology_lock_c2s 2205
//解锁
#define msg_astrology_unlock_c2s 2206
//更新占星背包size
#define msg_astrology_package_size_s2c 2207
//初始化占星背包
#define msg_astrology_init_package_s2c 2208
//统一错误消息
#define msg_astrology_error_s2c 2209
////元宝占星
//#define msg_astrology_gold_pos_c2s  1820
//拾取到占星背包，星座添加(人物身上)
#define msg_astrology_add_s2c 2211
//从占星背包删除星座
#define msg_astrology_delete_s2c 2212
//更新占星背包物品，更新星座
#define msg_astrology_update_s2c 2213
//扩展背包
#define msg_astrology_expand_package_c2s 2214
//移动背包
#define msg_astrology_swap_c2s 2215
//补充星魂
#define msg_astrology_add_money_c2s 2216
//更新星运值，星运评价
#define msg_astrology_update_value_s2c 2217
//查看别人星魂
#define msg_other_astrology_info_s2c 2218
//道具开启
#define msg_astrology_item_pos_c2s 2219


//宝石打磨
#define msg_polish_c2s 1840
//宝石抛光
#define msg_polished_c2s 1841
//宝石熔炼
#define msg_polish_mix_c2s 1842
//操作成功
#define msg_polish_success_s2c 1843
//操作失败
#define msg_polish_error_s2c 1844
//二级密码
//初始化及更新消息
#define msg_protectlock_update_s2c 1861
//设置密码消息
#define msg_protectlock_set_c2s 1862
//解除锁定
#define msg_protectlock_change_lock_c2s 1863
//修改密码  作为战场消息使用
//#define msg_protectlock_change_pwd_c2s 1864
//删除密码 
#define msg_protectlock_del_pwd_c2s 1865 
//结果消息
#define msg_protectlock_res_s2c 1866

//竞拍活动
//竞拍面板初始化请求
#define msg_bid_init_c2s 1871
//竞拍面板初始化
#define msg_bid_init_s2c 1872
//出价
#define msg_bid_offer_price_c2s 1873
//更新
#define msg_bid_update_info_s2c 1874
//时间同步消息
#define msg_bid_sync_time_s2c 1875
//错误码
#define msg_bid_result_s2c 1876
//竞拍开始通知消息
#define msg_bid_notice_s2c 1877
//请求竞拍列表
#define msg_request_bid_list_c2s 1878
//返回竞拍列表
#define msg_request_bid_list_s2c 1879	
//返回元宝或礼券消息
#define msg_return_bid_price_s2c 1880

//结婚
#define msg_wedding_marriage_c2s 1890
#define msg_wedding_divorce_c2s 1891
#define msg_wedding_force_divorce_c2s 1892
#define msg_wedding_error_s2c 1893
#define msg_wedding_propose_to_you_s2c 1894
#define msg_wedding_propose_agree_c2s 1895
#define msg_wedding_propose_result_s2c 1896

//结婚仪式
//仪式预定显示信息
#define msg_wedding_ceremony_show_c2s 1897
#define msg_wedding_ceremony_show_s2c 1898
//仪式预定
#define msg_wedding_ceremony_predestine_c2s 1899
//设定礼金下线
#define msg_wedding_set_gift_lower_limit_c2s 1901
//邀请好友
#define msg_wedding_invite_friends_c2s 1902
//仪式开始请求
#define msg_wedding_ceremony_start_c2s 1903
//仪式通知消息
#define msg_wedding_ceremony_notice_s2c 1905
//当前礼金下限请求
#define msg_wedding_cur_gift_lower_limit_c2s 1906

#define msg_wedding_cur_gift_lower_limit_s2c 1907

#define msg_wedding_room_join_c2s 1908
#define msg_wedding_room_leave_c2s 1909
#define msg_wedding_room_start_notice_s2c 1910
#define msg_wedding_room_join_s2c 1911
#define msg_wedding_room_leave_s2c 1912
#define msg_wedding_room_stop_s2c 1913
#define msg_wedding_room_need_gifts_s2c 1914
#define msg_wedding_room_gave_gifts_c2s 1915

///////////////////////////////////////////////////////////////////////////////////
//如意袋1931 - 1960 
///////////////////////////////////////////////////////////////////////////////////				
//选择模式			//1:普通抽取 2:乾坤一致
#define msg_bag_lottery_type_c2s 1931														//bag_lottery_packet
//展现备选物品
#define msg_bag_lottery_items_s2c 1932
//免费刷新
#define msg_bag_lottery_refresh_c2s 1933												//bag_lottery_packet
//点击选择	
#define msg_bag_lottery_choose_c2s 1934													//bag_lottery_packet
//抽取物品结果
#define msg_bag_lottery_choose_s2c 1935
//点击领取(第一种模式)
#define msg_bag_lottery_got_c2s 1936														//bag_lottery_packet
//点击退还(第二种模式)
#define msg_bag_lottery_back_c2s 1937														//bag_lottery_packet
//抽取结束物品展现
#define msg_bag_lottery_end_items_s2c 1938
//开启额外奖励
#define msg_bag_lottery_ext_s2c 1939
//额外点击选择
#define msg_bag_lottery_choose_ext_c2s 1940											//bag_lottery_packet
//额外抽取物品结果
#define msg_bag_lottery_choose_ext_s2c 1941
//额外抽取结束物品展现
#define msg_bag_lottery_end_items_ext_s2c 1942
//开启超级奖励
#define msg_bag_lottery_sup_s2c 1943
//超级点击选择
#define msg_bag_lottery_choose_sup_c2s 1944											//bag_lottery_packet
//超级抽取物品结果
#define msg_bag_lottery_choose_sup_s2c 1945
//超级抽取结束物品展现
#define msg_bag_lottery_end_items_sup_s2c 1946
//操作错误码
#define msg_bag_lottery_error_s2c 1947
//奖励结束
#define msg_bag_lottery_reward_end_s2c 1948
//重置乾坤袋
#define msg_reset_bag_lottery_c2s 1949

//请求丹药队列信息
#define msg_get_furnace_queue_info_c2s 2411
//提取丹药
#define msg_get_furnace_queue_item_c2s 2412
//炼丹
#define msg_create_pill_c2s 2418
//中断炼制
#define msg_quit_furnace_queue_c2s 2410
//加速炼丹
#define msg_accelerate_furnace_queue_c2s 2419
//开启炼炉
#define msg_unlock_furnace_queue_c2s 2416
//炼炉升级
#define msg_up_furnace_c2s 2414
//丹药队列信息
#define msg_furnace_queue_info_s2c 2413
//炼炉的信息
#define msg_furnace_info_s2c 2415
//丹药效果
#define msg_pill_info_s2c 2417