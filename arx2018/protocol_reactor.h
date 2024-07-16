#pragma once

#include "icmd.h"

namespace wzj
{
	class protocol_reactor : public icmd {
	public:
		static protocol_reactor* instance() {
			static protocol_reactor one;
			return &one;
		}

		const TCHAR* name() const override {
			return _T("protocol_reactor");
		}

		void init_impl() override;

		void stop_impl() override;
	};
}