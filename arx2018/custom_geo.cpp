#include "pch.h"
#include "custom_geo.h"

#include <geassign.h>
#include <acestext.h>
#include <dbregion.h>
#include <dbbody.h>

#include "command.h"


namespace wzj {

    namespace detail {

        
    }


    void custom_geo::init_impl() {
        
    }

    void custom_geo::stop_impl() {
        acedRegCmds->removeGroup(_T("MY_COMMAND_CGEO"));
    }
}