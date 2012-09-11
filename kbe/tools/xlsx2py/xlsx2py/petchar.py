# -*- coding: gb2312 -*-
#


#-----------------------------------------------------------------------------------------------------------------------------------
# 宠物
def funcPetData(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	if dataName == "petCombatDatas":
		for value in datas.values():
			entityNo = value["entityNO"]
			level = value["level"]
			key = (entityNo, level)

			value.pop("entityNO")
			value.pop("level")
			value.pop("id")
			
			d[key] = value
	elif dataName == "petFellowDatas":
		for value in datas.values():
			genuisClass = value["genuisClass"]
                        fiveDimType = value["fiveDimType"]
                        level = value["level"]
                        key = (genuisClass, fiveDimType, level)
                        
                        value.pop("genuisClass")
                        value.pop("fiveDimType")
                        value.pop("level")
                        value.pop("id")
                        
                        d[key] = value
        elif dataName == "petFellowLimitDatas":
                for value in datas.values():
			genuisClass = value["genuisClass"]
                        fiveDimType = value["fiveDimType"]
                        level = value["level"]
                        key = (genuisClass, fiveDimType, level)
                        
                        value.pop("genuisClass")
                        value.pop("fiveDimType")
                        value.pop("level")
                        value.pop("id")
                        
                        d[key] = value
        elif dataName == "petFellowModelDatas":
                for value in datas.values():
                        entityNo = value["entityNO"]
                        level = value["level"]
                        key = (entityNo, level)

                        value.pop("entityNO")
			value.pop("level")
			value.pop("id")

			d[key] = value
        else:
                print( "Unknow Table")
                print( x)
	return d
