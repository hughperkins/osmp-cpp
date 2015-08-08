set HELPFILEPATH=%1
if .%HELPFILEPATH%==. set HELPFILEPATH="http://manageddreams.com/osmpwiki/index.php?title=User_Instructions"
start /separate "Help" %HELPFILEPATH%
