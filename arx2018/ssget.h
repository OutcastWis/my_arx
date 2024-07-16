#pragma once

#include "icmd.h"

namespace wzj {
	class ssget : public icmd {
	public :
		static ssget* instance() {
			static ssget one;
			return &one;
		}

		const TCHAR* name() const override {
			return _T("ssget");
		}

		void init_impl() override;
		
		void stop_impl() override;
	};
}