#pragma once

#include <thread>
#include <atomic>

namespace FreezeWatcher
{
	// Config Options
    static REX::INI::I32 iSleepTimer{ "FreezeWatcher"sv, "SleepTimer"sv, 125 };
    static REX::INI::I32 iMaxLockCount{ "FreezeWatcher"sv, "MaxLockCount"sv, 8 };

	// Relocation IDs
	// NG 1.10.984.0: 2692050
	// AE 1.11.137.0+: 4799342
	REL::Relocation<int*> ConditionLockCountPointer{ REL::ID(4799342) };
	
	static void FreezeWatcherThread()
	{
		REX::INFO("Started FreezeWatcher Thread");

		int LoopCounter = 0;
		bool Escaped = false;

		while (true)
		{
			// Are there any Locks?
			if (*ConditionLockCountPointer == 0)
			{
				if (Escaped)
				{
					// There was and we Escaped
					REX::INFO("Successfully Escaped from Freezing!");
				}

				// Reset
				LoopCounter = 0;
				Escaped = false;

				// Sleep
				std::this_thread::sleep_for(std::chrono::milliseconds(iSleepTimer.GetValue()));
				continue;
			}

			// Lock Detected!
			REX::INFO("Lock Detected! Lock Count: {}, Loop Count: {}", *ConditionLockCountPointer, LoopCounter++);

			// Exceeded the Threshold
			if (LoopCounter > iMaxLockCount.GetValue())
			{
				REX::INFO("Exceeded Threshold, Unlocking...");
				
				*ConditionLockCountPointer = 0;
				Escaped = true;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(iSleepTimer.GetValue()));
		}
	}

	inline bool Install()
	{
		REX::INFO("Starting FreezeWatcher Thread");
		std::thread(FreezeWatcherThread).detach();

		return true;
	}
}
