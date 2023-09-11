#pragma once
namespace wzj {
    class exdict
    {
        static exdict* instance() {
            static exdict one;
            return &one;
        }

        void stop();

        void init();


    };
}

