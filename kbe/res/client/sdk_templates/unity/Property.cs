namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	
	/*
		抽象出一个entitydef中定义的属性
		该模块描述了属性的id以及数据类型等信息
	*/
    public class Property 
    {
		public enum EntityDataFlags
		{
			ED_FLAG_UNKOWN													= 0x00000000, // 未定义
			ED_FLAG_CELL_PUBLIC												= 0x00000001, // 相关所有cell广播
			ED_FLAG_CELL_PRIVATE											= 0x00000002, // 当前cell
			ED_FLAG_ALL_CLIENTS												= 0x00000004, // cell广播与所有客户端
			ED_FLAG_CELL_PUBLIC_AND_OWN										= 0x00000008, // cell广播与自己的客户端
			ED_FLAG_OWN_CLIENT												= 0x00000010, // 当前cell和客户端
			ED_FLAG_BASE_AND_CLIENT											= 0x00000020, // base和客户端
			ED_FLAG_BASE													= 0x00000040, // 当前base
			ED_FLAG_OTHER_CLIENTS											= 0x00000080, // cell广播和其他客户端
		};
		    	
		public string name = "";
    	public KBEDATATYPE_BASE utype = null;
		public UInt16 properUtype = 0;
		public UInt32 properFlags = 0;
		public Int16 aliasID = -1;
		
		public string defaultValStr = "";
		public System.Reflection.MethodInfo setmethod = null;
		
		public object val = null;
		
		public Property()
		{
			
		}
		
		public bool isBase()
		{
			return properFlags == (UInt32)EntityDataFlags.ED_FLAG_BASE_AND_CLIENT ||
				properFlags == (UInt32)EntityDataFlags.ED_FLAG_BASE;
		}
		
		public bool isOwnerOnly()
		{
			return properFlags == (UInt32)EntityDataFlags.ED_FLAG_CELL_PUBLIC_AND_OWN ||
				properFlags == (UInt32)EntityDataFlags.ED_FLAG_OWN_CLIENT;
		}
		
		public bool isOtherOnly()
		{
			return properFlags == (UInt32)EntityDataFlags.ED_FLAG_OTHER_CLIENTS ||
				properFlags == (UInt32)EntityDataFlags.ED_FLAG_OTHER_CLIENTS;
		}
    }
    
} 
