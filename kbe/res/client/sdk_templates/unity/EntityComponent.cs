namespace KBEngine
{
	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Threading;

	/*
		实体组件模块基础类
	*/
	public class EntityComponent
	{
		public UInt16 entityComponentPropertyID = 0;
		public UInt16 componentType = 0;
		public Int32 ownerID = 0;
		public Entity owner = null;

		public virtual void onRemoteMethodCall(UInt16 methodUtype, MemoryStream stream)
		{
			// 动态生成
		}

		public virtual void onUpdatePropertys(UInt16 propUtype, MemoryStream stream, int maxCount)
		{
			// 动态生成
		}

		public virtual void callPropertysSetMethods()
		{
			// 动态生成
		}

		public virtual void createFromStream(MemoryStream stream)
		{
			componentType = (UInt16)stream.readInt32();
			ownerID = stream.readInt32();

			//UInt16 ComponentDescrsType;
			stream.readUint16();

			UInt16 count = stream.readUint16();

			if(count > 0)
				onUpdatePropertys(0, stream, count);
		}
	}
}