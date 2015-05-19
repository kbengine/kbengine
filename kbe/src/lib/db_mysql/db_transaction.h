/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef KBE_DB_TRANSACTION_HELPER_H
#define KBE_DB_TRANSACTION_HELPER_H

namespace KBEngine { 
class DBInterface;

/**
 */
class DBTransaction
{
public:
	DBTransaction(DBInterface* dbi, bool autostart = true);
	~DBTransaction();
	
	void start();
	void end();

	void commit();

	bool shouldRetry() const;

	void pdbi(DBInterface* dbi){ dbi_ = dbi; }
private:
	DBInterface* dbi_;
	bool committed_;
	bool autostart_;
};

}
#endif // KBE_DB_TRANSACTION_HELPER_H

