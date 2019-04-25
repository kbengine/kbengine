// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBVar.h"
#include "KBECommon.h"

namespace KBEngine
{

class Method;
class Property;
class EntityCall;
class MemoryStream;
class EntityComponent;

/*
	KBEngine逻辑层的实体基础类
	所有扩展出的游戏实体都应该继承于该模块

	https://github.com/kbengine/kbengine/blob/master/kbe/res/sdk_templates/client/ue4/README.md
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

	virtual void onComponentsEnterworld()
	{
		// 动态生成， 通知组件onEnterworld
	}

	virtual void onComponentsLeaveworld()
	{
		// 动态生成， 通知组件onLeaveworld
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

	virtual TArray<EntityComponent*> getComponents(FString componentName, bool all)
	{
		TArray<EntityComponent*> founds;

		return founds;
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

}







