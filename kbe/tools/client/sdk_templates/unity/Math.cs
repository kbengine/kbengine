using UnityEngine;
using KBEngine;
using System; 
using System.Collections;

namespace KBEngine
{

/*
	KBEngine的数学相关模块
*/
public class KBEMath 
{
	public static float int82angle(SByte angle, bool half)
	{
		float halfv = 128f;
		if(half == true)
			halfv = 254f;
		
		halfv = ((float)angle) * ((float)System.Math.PI / halfv);
		return halfv;
	}
	
	public static bool almostEqual(float f1, float f2, float epsilon)
	{
		return Math.Abs( f1 - f2 ) < epsilon;
	}
}


}
