namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	/*
		实体定义的方法模块
		抽象出一个def文件中定义的方法，改模块类提供了该方法的相关描述信息
		例如：方法的参数、方法的id、方法对应脚本的handler
	*/
    public class Method 
    {
		public string name = "";
    	public UInt16 methodUtype = 0;
    	public Int16 aliasID = -1;
		public List<DATATYPE_BASE> args = null;
		
		public Method()
		{
			
		}
    }
    
} 
