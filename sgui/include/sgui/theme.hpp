#pragma once
#include "dr4/math/color.hpp"
#include <string_view>

namespace sgui {

using dr4::Color;

namespace theme {

    static const float fontSize = 12;
    static const Color textColor = Color(255, 255, 255);

    namespace menu {
        static const Color bgColor = Color(80, 80, 80);
        static const Color bgColor_hover = Color(130, 130, 130);
        static const Color textColor = Color(255, 255, 255);
        static const Color textColor_arrow = Color(180, 180, 180);
        static const Color borderColor = Color(150, 150, 150);
        static const float fontSize = 12;
    };

    namespace item {
        static const Color bgColor = Color(40, 40, 40);
        static const Color bgColor_hover = Color(70, 70, 70);
        static const Color bgColor_press = Color(100, 100, 100);
        static const Color bgColor_selected = Color(0, 100, 0);
        static const Color bgColor_selected_press = Color(0, 150, 0);

        static const Color textColor = Color(255, 255, 255);
        static const Color textColor_selected = Color(255, 255, 255);
    };

    namespace button {
        static const Color bgColor = Color(80, 80, 80);
        static const Color bgColor_hover = Color(130, 130, 130);
        static const Color bgColor_press = Color(150, 150, 150);
        static const Color bgColor_selected = Color(0, 100, 0);

        static const Color textColor = Color(255, 255, 255);
        static const Color textColor_selected = Color(255, 255, 255);

        static const Color borderColor = Color(150, 150, 150);
        static const Color borderColor_selected = Color(0, 255, 0);
        
        static const float fontSize = 12;
    };

    namespace window {
        static const Color titlebarBg      = Color(0,   100, 0  );
        static const Color titlebarBg_drag = Color(0,   130, 0  );
        static const Color titlebarText    = Color(255, 255, 255);
        static const Color titlebarBorder  = Color(0,   255, 0  );
        static const Color border          = Color(100, 100, 100);
        static const Color bg              = Color(30,  30,  30 );

        static const float borderWidth     = 1;
        static const float xPad            = 10;
        static const float titleHeight     = 24;
        static const float fontSize        = 12;
    };

    namespace scrollbar {
        static const std::string_view upIcon = "";
        static const std::string_view downIcon = "";
        static const std::string_view scrollerIcon = "󰇙";
        static const float width = 20;
        static const float buttonScrollDist = 30;
        static const float wheelSpeed = 10;
        static const Color bg = Color(50, 50, 50);
        static const Color border = Color(100, 100, 100);
    };

    namespace textedit {
        static const Color bgColor = Color(60, 60, 60);
        static const Color bgColor_active = Color(80, 80, 80);
        static const Color textColor = Color(255, 255, 255);
        static const Color borderColor = Color(100, 100, 100);
        static const Color borderColor_bad = Color(255, 0, 0);
    };


    namespace theme {
        static const Color bgColor = Color(0, 100, 0);
        static const Color bgColor_press = Color(0, 130, 0);
        static const Color textColor = Color(255, 255, 255);

        static const Color titleColor = Color(0, 255, 0);
        static const Color titleColor2 = Color(0, 200, 0);
    };
};
};


