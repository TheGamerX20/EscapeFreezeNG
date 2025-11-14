#pragma once

#include <atomic>

namespace EscapeFreeze
{
	struct lockPatchAddresses
	{
		const std::string name;
		const int id;
		const int offset;
	};

	struct unlockPatchAddresses
	{
		const std::string name;
		const int id;
		const int startOffset;
		const int endOffset;
	};

	const std::vector<std::pair<lockPatchAddresses, unlockPatchAddresses>> addresses = { { { "VATS::GetCurrentAction", 2237208, 0x2F }, { "VATS::GetCurrentAction", 2237208, 0x5A, 0x78 } } };

	class Spinlock
	{
	private:
		std::atomic_flag atomic_flag = ATOMIC_FLAG_INIT;

	public:
		void lock()
		{
			_lockCount++;

			while (atomic_flag.test_and_set(std::memory_order_acquire))
			{
				Sleep(1);
			}

			_owningThread = REX::W32::GetCurrentThreadId();
		}
		void unlock()
		{
			if (_owningThread == REX::W32::GetCurrentThreadId())
			{
				atomic_flag.clear(std::memory_order_release);
				_owningThread = 0;
				_lockCount--;
			}
		}

		std::atomic<std::uint32_t> _owningThread{ 0 };
		volatile std::atomic<std::uint32_t> _lockCount{ 0 };
	};

	static std::vector<Spinlock*> spinlocks{};

	struct detail
	{
		struct Patch : Xbyak::CodeGenerator
		{
			Patch(uintptr_t a_dst, uintptr_t a_newlock)
			{
				Xbyak::Label dst;

				mov(rcx, a_newlock);
				jmp(ptr[rip + dst]);

				L(dst);
				dq(a_dst);
			}
		};

		static void lock(Spinlock* a_sl)
		{
			a_sl->lock();
		};

		static void unlock(Spinlock* a_sl)
		{
			a_sl->unlock();
		};
	};

	inline bool Install()
	{
		auto& trampoline = REL::GetTrampoline();
		for (const auto& pair : addresses)
		{
			auto& lockAddress = pair.first;
			auto& unlockAddress = pair.second;
			auto sl = new Spinlock();
			spinlocks.push_back(sl);

			// Lock
			REL::Relocation<uintptr_t> lockTarget{ REL::ID(lockAddress.id), lockAddress.offset };
			detail::Patch lockP{ reinterpret_cast<uintptr_t>(&detail::lock), reinterpret_cast<uintptr_t>(sl) };
			lockP.ready();
			trampoline.write_call<5>(lockTarget.address(), trampoline.allocate(lockP));
			REX::INFO("Escape Freeze: Installed Escape Freeze Lock Fix: {0:x}", lockTarget.get());

			// UnLock
			REL::Relocation<uintptr_t> unlockTarget{ REL::ID(unlockAddress.id), unlockAddress.startOffset };
			REL::Relocation<uintptr_t> end{ REL::ID(unlockAddress.id), unlockAddress.endOffset };

			REL::WriteSafeFill(unlockTarget.address(), REL::NOP, end.address() - unlockTarget.address());

			detail::Patch unlockP{ reinterpret_cast<uintptr_t>(&detail::unlock), reinterpret_cast<uintptr_t>(sl) };
			unlockP.ready();
			trampoline.write_call<5>(unlockTarget.address(), trampoline.allocate(unlockP));
			REX::INFO("Escape Freeze: Installed Escape Freeze UnLock Fix: {0:x}", unlockTarget.address());
		}

		REX::INFO("Escape Freeze: Installed {} Locks for Escape Freeze Fix", spinlocks.size());

		for (const auto& spinlock : spinlocks)
		{
			REX::INFO("Escape Freeze: {:x}", reinterpret_cast<uintptr_t>(spinlock));
		}

		return true;
	}
}
