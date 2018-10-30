// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBVar.h"
#include "KBECommon.h"

class Method;
class Property;
class EntityCall;
class MemoryStream;

/*
	KBEngine逻辑层的实体基础类
	所有扩展出的游戏实体都应该继承于该模块

	要实现一个KBE对应的实体必须经过以下几步
	1: 在服务器entity_defs中entities.xml中注册实体并实现实体的def定义
	2: 在服务器的相关位置如：assets/scripts目录的cell（ 取决于实体拥有该部分）或base（ 取决于实体拥有该部分）文件夹下实现实体的服务器部分py脚本模块
	3: 在UE4客户端中kbe_scripts文件夹下实现实体的客户端部分脚本模块（这里统一称为实体脚本，虽然是C++实现），UE4实现实体后必须有如下几步
		A：在实体的头文件中按照格式定义ENTITYDEF_DECLARE_模块名（用于声明实体def相关属性和方法用于后续自动化协议绑定）， 具体看demo
		B：在实体CPP文件中ENTITYDEF_CLASS_REGISTER将实体注册（用于告诉插件客户端具体实现了哪些实体），具体看demo
*/
class KBENGINEPLUGINS_API Entity
{
public:
	Entity();
	virtual ~Entity();

	virtual void __init__();

	static void clear();

public:
	const FString& className() const {
		return className_;
	}

	void className(const FString& v) {
		className_ = v;
	}

	bool inWorld() const {
		return inWorld_;
	}

	void inWorld(bool v) {
		inWorld_ = v;
	}

	bool isControlled() const {
		return isControlled_;
	}

	void isControlled(bool v) {
		isControlled_ = v;
	}

	bool inited() const {
		return inited_;
	}

	void inited(bool v) {
		inited_ = v;
	}

	ENTITY_ID id() const {
		return id_;
	}

	void id(ENTITY_ID v) {
		id_ = v;
	}

	float velocity() const {
		return velocity_;
	}

	void velocity(float v) {
		velocity_ = v;
	}

	bool isPlayer();

	void isOnGround(bool v) {
		isOnGround_ = v;
	}

	bool isOnGround() const {
		return isOnGround_;
	}

public:
	void destroy()
	{
		detachComponents();
		onDestroy();
	}

	virtual void onDestroy()
	{
	}

	void enterWorld();
	virtual void onEnterWorld();
	void leaveWorld();
	virtual void onLeaveWorld();

	virtual void enterSpace();
	virtual void onEnterSpace();
	virtual void leaveSpace();
	virtual void onLeaveSpace();

	// This callback method is called when the local entity control by the client has been enabled or disabled. 
	// See the Entity.controlledBy() method in the CellApp server code for more infomation.
	// 对于玩家自身来说，它表示是否自己被其它玩家控制了；
	// 对于其它entity来说，表示我本机是否控制了这个entity
	virtual void onControlled(bool isControlled)
	{

	}

	virtual void onUpdateVolatileData()
	{
	}

	virtual void onPositionChanged(const FVector& oldValue);
	virtual void onDirectionChanged(const FVector& oldValue);

	virtual EntityCall* getBaseEntityCall()
	{
		return NULL;
	}

	virtual EntityCall* getCellEntityCall()
	{
		return NULL;
	}

	virtual void onRemoteMethodCall(MemoryStream& stream)
	{
		// 动态生成
	}

	virtual void onUpdatePropertys(MemoryStream& stream)
	{
		// 动态生成
	}

	virtual void onGetBase()
	{
		// 动态生成
	}

	virtual void onGetCell()
	{
		// 动态生成
	}

	virtual void onLoseCell()
	{
		// 动态生成
	}

	virtual void callPropertysSetMethods()
	{
		// 动态生成
	}

	virtual void attachComponents()
	{
		// 动态生成
	}

	virtual void detachComponents()
	{
		// 动态生成
	}

protected:
	ENTITY_ID id_;
	FString className_;

	bool isOnGround_;

	// enterworld之后设置为true
	bool inWorld_;

	// 对于玩家自身来说，它表示是否自己被其它玩家控制了；
	// 对于其它entity来说，表示我本机是否控制了这个entity
	bool isControlled_;

	bool inited_;

	float velocity_;

public:
	FVector position;
	FVector direction;
	uint32 spaceID;

	// 当前玩家最后一次同步到服务端的位置与朝向
	// 这两个属性是给引擎KBEngine.cs用的，别的地方不要修改
	FVector entityLastLocalPos;
	FVector entityLastLocalDir;

	//EntityCall* baseEntityCall = null;
	//EntityCall* cellEntityCall = null;
};








