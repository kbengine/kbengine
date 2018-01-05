# -*- coding: utf-8 -*-
import KBEngine
import GlobalConst
import d_spaces
import d_avatar_inittab
from KBEDebug import * 

class Teleport:
	def __init__(self):
		# 如果登录是一个副本, 无论如何登录都放置在主场景上
		# 因为副本是需要钥匙开启的，所有的副本都使用实体SpaceDuplicate创建
		# 因此我们只需要简单判断当前spaceUType所对应的配置中场景的脚本类型是否包含"Duplicate"
		# 就能得出是否在一个副本中
		spacedatas = d_spaces.datas[self.cellData["spaceUType"]]
		avatar_inittab = d_avatar_inittab.datas[self.roleType]

		if "Duplicate" in spacedatas["entityType"]:
			self.cellData["spaceUType"] = avatar_inittab["spaceUType"]
			self.cellData["direction"] = (0, 0, avatar_inittab["spawnYaw"])
			self.cellData["position"] = avatar_inittab["spawnPos"]

	#--------------------------------------------------------------------------------------------
	#                              Callbacks
	#--------------------------------------------------------------------------------------------
	def onEntitiesEnabled(self):
		"""
		KBEngine method.
		该entity被正式激活为可使用， 此时entity已经建立了client对应实体， 可以在此创建它的
		cell部分。
		"""
		if self.cell is not None:
			return 

		# 防止使用同一个号登陆不同的demo造成无法找到匹配的地图从而无法加载资源导致无法进入游戏
		# 这里检查一下， 发现不对则强制同步到匹配的地图
		# 忽略机器人的检查
		if hasattr(self, "cellData") and self.getClientType() != 6:
			# 如果角色跳转到了同属某个demo的其他场景那么不强制回到出生的主场景
			if self.cellData["spaceUType"] in GlobalConst.g_demoMaps.values():
				spaceUType = GlobalConst.g_demoMaps.get(self.getClientDatas()[0], 1)

				if self.cellData["spaceUType"] != spaceUType:
					spacedatas = d_spaces.datas[spaceUType]
					self.spaceUTypeB = spaceUType
					self.cellData["spaceUType"] = spaceUType
					self.cellData["position"] = spacedatas.get("spawnPos", (0,0,0))
		
		KBEngine.globalData["Spaces"].loginToSpace(self, self.spaceUTypeB, {})



