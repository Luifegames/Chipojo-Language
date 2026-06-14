#include "core.h"





void run_game(){
    if (function_exist("_init")){
        Value init = get_var_value("_init");
        Value args[0];

        function_call(init,args,0,indx);
    }


    while (!WindowShouldClose()){
        float dt = GetFrameTime();
        
        if (function_exist("_update"))
        {
            Value init = get_var_value("_update");
            Value args[1];
            args[0].type = VAR_NUMBER;
            args[0].value.num = dt;

            function_call(init, args, 1, indx);
        }


        BeginDrawing();
        ClearBackground(BLACK);

        if (function_exist("_draw"))
        {
            Value init = get_var_value("_draw");
            Value args[0];
            function_call(init, args, 0, indx);
        }

        EndDrawing();

    }

    CloseWindow();

}