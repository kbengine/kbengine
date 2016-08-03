#pragma once

namespace KBEngine {
	class DBInterface;
	namespace mongodb {

		class DBTransaction
		{
		public:
			DBTransaction(DBInterface* pdbi, bool autostart = true);
			~DBTransaction();

			void start();
			void end();

			void commit();

			bool shouldRetry() const;

			void pdbi(DBInterface* pdbi){ pdbi_ = pdbi; }

		private:
			DBInterface* pdbi_;
			bool committed_;
			bool autostart_;
		};
	}
}