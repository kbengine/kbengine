//
//  KDataTypes.cpp
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#include "KDataTypes.h"
#include "KMessage.h"
#include "KNetworkInterface.h"
#include "cocos2d.h"
#include "KEntitydef.h"


namespace KBEngineClient{

	void LoadArgFromStream(int argtype, object& obj,MemoryStream& stream)
	{
		KBEDATATYPE_BASE& kbtype = EntityDef::iddatatypes[argtype];
		obj = kbtype.createFromStream(stream);
 	}

	void KBEDATATYPE_FIXED_DICT::bind()
	{
			//{
			//	////string[] keys = new string[dicttype.Keys.Count];
			//	////dicttype.Keys.CopyTo(keys, 0);
			//	//
			//	//foreach(string itemkey in keys)
			//	//{
			//	//	object type = dicttype[itemkey];
			//	//	
			//	//	if(type.GetType() == typeof(KBEDATATYPE_BASE).GetType())
			//	//		((KBEDATATYPE_BASE)type).bind();
			//	//	else
			//	//		if(EntityDef.iddatatypes.ContainsKey((UInt16)type))
			//	//			dicttype[itemkey] = EntityDef.iddatatypes[(UInt16)type];
			//	//}
			
			for(std::map<std::string, uint16>::iterator it = dicttype.begin(); it != dicttype.end(); it++)
				{
					uint16 typid = it->second;
					
					if( EntityDef::iddatatypes.find(typid) != EntityDef::iddatatypes.end() )
					{
						dicttype[it->first]  = typid;//EntityDef::iddatatypes[ typid ];
						//it should be a simple data type or complex ? 
					}
				
				}
	}


	void KBEDATATYPE_ARRAY::bind() /**/
	{
		////*	if(type.GetType() == typeof(KBEDATATYPE_BASE).GetType())
		//		((KBEDATATYPE_BASE)type).bind();
		//	else
		//		if(EntityDef.iddatatypes.ContainsKey((UInt16)type))
		//			type = EntityDef.iddatatypes[(UInt16)type];*/
		//  replaced with really data type? 
	}

} //end namespace.
