/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

namespace KBEngine
{

INLINE const char * ProfileVal::c_str() const { return name_.c_str(); }

INLINE double ProfileVal::lastTimeInSeconds() const { return stampsToSeconds(lastTime_); }
INLINE double ProfileVal::sumTimeInSeconds() const  { return stampsToSeconds( sumTime_ ); }
INLINE double ProfileVal::lastIntTimeInSeconds() const { return stampsToSeconds( lastIntTime_ ); }
INLINE double ProfileVal::sumIntTimeInSeconds() const { return stampsToSeconds( sumIntTime_ ); }

INLINE TimeStamp ProfileVal::lastTime() const
{
	return lastTime_;
}

INLINE bool ProfileVal::stop(const char * filename, int lineNum, uint32 qty)
{
	this->stop(qty);

	const bool tooLong = isTooLong();

	if (tooLong)
	{
		WARNING_MSG(boost::format("%s:%d: Profile %s took %.2f seconds\n") %
			filename %
			lineNum %
			name_.c_str() %
			(lastTime_  / stampsPerSecondD()));
	}

	return true;
}

INLINE bool ProfileVal::isTooLong() const
{
	return (lastTime_ > warningPeriod_);
}

}