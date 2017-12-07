using System; 
using System.Net.Sockets; 
using System.Net; 
using System.Collections; 
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;

namespace KBEngine
{
	public class Profile 
	{
		System.DateTime startTime;
		string _name = "";
		
		public Profile(string name)
		{
			_name = name;
		}
		
		~Profile()
		{
		}

		public void start()
		{
			startTime = System.DateTime.Now;
		}
		
		public void end()
		{
			System.TimeSpan time = System.DateTime.Now - startTime;
			
			if(time.Milliseconds >= 100)
				Dbg.WARNING_MSG("Profile::profile(): '" + _name + "' took " + time.Milliseconds + " ms");
		}
	}
}
