
function Timer()
   WriteToConsole( "Timer event start" );
   
   iNumObjects = GetNumObjects();
   WriteToConsole( "Num Objects:" .. iNumObjects );
   
   WriteToConsole("test3");

            movedata = {}
         movedata["pos"] = {};
         movedata["pos"]["x"] = math.random() * 20.0 - 10.0;
         movedata["pos"]["y"] = math.random() * 20.0 - 10.0;
         movedata["pos"]["z"] = math.random() * 10.0;
         movedata["scale"] = {};
         movedata["scale"]["x"] = math.random() * 3.0;
         movedata["scale"]["y"] = math.random() * 3.0;
         movedata["scale"]["z"] = math.random() * 3.0;
         movedata["color"] = {};
         movedata["color"]["r"] = math.random();
         movedata["color"]["g"] = math.random();
         movedata["color"]["b"] = math.random();
         movedata["duration"] = 5000;
      
         Axis = {};
         Axis["x"] = math.random();
         Axis["y"] = math.random();
         Axis["z"] = math.random();
         movedata["rot"] = AxisAngle2Rot( Axis, math.random() * 6 ); 

         MoveMeTableVersion( movedata );
   
    
   WriteToConsole( "end of script");
end

