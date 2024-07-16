#include "pch.h"
#include "input_point.h"

#include <

namespace wzj {

    namespace detail {

    } 

    void input_point::init_impl()
    {
        curDoc()->inputPointManager()->Add;
        // add monitor
       
        //acedRegCmds->addCommand(_T("WZJ_COMMAND_INPUT_POINT"), _T("GLOBAL_IPT_"), _T(""), ACRX_CMD_TRANSPARENT, detail::);;
    }

    void input_point::stop_impl()
    {
        auto ipm = curDoc()->inputPointManager();
        // remove monitor

    }
}