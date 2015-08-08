
function Timer()
   WriteToConsole( "Timer event start" );

   for i = 1,10 do
   end

--   while true do
      WriteToConsole( "test" );
 --  end   
    
   WriteToConsole( "end of script");
end

function FunctionTwo()
   WriteToConsole("function two start" );
--   for i = 1, 200 do
      WriteToConsole("function two " .. i );
 --  end
   WriteToConsole("function two end" );
end

function FunctionThree()
   WriteToConsole("function three run" );
end

